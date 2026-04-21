# xEdit-Linker

Script extender plugin for Bethesda games that bridges the in-game console to [xEdit](https://github.com/TES5Edit/TES5Edit) (SSEEdit, FO4Edit, etc.).

Select a reference in the console, press **F2** (configurable), and xEdit navigates to the matching record. If xEdit is not running yet, an in-game confirmation prompt launches it automatically.

> **Status:** Skyrim SE / AE is fully implemented. All other game targets are stubs - the IPC mechanism is in place, but the script extender hooks still need to be wired up.

## How it works

The plugin writes the selected reference's FormID to `Data\xEdit\xEditLink.ini`. xEdit's built-in GameLink thread watches that file and navigates to the matching record.

---

## Supported games

| Directory         | Game               | Script extender                        | Mod page                                                                    | Status          |
| ----------------- | ------------------ | -------------------------------------- | --------------------------------------------------------------------------- | --------------- |
| `src/skyrim_se` | Skyrim SE / AE     | [SKSE64](https://skse.silverlock.org/) | [Nexus](https://www.nexusmods.com/skyrimspecialedition/mods/178053)         | **Implemented** |
| `src/skyrim_vr` | Skyrim VR          | [SKSEVR](https://skse.silverlock.org/) |                                                                             | Stub            |
| `src/skyrim_le` | Skyrim LE          | [SKSE32](https://skse.silverlock.org/) |                                                                             | Stub            |
| `src/fo4`       | Fallout 4          | [F4SE](https://f4se.silverlock.org/)   |                                                                             | Stub            |
| `src/fo4_vr`    | Fallout 4 VR       | [F4SEVR](https://f4se.silverlock.org/) |                                                                             | Stub            |
| `src/fo3`       | Fallout 3          | [FOSE](https://fose.silverlock.org/)   |                                                                             | Stub            |
| `src/fnv`       | Fallout: New Vegas | [NVSE](https://github.com/xNVSE/NVSE)  |                                                                             | Stub            |
| `src/tes4`      | Oblivion           | [OBSE](https://github.com/llde/xOBSE)  |                                                                             | Stub            |
| `src/sf1`       | Starfield          | [SFSE](https://sfse.silverlock.org/)   |                                                                             | Stub            |

---

## Configuration

`Data\SKSE\Plugins\SSEEditLinker.ini`

| Setting         | Default        | Description                                     |
| --------------- | -------------- | ----------------------------------------------- |
| `xEditPath`   | *(required)* | Full path to the xEdit executable               |
| `TriggerMode` | `Hotkey`     | `Hotkey`, `DoubleClick`, or `SingleClick` |
| `HotkeyCode`  | `60` (F2)    | DirectInput scancode - Hotkey mode only         |

### Trigger modes

- **Hotkey** - Open the console, select a reference, press the configured key.
- **DoubleClick** - Open the console, double-click a reference.
- **SingleClick** - Open the console, single-click a reference.

---

## Building

### Prerequisites

- Visual Studio 2022 or later with the **Desktop development with C++** workload
- CMake ≥ 3.21

### Skyrim SE / AE

Requires [vcpkg](https://vcpkg.io/) with `VCPKG_ROOT` set and the `x64-windows-static-md` triplet.

```powershell
cmake --preset flatrim
cmake --build build-flatrim --config Release --target xedit_linker_skyrim_se
```

### Other 64-bit targets

FetchContent downloads the required CommonLib automatically. `skyrim_se`/`skyrim_vr` and `fo4`/`fo4_vr` share a FetchContent name and must be configured in separate build directories.

```powershell
cmake -B build-skyrim-vr -A x64 -DBUILD_SKYRIM_VR=ON
cmake --build build-skyrim-vr --config Release
```

### 32-bit targets

Legacy SDKs must be downloaded manually.

| Target        | CMake variable      | SDK                           |
| ------------- | ------------------- | ----------------------------- |
| `skyrim_le` | `SKSE32_SDK_PATH` | https://skse.silverlock.org/  |
| `fo3`       | `FOSE_SDK_PATH`   | https://fose.silverlock.org/  |
| `fnv`       | `NVSE_SDK_PATH`   | https://github.com/xNVSE/NVSE |
| `tes4`      | `OBSE_SDK_PATH`   | https://github.com/llde/xOBSE |

```powershell
cmake -B build-32 -A Win32 `
    -DBUILD_SKYRIM_LE=ON -DSKSE32_SDK_PATH="C:/SDKs/skse32/src" `
    -DBUILD_FNV=ON       -DNVSE_SDK_PATH="C:/SDKs/nvse"
cmake --build build-32 --config Release
```

---

## License

GPL-3.0 - Copyright 2026 Modding Forge
