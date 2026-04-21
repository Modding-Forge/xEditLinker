#include <F4SE/F4SE.h>
#include "xedit_linker.h"

namespace
{
    /// Returns the FormID of the reference currently selected in the in-game
    /// console.  Implementation pending - returns 0 as a placeholder.
    std::uint32_t GetConsoleRefFormID()
    {
        // TODO: Read the console-selected reference via RE::Console / F4SE
        return 0;
    }
}  // namespace

/// Plugin version descriptor (required by F4SE).
extern "C" DLLEXPORT constinit auto F4SEPlugin_Version = []() noexcept
{
    F4SE::PluginVersionData v;
    v.PluginName("xEdit-Linker");
    v.PluginVersion({ 1, 0, 0 });
    v.UsesAddressLibrary();
    return v;
}();

/// Plugin entry point called by F4SE after all game masters have loaded.
extern "C" DLLEXPORT bool F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
    F4SE::Init(a_f4se);
    // TODO: Register hooks / event sinks
    return true;
}
