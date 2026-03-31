#include "AcquiredTracker.h"

#include <cstdint>

namespace skyui_recent
{
    namespace papyrus
    {
        // Native signature expected by Papyrus:
        // int function GetItemAcquiredTime(int formID, int uniqueID) global native
        std::int32_t GetItemAcquiredTime(std::uint32_t formID, std::uint32_t uniqueID)
        {
            return static_cast<std::int32_t>(AcquiredTracker::GetSingleton().GetAcquiredTime(formID, uniqueID));
        }

        bool Register()
        {
            // Bind native function to VirtualMachine here.
            return true;
        }
    }
}
