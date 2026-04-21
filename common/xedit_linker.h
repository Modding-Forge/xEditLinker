#pragma once

#include <cstdint>
#include <filesystem>

/**
 * @brief Creates Data\xEdit\ and a stub xEditLink.ini so xEdit's GameLink
 *        menu item (gated on FileExists) becomes visible. Call once on load.
 *
 * @param dataPath  Absolute path to the game's Data\ directory.
 */
void InitXEditLinker(const std::filesystem::path& dataPath);

/**
 * @brief Writes FormIDs to Data\xEdit\xEditLink.ini so a running xEdit
 *        instance navigates to the matching record.
 *
 * @note  xEdit must be running with -autogamelink or GameLink enabled via menu.
 * @param refFormID   Load-order FormID of the selected reference.
 * @param baseFormID  Load-order FormID of the base object (0 if unknown).
 */
void OpenInXEdit(std::uint32_t refFormID, std::uint32_t baseFormID = 0);
