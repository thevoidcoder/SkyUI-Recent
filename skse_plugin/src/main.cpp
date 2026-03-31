#include "AcquiredTracker.h"

#include <chrono>
#include <cstdint>

namespace skyui_recent
{
    namespace events
    {
        // Hook this from a player inventory change sink in production (e.g., TESContainerChangedEvent).
        void OnPlayerItemAdded(std::uint32_t formID, std::uint32_t uniqueID)
        {
            const auto now = std::chrono::time_point_cast<std::chrono::seconds>(
                std::chrono::system_clock::now());
            const auto unixSeconds = static_cast<std::int64_t>(now.time_since_epoch().count());
            AcquiredTracker::GetSingleton().MarkItemAdded(formID, uniqueID, unixSeconds);
        }
    }

    // SKSEPluginLoad(...) should register serialization, papyrus functions, and event listeners.
}
