#include "AcquiredTracker.h"
#include "PapyrusBindings.h"

#include <RE/Skyrim.h>

namespace skyui_recent::papyrus
{
    // Native signature: int function GetItemAcquiredTime(int formID, int uniqueID) global native
    static std::int32_t GetItemAcquiredTime(
        RE::StaticFunctionTag*,
        std::int32_t a_formID,
        std::int32_t a_uniqueID)
    {
        const auto formID   = static_cast<std::uint32_t>(a_formID);
        const auto uniqueID = static_cast<std::uint32_t>(a_uniqueID);
        auto ts = AcquiredTracker::GetSingleton().GetAcquiredTime(formID, uniqueID);
        if (ts == 0 && uniqueID != 0) {
            ts = AcquiredTracker::GetSingleton().GetLatestAcquiredTime(formID);
        }
        return static_cast<std::int32_t>(ts);
    }

    // Native function for MCM to manually fill timestamps for existing items
    // Returns: number of items processed, 0 if already initialized, -1 on error
    static std::int32_t FillExistingItemTimestamps(RE::StaticFunctionTag*)
    {
        try {
            auto& tracker = AcquiredTracker::GetSingleton();
            
            // Check if already initialized
            if (tracker.IsInitialized()) {
                SKSE::log::info("FillExistingItemTimestamps: Already initialized, skipping");
                return 0;
            }
            
            auto* player = RE::PlayerCharacter::GetSingleton();
            if (!player) {
                SKSE::log::error("FillExistingItemTimestamps: Player not found");
                return -1;
            }
            
            SKSE::log::info("FillExistingItemTimestamps: Starting manual timestamp fill from MCM");
            
            int itemsProcessed = 0;
            
            // Process inventory - safe when called from MCM (not during Scaleform callback)
            auto inv = player->GetInventory();
            
            for (const auto& [item, data] : inv) {
                if (!item) continue;
                
                auto formID = item->GetFormID();
                
                // Only assign if not already tracked
                if (tracker.GetLatestAcquiredTime(formID) == 0) {
                    tracker.MarkItemAdded(formID, 0);
                    itemsProcessed++;
                }
            }
            
            // Mark as initialized
            tracker.SetInitialized(true);
            
            SKSE::log::info("FillExistingItemTimestamps: Processed {} items", itemsProcessed);
            return static_cast<std::int32_t>(itemsProcessed);
            
        } catch (const std::exception& e) {
            SKSE::log::error("FillExistingItemTimestamps: Exception - {}", e.what());
            return -1;
        }
    }

    bool Register(RE::BSScript::IVirtualMachine* a_vm)
    {
        a_vm->RegisterFunction("GetItemAcquiredTime", "SUIR_AcquiredBridge", GetItemAcquiredTime);
        a_vm->RegisterFunction("FillExistingItemTimestamps", "SkyUIRecentSort_MCM", FillExistingItemTimestamps);
        SKSE::log::info("Papyrus: Registered native functions");
        return true;
    }
}
