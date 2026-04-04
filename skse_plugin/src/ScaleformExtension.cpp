#include "AcquiredTracker.h"
#include "ScaleformExtension.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

namespace skyui_recent::scaleform
{
    void OnInventoryItem(
        RE::GFxMovieView*,
        RE::GFxValue*           a_object,
        RE::InventoryEntryData* a_item)
    {
        if (!a_item || !a_object || !a_object->IsObject()) {
            return;
        }

        const auto* form = a_item->GetObject();
        if (!form) {
            return;
        }

        // DISABLED: RandomizeExistingInventory causes crashes
        // Pre-existing items will show timestamp 0 until acquired again
        // TODO: Find safe way to initialize inventory or accept 0 for pre-existing items
        auto& tracker = AcquiredTracker::GetSingleton();

        const auto formID = form->GetFormID();
        SKSE::log::trace("OnInventoryItem: {:08X}", formID);

        const auto ts = tracker.GetLatestAcquiredTime(formID);

        RE::GFxValue val{ static_cast<double>(ts) };
        a_object->SetMember("acquiredTime", val);
        SKSE::log::trace("OnInventoryItem: {:08X} ts={} done", formID, ts);
    }
}
