#include <cstdint>
#include "xedit_linker.h"

// SKSE32 headers - compiled only when SKSE32_SDK_PATH is set in CMake.
// Download: https://skse.silverlock.org/  (Classic / LE build)
// Pass -DSKSE32_SDK_PATH=<path-to-sdk/src> to enable.
#if defined(SKSE32_AVAILABLE)
#   include "skse/PluginAPI.h"
#   include "common/ITypes.h"
#endif

#if defined(SKSE32_AVAILABLE)
extern "C"
{
    __declspec(dllexport) bool SKSEPlugin_Query(
        const SKSEInterface* a_skse,
        PluginInfo*          a_info)
    {
        a_info->infoVersion = PluginInfo::kInfoVersion;
        a_info->name        = "xEdit-Linker";
        a_info->version     = 1;
        return true;
    }

    __declspec(dllexport) bool SKSEPlugin_Load(const SKSEInterface* a_skse)
    {
        // TODO: Register hooks
        return true;
    }
}  // extern "C"
#endif
