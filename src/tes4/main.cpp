#include <cstdint>
#include "xedit_linker.h"

// OBSE / xOBSE headers - compiled only when OBSE_SDK_PATH is set in CMake.
// Repository: https://github.com/llde/xOBSE
// Pass -DOBSE_SDK_PATH=<path> to enable.
#if defined(OBSE_AVAILABLE)
#   include "obse/PluginAPI.h"
#   include "obse/GameTypes.h"
#endif

#if defined(OBSE_AVAILABLE)
extern "C"
{
    __declspec(dllexport) bool OBSEPlugin_Query(
        const OBSEInterface* a_obse,
        PluginInfo*          a_info)
    {
        a_info->infoVersion = PluginInfo::kInfoVersion;
        a_info->name        = "xEdit-Linker";
        a_info->version     = 1;
        return true;
    }

    __declspec(dllexport) bool OBSEPlugin_Load(const OBSEInterface* a_obse)
    {
        // TODO: Register hooks
        return true;
    }
}  // extern "C"
#endif
