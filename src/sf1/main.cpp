#include <SFSE/SFSE.h>
#include "xedit_linker.h"

namespace
{
    /// Returns the FormID of the reference currently selected in the in-game
    /// console.  Implementation pending - returns 0 as a placeholder.
    std::uint32_t GetConsoleRefFormID()
    {
        // TODO: Read the console-selected reference via RE::Console / SFSE
        return 0;
    }
}  // namespace

/// Plugin version descriptor (required by SFSE).
extern "C" DLLEXPORT constinit auto SFSEPlugin_Version = []() noexcept
{
    SFSE::PluginVersionData v;
    v.PluginName("xEdit-Linker");
    v.PluginVersion({ 1, 0, 0 });
    return v;
}();

/// Plugin entry point called by SFSE after all game masters have loaded.
extern "C" DLLEXPORT bool SFSEPlugin_Load(const SFSE::LoadInterface* a_sfse)
{
    SFSE::Init(a_sfse);
    // TODO: Register hooks / event sinks
    return true;
}
