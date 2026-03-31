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

        const auto formID = form->GetFormID();

        // Try to find the instance-specific ExtraUniqueID first.
        std::uint32_t uniqueID = 0;
        if (a_item->extraLists) {
            for (const auto* extraList : *a_item->extraLists) {
                if (extraList) {
                    const auto* extra = extraList->GetByType<RE::ExtraUniqueID>();
                    if (extra) {
                        uniqueID = extra->uniqueID;
                        break;
                    }
                }
            }
        }

        auto ts = AcquiredTracker::GetSingleton().GetAcquiredTime(formID, uniqueID);
        if (ts == 0 && uniqueID != 0) {
            ts = AcquiredTracker::GetSingleton().GetLatestAcquiredTime(formID);
        }

        RE::GFxValue val{ static_cast<double>(ts) };
        if (!a_object->SetMember("acquiredTime", val)) {
            SKSE::log::warn("ScaleformExtension: SetMember(acquiredTime) failed for formID {:08X}", formID);
        }
    }
}
