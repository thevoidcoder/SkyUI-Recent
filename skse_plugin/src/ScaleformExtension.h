#pragma once

namespace RE
{
    class GFxMovieView;
    class GFxValue;
    class InventoryEntryData;
}

namespace skyui_recent::scaleform
{
    // Registered via SKSE::GetScaleformInterface()->Register(OnInventoryItem).
    // Called for every item when SkyUI opens the inventory (skse.ExtendData(true) is active).
    // Sets entry.acquiredTime = unix timestamp so config.txt @acquiredTime works.
    void OnInventoryItem(
        RE::GFxMovieView*       a_view,
        RE::GFxValue*           a_object,
        RE::InventoryEntryData* a_item);

    // Register menu event handler for deferred lazy tracking
    void RegisterMenuEventHandler();
}
