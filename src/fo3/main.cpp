#include <cstdint>
#include "xedit_linker.h"

// FOSE headers - compiled only when FOSE_SDK_PATH is set in CMake.
// Download: https://fose.silverlock.org/
// Pass -DFOSE_SDK_PATH=<path> to enable.
#if defined(FOSE_AVAILABLE)
#   include "fose/PluginAPI.h"
#   include "fose/GameTypes.h"
#endif

#if defined(FOSE_AVAILABLE)
extern "C"
{
    __declspec(dllexport) bool FOSEPlugin_Query(
        const FOSEInterface* a_fose,
        PluginInfo*          a_info)
    {
        a_info->infoVersion = PluginInfo::kInfoVersion;
        a_info->name        = "xEdit-Linker";
        a_info->version     = 1;
        return true;
    }

    __declspec(dllexport) bool FOSEPlugin_Load(const FOSEInterface* a_fose)
    {
        // TODO: Register hooks
        return true;
    }
}  // extern "C"
#endif
