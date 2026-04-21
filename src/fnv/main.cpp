#include <cstdint>
#include "xedit_linker.h"

// NVSE headers - compiled only when NVSE_SDK_PATH is set in CMake.
// Repository: https://github.com/xNVSE/NVSE
// Pass -DNVSE_SDK_PATH=<path> to enable.
#if defined(NVSE_AVAILABLE)
#   include "nvse/PluginAPI.h"
#   include "nvse/GameTypes.h"
#endif

#if defined(NVSE_AVAILABLE)
extern "C"
{
    __declspec(dllexport) bool NVSEPlugin_Query(
        const NVSEInterface* a_nvse,
        PluginInfo*          a_info)
    {
        a_info->infoVersion = PluginInfo::kInfoVersion;
        a_info->name        = "xEdit-Linker";
        a_info->version     = 1;
        return true;
    }

    __declspec(dllexport) bool NVSEPlugin_Load(const NVSEInterface* a_nvse)
    {
        // TODO: Register hooks
        return true;
    }
}  // extern "C"
#endif
