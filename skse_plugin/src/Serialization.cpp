#include "AcquiredTracker.h"
#include "Serialization.h"

#include <SKSE/SKSE.h>
#include <RE/Skyrim.h>

namespace skyui_recent::serialization
{
    struct SaveRecord
    {
        std::uint32_t formID;
        std::uint32_t uniqueID;
        std::int64_t  acquiredAt;
    };

    void OnSave(SKSE::SerializationInterface* a_intfc)
    {
        AcquiredTracker::GetSingleton().ForEach(
            [a_intfc](std::uint32_t formID, std::uint32_t uniqueID, std::int64_t ts) {
                if (a_intfc->OpenRecord(kRecordType, kVersion)) {
                    const SaveRecord rec{ formID, uniqueID, ts };
                    a_intfc->WriteRecordData(rec);
                }
            });
    }

    void OnLoad(SKSE::SerializationInterface* a_intfc)
    {
        bool hasData = false;
        std::uint32_t type = 0, version = 0, length = 0;
        while (a_intfc->GetNextRecordInfo(type, version, length)) {
            if (type != kRecordType || length != sizeof(SaveRecord)) {
                continue;
            }
            SaveRecord rec{};
            if (a_intfc->ReadRecordData(rec) != sizeof(rec)) {
                continue;
            }
            RE::FormID newFormID = 0;
            if (!a_intfc->ResolveFormID(rec.formID, newFormID)) {
                continue;
            }
            AcquiredTracker::GetSingleton().RestoreItem(newFormID, rec.uniqueID, rec.acquiredAt);
            hasData = true;
        }
        
        // If we loaded save data, mark as initialized (don't randomize on first menu open)
        if (hasData) {
            AcquiredTracker::GetSingleton().SetInitialized(true);
        }
    }

    void OnRevert(SKSE::SerializationInterface*)
    {
        AcquiredTracker::GetSingleton().Clear();
    }
}
