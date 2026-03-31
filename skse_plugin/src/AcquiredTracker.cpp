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

    void AcquiredTracker::Clear()
    {
        std::unique_lock lock(_lock);
        _timestamps.clear();
    }
}
