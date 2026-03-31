#pragma once

#include <cstdint>

namespace SKSE { class SerializationInterface; }

namespace skyui_recent::serialization
{
    inline constexpr std::uint32_t kPluginID   = 'RUCS'; // 'SUIR' in little-endian
    inline constexpr std::uint32_t kRecordType = 0x41435144; // 'ACQD'
    inline constexpr std::uint32_t kVersion    = 1;

    void OnSave(SKSE::SerializationInterface* a_intfc);
    void OnLoad(SKSE::SerializationInterface* a_intfc);
    void OnRevert(SKSE::SerializationInterface* a_intfc);
}
