// ui.cpp is compiled WITHOUT the precompiled header so we can control
// include order. CommonLib MUST come first - REX::W32 errors if Windows.h
// is detected before CommonLib headers are included.
// CMake: set_source_files_properties(src/ui.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "xedit_linker.h"
#include "ui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace UI
{
    namespace
    {
        bool g_initialized = false;

        struct OverlayState
        {
            bool          visible = false;
            std::uint32_t refID   = 0;
            std::uint32_t baseID  = 0;
            std::string   name;
        } g_overlay;

        WNDPROC g_origWndProc = nullptr;

        LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            if (g_initialized)
                ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
            return ::CallWindowProcW(g_origWndProc, hWnd, msg, wParam, lParam);
        }

        void InitImGui(IDXGISwapChain* swapChain)
        {
            ID3D11Device*        device = nullptr;
            ID3D11DeviceContext* ctx    = nullptr;

            if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device),
                    reinterpret_cast<void**>(&device))))
                return;

            device->GetImmediateContext(&ctx);

            DXGI_SWAP_CHAIN_DESC desc{};
            swapChain->GetDesc(&desc);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui::GetIO().IniFilename = nullptr;  // no imgui.ini on disk

            ImGui::StyleColorsDark();
            ImGuiStyle& style               = ImGui::GetStyle();
            style.WindowRounding            = 6.0f;
            style.FrameRounding             = 4.0f;
            style.WindowPadding             = ImVec2(12.f, 10.f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.88f);
            style.Colors[ImGuiCol_Border]   = ImVec4(0.30f, 0.30f, 0.30f, 0.60f);

            ImGui_ImplWin32_Init(desc.OutputWindow);
            ImGui_ImplDX11_Init(device, ctx);

            // Subclass the game window so ImGui receives Win32 mouse messages
            g_origWndProc = reinterpret_cast<WNDPROC>(
                ::SetWindowLongPtrW(desc.OutputWindow, GWLP_WNDPROC,
                    reinterpret_cast<LONG_PTR>(HookedWndProc)));

            ctx->Release();
            device->Release();
        }

        void RenderFrame()
        {
            if (!g_overlay.visible)
                return;

            auto* ui = RE::UI::GetSingleton();
            if (!ui || !ui->IsMenuOpen(RE::Console::MENU_NAME))
                return;

            const ImGuiViewport* vp = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(
                ImVec2(vp->WorkPos.x + 20.f, vp->WorkPos.y + 20.f),
                ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.88f);

            constexpr ImGuiWindowFlags kFlags =
                ImGuiWindowFlags_NoTitleBar        |
                ImGuiWindowFlags_NoResize          |
                ImGuiWindowFlags_NoMove            |
                ImGuiWindowFlags_AlwaysAutoResize  |
                ImGuiWindowFlags_NoSavedSettings   |
                ImGuiWindowFlags_NoFocusOnAppearing|
                ImGuiWindowFlags_NoNav;

            if (ImGui::Begin("##xEditLinkerOverlay", nullptr, kFlags)) {

                if (!g_overlay.name.empty()) {
                    ImGui::TextUnformatted(g_overlay.name.c_str());
                    ImGui::SameLine();
                    ImGui::TextDisabled("[%08X]", g_overlay.refID);
                } else {
                    ImGui::TextDisabled("FormID: %08X", g_overlay.refID);
                }

                ImGui::Spacing();
                ImGui::TextColored(
                    ImVec4(0.45f, 0.45f, 0.45f, 1.00f),
                    "double-click -> xEdit");
            }
            ImGui::End();
        }

        // Present sits at vtable slot 8:
        //   IUnknown(0-2) + IDXGIObject(3-6) + IDXGIDeviceSubObject(7) + Present(8)

        using FnPresent = HRESULT(STDMETHODCALLTYPE*)(IDXGISwapChain*, UINT, UINT);
        FnPresent g_origPresent = nullptr;

        HRESULT STDMETHODCALLTYPE HookedPresent(
            IDXGISwapChain* swapChain,
            UINT            syncInterval,
            UINT            flags)
        {
            if (!g_initialized) {
                InitImGui(swapChain);
                g_initialized = true;
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            RenderFrame();

            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            return g_origPresent(swapChain, syncInterval, flags);
        }

        void HookSwapChainPresent(IDXGISwapChain* swapChain)
        {
            constexpr int kPresentVtblIdx = 8;

            void** vtable = *reinterpret_cast<void***>(swapChain);
            g_origPresent = reinterpret_cast<FnPresent>(vtable[kPresentVtblIdx]);

            DWORD oldProtect{};
            ::VirtualProtect(&vtable[kPresentVtblIdx], sizeof(void*),
                PAGE_READWRITE, &oldProtect);
            vtable[kPresentVtblIdx] = reinterpret_cast<void*>(&HookedPresent);
            ::VirtualProtect(&vtable[kPresentVtblIdx], sizeof(void*),
                oldProtect, &oldProtect);
        }

    }  // namespace (anonymous)

    void Init()
    {
        auto* renderer = RE::BSGraphics::Renderer::GetSingleton();
        if (!renderer)
            return;

        // renderWindows[0] is the main game window - holds the live swapchain
        auto* swapChain = reinterpret_cast<IDXGISwapChain*>(
            renderer->GetRuntimeData().renderWindows[0].swapChain);
        if (!swapChain)
            return;

        HookSwapChainPresent(swapChain);
    }

    void SetPendingRef(std::uint32_t refID, std::uint32_t baseID, std::string name)
    {
        g_overlay = { true, refID, baseID, std::move(name) };
    }

    void ClearPendingRef()
    {
        g_overlay.visible = false;
    }

    void SendPendingRef()
    {
        if (g_overlay.visible)
            OpenInXEdit(g_overlay.refID, g_overlay.baseID);
    }

}  // namespace UI
