// Copyright (c) Modding Forge
// SKSE plugin entry point; wires input events to xEdit IPC.
#include "xedit_linker.h"

#include <atomic>
#include <chrono>
#include <thread>

namespace Config
{
    enum class TriggerMode { kDoubleClick = 1, kHotkey = 2, kSingleClick = 3 };

    std::wstring  xEditPath;
    TriggerMode   triggerMode = TriggerMode::kHotkey;
    std::uint32_t hotkeyCode  = 0;  // DirectInput scancode; 0 = disabled

    void Load(const std::wstring& iniPath)
    {
        const wchar_t* ini = iniPath.c_str();

        wchar_t pathBuf[MAX_PATH]{};
        ::GetPrivateProfileStringW(L"xEditLinker", L"xEditPath", L"",
            pathBuf, MAX_PATH, ini);
        xEditPath = pathBuf;

        wchar_t modeBuf[32]{};
        ::GetPrivateProfileStringW(L"xEditLinker", L"TriggerMode", L"Hotkey",
            modeBuf, 32, ini);
        if      (_wcsicmp(modeBuf, L"SingleClick")  == 0) triggerMode = TriggerMode::kSingleClick;
        else if (_wcsicmp(modeBuf, L"DoubleClick")  == 0) triggerMode = TriggerMode::kDoubleClick;
        else                                               triggerMode = TriggerMode::kHotkey;

        hotkeyCode = static_cast<std::uint32_t>(
            ::GetPrivateProfileIntW(L"xEditLinker", L"HotkeyCode", 60, ini));
    }
}

namespace
{
    std::filesystem::path GetDataPath()
    {
        return std::filesystem::current_path() / L"Data";
    }

    std::atomic<bool> g_xEditRunning = false;
    HANDLE             g_xEditProcess  = nullptr;

    bool IsXEditAlive()
    {
        if (!g_xEditProcess)
            return false;
        if (::WaitForSingleObject(g_xEditProcess, 0) == WAIT_TIMEOUT)
            return true;
        // Process has exited
        ::CloseHandle(g_xEditProcess);
        g_xEditProcess = nullptr;
        g_xEditRunning.store(false, std::memory_order_relaxed);
        return false;
    }

    void LaunchXEdit()
    {
        if (Config::xEditPath.empty()) {
            RE::ConsoleLog::GetSingleton()->Print(
                "[xEditLinker] xEditPath not configured. "
                "Edit Data\\SKSE\\Plugins\\SSEEditLinker.ini");
            return;
        }

        const auto dataPath = GetDataPath();

        std::wstring cmdLine =
            L"\"" + Config::xEditPath + L"\""
            L" -agl"
            L" -D:\"" + dataPath.wstring() + L"\"";

        STARTUPINFOW        si{}; si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};

        if (::CreateProcessW(
                Config::xEditPath.c_str(),
                cmdLine.data(),
                nullptr, nullptr, FALSE,
                DETACHED_PROCESS,
                nullptr, nullptr, &si, &pi))
        {
            ::CloseHandle(pi.hThread);
            g_xEditProcess = pi.hProcess;  // keep open to detect termination
            g_xEditRunning.store(true, std::memory_order_release);
            RE::ConsoleLog::GetSingleton()->Print("[xEditLinker] xEdit launched.");
        }
        else
        {
            RE::ConsoleLog::GetSingleton()->Print(
                "[xEditLinker] Failed to launch xEdit (Win32 error %lu).",
                ::GetLastError());
        }
    }

    struct PendingRef { std::uint32_t refID = 0; std::uint32_t baseID = 0; };
    PendingRef    g_pendingRef;
    std::uint32_t g_lastReadFormID = 0;

    void ReadCurrentConsoleRef()
    {
        const auto ref = RE::Console::GetSelectedRef();
        if (!ref) return;
        const std::uint32_t refID = ref->GetFormID();
        if (refID == 0 || refID == g_lastReadFormID) return;
        g_lastReadFormID = refID;
        const auto* base = ref->GetBaseObject();
        g_pendingRef     = { refID, base ? base->GetFormID() : 0u };
    }

    PendingRef g_pendingLaunchRef;

    // RE::CreateMessage's second parameter is typed as IMessageBoxCallback* in CommonLib,
    // but Skyrim internally stores it as OldMessageBoxCallback::Callback* (raw function ptr).
    // Passing a vtable object causes a crash - pass a static function cast instead.
    void XelLaunchCallbackFn(RE::IMessageBoxCallback::Message a_msg)
    {
        // Button 0 = first button ("Yes"), anything else = cancel/No
        if (a_msg == RE::IMessageBoxCallback::Message::kUnk0) {
            std::thread([ref = g_pendingLaunchRef]() {
                LaunchXEdit();
                if (g_xEditRunning.load(std::memory_order_acquire) && ref.refID != 0)
                    OpenInXEdit(ref.refID, ref.baseID);
            }).detach();
        }
        g_pendingLaunchRef = {};
    }

    void TrySendToXEdit()
    {
        if (!g_xEditRunning || !IsXEditAlive()) {

            if (Config::xEditPath.empty()) {
                RE::ConsoleLog::GetSingleton()->Print(
                    "[xEditLinker] xEditPath not configured. "
                    "Edit Data\\SKSE\\Plugins\\SSEEditLinker.ini");
                return;
            }
            if (!std::filesystem::exists(Config::xEditPath)) {
                const auto narrow = SKSE::stl::utf16_to_utf8(Config::xEditPath).value_or("<encoding error>"s);
                RE::ConsoleLog::GetSingleton()->Print(
                    "[xEditLinker] xEdit not found at: %s", narrow.c_str());
                return;
            }

            // Save the pending ref so the callback can send it after launch
            g_pendingLaunchRef = g_pendingRef;

            const auto pathUtf8 = SKSE::stl::utf16_to_utf8(Config::xEditPath).value_or("<encoding error>"s);

            // CreateMessage uses a static body string pointer - store in static to keep alive
            static std::string s_msgBody;
            s_msgBody = "Start xEdit?\n" + pathUtf8;

            // Hide the Console first so the MessageBox appears above it.
            if (auto* msgQ = RE::UIMessageQueue::GetSingleton())
                msgQ->AddMessage(RE::Console::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);

            // The native Skyrim function is variadic and reads buttons until nullptr.
            // RE::CreateMessage wraps it with a fixed 7-param signature, omitting the null
            // terminator - causing Skyrim to read a garbage third button off the stack.
            // Call via RELOCATION_ID directly with an explicit null terminator.
            using CreateMessageFn = void(*)(
                const char*, RE::IMessageBoxCallback*,
                std::uint32_t, std::uint32_t, std::uint32_t,
                const char*, const char*, const char*);
            static REL::Relocation<CreateMessageFn> createMessage{ RELOCATION_ID(51420, 52269) };
            createMessage(
                s_msgBody.c_str(),
                reinterpret_cast<RE::IMessageBoxCallback*>(XelLaunchCallbackFn),
                0, 4, 10,
                "Yes", "No", nullptr);
            return;
        }
        if (g_pendingRef.refID != 0)
            OpenInXEdit(g_pendingRef.refID, g_pendingRef.baseID);
    }

    class InputEventSink final : public RE::BSTEventSink<RE::InputEvent*>
    {
    public:
        [[nodiscard]] static InputEventSink* GetSingleton() noexcept
        {
            static InputEventSink s;
            return &s;
        }

        RE::BSEventNotifyControl ProcessEvent(
            RE::InputEvent* const*               a_events,
            RE::BSTEventSource<RE::InputEvent*>*) override
        {
            if (!a_events)
                return RE::BSEventNotifyControl::kContinue;

            auto* ui = RE::UI::GetSingleton();
            if (!ui || !ui->IsMenuOpen(RE::Console::MENU_NAME))
                return RE::BSEventNotifyControl::kContinue;

            for (auto* ev = *a_events; ev; ev = ev->next) {
                if (ev->GetEventType() != RE::INPUT_EVENT_TYPE::kButton)
                    continue;

                const auto* btn = static_cast<const RE::ButtonEvent*>(ev);

                if (Config::triggerMode == Config::TriggerMode::kHotkey &&
                    Config::hotkeyCode  != 0 &&
                    btn->GetDevice()    == RE::INPUT_DEVICE::kKeyboard &&
                    btn->GetIDCode()    == Config::hotkeyCode &&
                    btn->IsDown())
                {
                    SKSE::GetTaskInterface()->AddTask(ReadCurrentConsoleRef);
                    SKSE::GetTaskInterface()->AddTask(TrySendToXEdit);
                    continue;
                }

                if (btn->GetDevice() != RE::INPUT_DEVICE::kMouse ||
                    btn->GetIDCode()  != 0 ||
                    !btn->IsDown())
                    continue;

                switch (Config::triggerMode) {

                case Config::TriggerMode::kSingleClick:
                    SKSE::GetTaskInterface()->AddTask(ReadCurrentConsoleRef);
                    SKSE::GetTaskInterface()->AddTask(TrySendToXEdit);
                    break;

                case Config::TriggerMode::kDoubleClick:
                default: {
                    const auto now = std::chrono::steady_clock::now();
                    const bool dbl = (now - _lastClick) < std::chrono::milliseconds(300);
                    _lastClick     = now;
                    SKSE::GetTaskInterface()->AddTask(ReadCurrentConsoleRef);
                    if (dbl)
                        SKSE::GetTaskInterface()->AddTask(TrySendToXEdit);
                    break;
                }
                }
            }

            return RE::BSEventNotifyControl::kContinue;
        }

    private:
        std::chrono::steady_clock::time_point _lastClick{};
    };

    class MenuEventSink final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        [[nodiscard]] static MenuEventSink* GetSingleton() noexcept
        {
            static MenuEventSink s;
            return &s;
        }

        RE::BSEventNotifyControl ProcessEvent(
            const RE::MenuOpenCloseEvent*               a_event,
            RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
        {
            if (a_event &&
                a_event->menuName == RE::Console::MENU_NAME &&
                !a_event->opening)
            {
                g_lastReadFormID = 0;
                g_pendingRef     = {};
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };

    void RegisterEventSinks()
    {
        RE::BSInputDeviceManager::GetSingleton()
            ->AddEventSink(InputEventSink::GetSingleton());
        RE::UI::GetSingleton()
            ->AddEventSink<RE::MenuOpenCloseEvent>(MenuEventSink::GetSingleton());
    }

}  // namespace

SKSEPluginVersion = []() noexcept -> SKSE::PluginVersionData
{
    SKSE::PluginVersionData v;
    v.PluginName(Plugin::NAME);
    v.AuthorName("Modding Forge"sv);
    v.PluginVersion(Plugin::VERSION);
    v.UsesAddressLibrary();
    v.UsesNoStructs();
    return v;
}();

SKSE_EXPORT bool SKSEPlugin_Query(
    const SKSE::QueryInterface*, SKSE::PluginInfo* a_info)
{
    a_info->name        = SKSEPlugin_Version.pluginName;
    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->version     = SKSEPlugin_Version.pluginVersion;
    return true;
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);

    const auto dataPath = GetDataPath();
    InitXEditLinker(dataPath);
    Config::Load((dataPath / L"SKSE\\Plugins\\SSEEditLinker.ini").wstring());

    SKSE::GetMessagingInterface()->RegisterListener(
        [](SKSE::MessagingInterface::Message* msg) {
            if (msg->type == SKSE::MessagingInterface::kInputLoaded)
                RegisterEventSinks();
        });

    return true;
}