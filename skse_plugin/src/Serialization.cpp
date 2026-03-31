#include "AcquiredTracker.h"

#include <cstdint>

namespace skyui_recent
{
    namespace serialization
    {
        inline constexpr std::uint32_t kRecordType = 0x41435144; // ACQD
        inline constexpr std::uint32_t kVersion = 1;

        struct SaveRecord
        {
            std::uint32_t formID;
            std::uint32_t uniqueID;
            std::int64_t acquiredAt;
        };

        // Wire these to SKSE::SerializationInterface callbacks in production.
        void Save()
        {
            AcquiredTracker::GetSingleton().ForEach([](std::uint32_t, std::uint32_t, std::int64_t) {
                // WriteRecordData(...) goes here.
            });
        }

        void Load()
        {
            // ReadRecordData(...) loop goes here.
            // AcquiredTracker::GetSingleton().MarkItemAdded(record.formID, record.uniqueID, record.acquiredAt);
        }

        void Revert()
        {
            AcquiredTracker::GetSingleton().Clear();
        }
    }
}
