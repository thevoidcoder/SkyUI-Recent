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

    bool Register(RE::BSScript::IVirtualMachine* a_vm)
    {
        a_vm->RegisterFunction("GetItemAcquiredTime", "SUIR_AcquiredBridge", GetItemAcquiredTime);
        return true;
    }
}
