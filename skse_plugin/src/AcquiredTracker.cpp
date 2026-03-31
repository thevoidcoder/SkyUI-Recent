#include <mutex>
#include "AcquiredTracker.h"

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
        _timestamps[ItemKey{ formID, extraUniqueID }] = ++_counter;
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
        for (const auto& [key, value] : _timestamps) {
            if (key.formID == formID && value > latest) {
                latest = value;
            }
        }
        return latest;
    }

    void AcquiredTracker::Clear()
    {
        std::unique_lock lock(_lock);
        _timestamps.clear();
        _counter = 0;
    }
}
