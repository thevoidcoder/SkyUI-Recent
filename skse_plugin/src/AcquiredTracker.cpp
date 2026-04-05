#include <mutex>
#include "AcquiredTracker.h"
#include <RE/Skyrim.h>

namespace skyui_recent
{
    AcquiredTracker& AcquiredTracker::GetSingleton()
    {
        static AcquiredTracker singleton;
        return singleton;
    }

    void AcquiredTracker::MarkItemAdded(std::uint32_t formID, std::uint32_t extraUniqueID)
    {
        std::unique_lock lock(_lock);
        
        // Gold always gets timestamp 1 (bottom of recent list)
        if (formID == kGoldFormID) {
            ItemKey key{ formID, extraUniqueID };
            if (_timestamps.find(key) == _timestamps.end()) {
                _timestamps[key] = 1;
                SKSE::log::trace("MarkItemAdded: {:08X} (GOLD) -> timestamp 1 (fixed)", formID);
            }
            return;
        }
        
        // If same item as last, don't increment counter (avoid duplicate timestamps)
        if (formID == _lastItemFormID) {
            ItemKey key{ formID, extraUniqueID };
            auto it = _timestamps.find(key);
            if (it != _timestamps.end()) {
                SKSE::log::trace("MarkItemAdded: {:08X} (duplicate consecutive) -> reusing timestamp {}", formID, it->second);
                return;
            }
        }
        
        auto newTimestamp = ++_counter;
        _timestamps[ItemKey{ formID, extraUniqueID }] = newTimestamp;
        _lastItemFormID = formID;
        SKSE::log::trace("MarkItemAdded: {:08X} (extra={}) -> timestamp {}", formID, extraUniqueID, newTimestamp);
    }

    void AcquiredTracker::RestoreItem(std::uint32_t formID, std::uint32_t extraUniqueID, std::int64_t counterValue)
    {
        // TEMP: zero out legacy unix timestamps saved by older builds.
        // Unix timestamps for 2026 are ~1.74e9; real counters stay small.
        // TODO: remove this block once all saves have been migrated.
        static constexpr std::int64_t kUnixThreshold = 1'000'000'000LL;
        if (counterValue > kUnixThreshold) {
            counterValue = 0;
        }

        if (counterValue <= 0) {
            return;
        }

        std::unique_lock lock(_lock);
        _timestamps[ItemKey{ formID, extraUniqueID }] = counterValue;
        if (counterValue > _counter) {
            _counter = counterValue;
        }
    }

    std::int64_t AcquiredTracker::GetAcquiredTime(std::uint32_t formID, std::uint32_t extraUniqueID) const
    {
        std::shared_lock lock(_lock);
        if (const auto it = _timestamps.find(ItemKey{ formID, extraUniqueID }); it != _timestamps.end()) {
            return it->second;
        }
        return 0;
    }

    std::int64_t AcquiredTracker::GetLatestAcquiredTime(std::uint32_t formID) const
    {
        std::shared_lock lock(_lock);
        std::int64_t latest = 0;
        int matchCount = 0;
        for (const auto& [key, value] : _timestamps) {
            if (key.formID == formID && value > latest) {
                latest = value;
                matchCount++;
            }
        }
        if (matchCount > 0) {
            SKSE::log::trace("GetLatestAcquiredTime: {:08X} found {} entries, latest={}", formID, matchCount, latest);
        } else {
            SKSE::log::trace("GetLatestAcquiredTime: {:08X} NOT FOUND in {} tracked items", formID, _timestamps.size());
        }
        return latest;
    }

    void AcquiredTracker::Clear()
    {
        std::unique_lock lock(_lock);
        _timestamps.clear();
        _counter = 0;
        _lastItemFormID = 0;
    }

    void AcquiredTracker::RandomizeExistingInventory()
    {
        {
            std::shared_lock lock(_lock);
            if (_initialized) {
                SKSE::log::trace("AcquiredTracker: already initialized, skipping");
                return;
            }
        }

        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        auto inv = player->GetInventory();

        std::unique_lock lock(_lock);
        
        // Double-check after acquiring write lock
        if (_initialized) return;
        
        std::int64_t nextTimestamp = _counter + 1;
        int itemsProcessed = 0;

        for (const auto& [item, data] : inv) {
            if (!item) continue;
            
            auto formID = item->GetFormID();
            
            // Only assign if not already tracked
            ItemKey key{ formID, 0 };
            if (_timestamps.find(key) == _timestamps.end()) {
                _timestamps[key] = nextTimestamp++;
                itemsProcessed++;
            }
        }
        
        _counter = nextTimestamp - 1;
        _initialized = true;

        SKSE::log::info("AcquiredTracker: initialized {} pre-existing items with sequential timestamps", itemsProcessed);
    }

    std::int64_t AcquiredTracker::GetCounter() const
    {
        std::shared_lock lock(_lock);
        return _counter;
    }

    void AcquiredTracker::SetCounter(std::int64_t value)
    {
        std::unique_lock lock(_lock);
        _counter = value;
    }

    bool AcquiredTracker::IsInitialized() const
    {
        std::shared_lock lock(_lock);
        return _initialized;
    }

    void AcquiredTracker::SetInitialized(bool value)
    {
        std::unique_lock lock(_lock);
        _initialized = value;
    }
}
