#include <mutex>
#include "AcquiredTracker.h"

namespace skyui_recent
{
    AcquiredTracker& AcquiredTracker::GetSingleton()
    {
        static AcquiredTracker singleton;
        return singleton;
    }

    void AcquiredTracker::MarkItemAdded(std::uint32_t formID, std::uint32_t extraUniqueID, std::int64_t unixTimeSeconds)
    {
        std::unique_lock lock(_lock);
        _timestamps[ItemKey{ formID, extraUniqueID }] = unixTimeSeconds;
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
    }
}
