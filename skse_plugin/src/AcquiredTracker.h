#pragma once

#include <cstdint>
#include <shared_mutex>
#include <unordered_map>

namespace skyui_recent
{
    struct ItemKey
    {
        std::uint32_t formID{ 0 };
        std::uint32_t extraUniqueID{ 0 };

        [[nodiscard]] bool operator==(const ItemKey& rhs) const noexcept
        {
            return formID == rhs.formID && extraUniqueID == rhs.extraUniqueID;
        }
    };

    struct ItemKeyHash
    {
        std::size_t operator()(const ItemKey& key) const noexcept
        {
            return (static_cast<std::size_t>(key.formID) << 1) ^ key.extraUniqueID;
        }
    };

    class AcquiredTracker
    {
    public:
        static AcquiredTracker& GetSingleton();

        void MarkItemAdded(std::uint32_t formID, std::uint32_t extraUniqueID, std::int64_t unixTimeSeconds);
        [[nodiscard]] std::int64_t GetAcquiredTime(std::uint32_t formID, std::uint32_t extraUniqueID) const;

        template <class Fn>
        void ForEach(Fn&& fn) const
        {
            std::shared_lock lock(_lock);
            for (const auto& [key, value] : _timestamps) {
                fn(key.formID, key.extraUniqueID, value);
            }
        }

        [[nodiscard]] std::int64_t GetLatestAcquiredTime(std::uint32_t formID) const;

        void Clear();

    private:
        mutable std::shared_mutex _lock;
        std::unordered_map<ItemKey, std::int64_t, ItemKeyHash> _timestamps;
    };
}
