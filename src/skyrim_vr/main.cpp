#include <SKSE/SKSE.h>
#include "xedit_linker.h"

namespace
{
    /// Returns the FormID of the reference currently selected in the VR
    /// in-game console.  Implementation pending - returns 0 as a placeholder.
    std::uint32_t GetConsoleRefFormID()
    {
        // TODO: Read the console-selected reference via RE::Console / SKSEVR
        return 0;
    }
}  // namespace

/// Plugin version descriptor (required by SKSEVR / Address Library).
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept
{
    SKSE::PluginVersionData v;
    v.PluginName("xEdit-Linker");
    v.PluginVersion({ 1, 0, 0 });
    v.UsesAddressLibrary();
    return v;
}();

/// Plugin entry point called by SKSEVR after all game masters have loaded.
extern "C" DLLEXPORT bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);
    // TODO: Register hooks / event sinks (VR-specific)
    return true;
}
