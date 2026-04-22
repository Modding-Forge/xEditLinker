// Copyright (c) Modding Forge
// Public API for the xEdit IPC layer shared across all game targets.
#pragma once

#include <cstdint>
#include <filesystem>

/// <summary>
/// Creates Data\xEdit\ and a stub xEditLink.ini so xEdit's GameLink
/// menu item (gated on FileExists) becomes visible. Call once on load.
/// </summary>
/// <param name="dataPath">Absolute path to the game's Data\ directory.</param>
void InitXEditLinker(const std::filesystem::path& dataPath);

/// <summary>
/// Writes FormIDs to Data\xEdit\xEditLink.ini so a running xEdit instance
/// navigates to the matching record.
/// xEdit must be running with -autogamelink or GameLink enabled via menu.
/// </summary>
/// <param name="refFormID">Load-order FormID of the selected reference.</param>
/// <param name="baseFormID">Load-order FormID of the base object (0 if unknown).</param>
void OpenInXEdit(std::uint32_t refFormID, std::uint32_t baseFormID = 0);
