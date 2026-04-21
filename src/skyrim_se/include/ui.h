#pragma once

#include <cstdint>
#include <string>

namespace UI
{
    /// Hooks IDXGISwapChain::Present and initialises ImGui.
    /// Call once after kInputLoaded (D3D11 is alive by then).
    void Init();

    /// Show the "Open in xEdit" overlay for the given ref.
    /// Must be called on the game's main thread.
    void SetPendingRef(std::uint32_t refID, std::uint32_t baseID, std::string name);

    /// Hide the overlay (call when the console closes).
    void ClearPendingRef();

    /// If a ref is currently shown in the overlay, write xEditLink.ini for it.
    /// Safe to call from any game-main-thread task.
    void SendPendingRef();
}
