#include "xedit_linker.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdio>
#include <filesystem>
#include <system_error>

namespace
{
    std::filesystem::path g_iniPath;

    // Writes [Console]\r\nselectedRefID=XXXXXXXX\r\nselectedBaseID=XXXXXXXX\r\n
    // as a single atomic CreateFile/WriteFile/CloseHandle sequence.
    // CloseHandle triggers FindFirstChangeNotification in xEdit's TGameLinkThread.
    void WriteIni(std::uint32_t refFormID, std::uint32_t baseFormID)
    {
        if (g_iniPath.empty())
            return;

        char buf[80];
        const int len = std::snprintf(
            buf, sizeof(buf),
            "[Console]\r\nselectedRefID=%08X\r\nselectedBaseID=%08X\r\n",
            refFormID, baseFormID);

        if (len <= 0)
            return;

        const HANDLE hFile = ::CreateFileW(
            g_iniPath.c_str(),
            GENERIC_WRITE,
            FILE_SHARE_READ,   // compatible with xEdit's fmOpenRead|fmShareDenyNone
            nullptr,
            CREATE_ALWAYS,     // truncate + recreate in one handle lifetime
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (hFile == INVALID_HANDLE_VALUE)
            return;

        DWORD written{};
        ::WriteFile(hFile, buf, static_cast<DWORD>(len), &written, nullptr);
        ::CloseHandle(hFile);
    }
}  // namespace

void InitXEditLinker(const std::filesystem::path& dataPath)
{
    const auto xeditDir = dataPath / L"xEdit";

    std::error_code ec;
    std::filesystem::create_directories(xeditDir, ec);
    // ec is intentionally ignored: create_directories returns false (not an
    // error) when the directory already exists.

    g_iniPath = xeditDir / L"xEditLink.ini";

    // Write a stub so xEdit's GameLink menu item becomes visible on next launch.
    // The menu item is gated on: FileExists(wbDataPath + 'xEdit\xEditLink.ini')
    if (!std::filesystem::exists(g_iniPath, ec))
        WriteIni(0, 0);
}

void OpenInXEdit(std::uint32_t refFormID, std::uint32_t baseFormID)
{
    WriteIni(refFormID, baseFormID);
}
