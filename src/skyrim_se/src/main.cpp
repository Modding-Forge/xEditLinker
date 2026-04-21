#include "xedit_linker.h"

#include <chrono>

namespace Config
{
    enum class TriggerMode { DoubleClick = 1, Hotkey = 2, SingleClick = 3 };

    std::wstring  xEditPath;
    TriggerMode   triggerMode = TriggerMode::Hotkey;
    std::uint32_t hotkeyCode  = 0;   // DirectInput keyboard scancode; 0 = disabled

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
        if      (_wcsicmp(modeBuf, L"SingleClick")  == 0) triggerMode = TriggerMode::SingleClick;
        else if (_wcsicmp(modeBuf, L"DoubleClick")  == 0) triggerMode = TriggerMode::DoubleClick;
        else                                               triggerMode = TriggerMode::Hotkey;

        // HotkeyCode (decimal DX scancode; only used with TriggerMode=Hotkey)
        hotkeyCode = static_cast<std::uint32_t>(
            ::GetPrivateProfileIntW(L"xEditLinker", L"HotkeyCode", 60, ini));
    }
}

namespace
{
    std::filesystem::path GetDataPath()
    {
        wchar_t buf[MAX_PATH];
        ::GetModuleFileNameW(nullptr, buf, MAX_PATH);
        return std::filesystem::path(buf).parent_path() / L"Data";
    }

    bool g_xEditRunning = false;

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
            ::CloseHandle(pi.hProcess);
            g_xEditRunning = true;
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

    // Stores the ref to send once the user confirms xEdit launch.
    PendingRef g_pendingLaunchRef;

    // RE::CreateMessage's second parameter is typed as IMessageBoxCallback* in CommonLib,
    // but Skyrim internally stores it as OldMessageBoxCallback::Callback* (raw function ptr).
    // Passing a vtable object causes a crash - pass a static function cast instead.
    void XelLaunchCallbackFn(RE::IMessageBoxCallback::Message a_msg)
    {
        // Button 0 = first button ("Yes"), anything else = cancel/No
        if (a_msg == RE::IMessageBoxCallback::Message::kUnk0) {
            LaunchXEdit();
            if (g_xEditRunning && g_pendingLaunchRef.refID != 0)
                OpenInXEdit(g_pendingLaunchRef.refID, g_pendingLaunchRef.baseID);
        }
        g_pendingLaunchRef = {};
    }

    void TrySendToXEdit()
    {
        if (!g_xEditRunning) {

            if (Config::xEditPath.empty()) {
                RE::ConsoleLog::GetSingleton()->Print(
                    "[xEditLinker] xEditPath not configured. "
                    "Edit Data\\SKSE\\Plugins\\SSEEditLinker.ini");
                return;
            }
            if (!std::filesystem::exists(Config::xEditPath)) {
                const int len = ::WideCharToMultiByte(
                    CP_UTF8, 0, Config::xEditPath.c_str(), -1, nullptr, 0, nullptr, nullptr);
                std::string narrow(len > 0 ? len - 1 : 0, '\0');
                if (len > 0)
                    ::WideCharToMultiByte(CP_UTF8, 0, Config::xEditPath.c_str(), -1,
                        narrow.data(), len, nullptr, nullptr);
                RE::ConsoleLog::GetSingleton()->Print(
                    "[xEditLinker] xEdit not found at: %s", narrow.c_str());
                return;
            }

            // Save the pending ref so the callback can send it after launch
            g_pendingLaunchRef = g_pendingRef;

            const int len = ::WideCharToMultiByte(
                CP_UTF8, 0, Config::xEditPath.c_str(), -1, nullptr, 0, nullptr, nullptr);
            std::string pathUtf8(len > 0 ? len - 1 : 0, '\0');
            if (len > 0)
                ::WideCharToMultiByte(CP_UTF8, 0, Config::xEditPath.c_str(), -1,
                    pathUtf8.data(), len, nullptr, nullptr);

            // CreateMessage uses a static body string pointer - store in static to keep alive
            static std::string s_msgBody;
            s_msgBody = "Start xEdit?\n" + pathUtf8 +
                        "\n\nSkyrim will freeze for a couple of seconds.";

            RE::CreateMessage(
                s_msgBody.c_str(),
                reinterpret_cast<RE::IMessageBoxCallback*>(XelLaunchCallbackFn),
                0, 0, 0,
                "Yes", "No");
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

                if (Config::triggerMode == Config::TriggerMode::Hotkey &&
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

                case Config::TriggerMode::SingleClick:
                    SKSE::GetTaskInterface()->AddTask(ReadCurrentConsoleRef);
                    SKSE::GetTaskInterface()->AddTask(TrySendToXEdit);
                    break;

                case Config::TriggerMode::DoubleClick:
                default: {
                    const auto now = std::chrono::steady_clock::now();
                    const bool dbl = (now - m_lastClick) < std::chrono::milliseconds(300);
                    m_lastClick    = now;
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
        std::chrono::steady_clock::time_point m_lastClick{};
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