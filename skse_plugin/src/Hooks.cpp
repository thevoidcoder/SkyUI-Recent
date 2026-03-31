#include "PCH.h"
#include "Hooks.h"
#include "AcquiredTracker.h"

namespace skyui_recent::hooks
{
    namespace
    {
        using AddObjectToContainer_t = void(RE::Actor*, RE::TESBoundObject*,
                                            RE::ExtraDataList*, std::int32_t,
                                            RE::TESObjectREFR*);
        using PickUpObject_t = void(RE::Actor*, RE::TESObjectREFR*,
                                    std::uint32_t, bool, bool);

        REL::Relocation<AddObjectToContainer_t> _AddObjectToContainer;
        REL::Relocation<PickUpObject_t>         _PickUpObject;

        void AddObjectToContainer(RE::Actor* a_this, RE::TESBoundObject* a_object,
                                  RE::ExtraDataList* a_extraList, std::int32_t a_count,
                                  RE::TESObjectREFR* a_fromRefr)
        {
            _AddObjectToContainer(a_this, a_object, a_extraList, a_count, a_fromRefr);

            if (!a_object || a_this != RE::PlayerCharacter::GetSingleton()) {
                return;
            }

            std::uint16_t uniqueID = 0;
            if (a_extraList) {
                if (const auto* extra = a_extraList->GetByType<RE::ExtraUniqueID>()) {
                    uniqueID = extra->uniqueID;
                }
            }

            AcquiredTracker::GetSingleton().MarkItemAdded(
                a_object->GetFormID(), uniqueID);
            SKSE::log::trace("Tracked (container) {:08X} uid={}", a_object->GetFormID(), uniqueID);
        }

        void PickUpObject(RE::Actor* a_this, RE::TESObjectREFR* a_object,
                          std::uint32_t a_count, bool a_arg3, bool a_playSound)
        {
            _PickUpObject(a_this, a_object, a_count, a_arg3, a_playSound);

            if (!a_object || a_this != RE::PlayerCharacter::GetSingleton()) {
                return;
            }

            auto* base = a_object->GetBaseObject();
            if (!base) {
                return;
            }

            std::uint16_t uniqueID = 0;
            if (const auto* extra = a_object->extraList.GetByType<RE::ExtraUniqueID>()) {
                uniqueID = extra->uniqueID;
            }

            AcquiredTracker::GetSingleton().MarkItemAdded(
                base->GetFormID(), uniqueID);
            SKSE::log::trace("Tracked (pickup) {:08X} uid={}", base->GetFormID(), uniqueID);
        }
    }

    void Install()
    {
        REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_PlayerCharacter[0] };
        _AddObjectToContainer = vtbl.write_vfunc(0x5A, AddObjectToContainer);
        _PickUpObject          = vtbl.write_vfunc(0xCC, PickUpObject);
        SKSE::log::info("SkyUIRecentSort: pickup hooks installed.");
    }
}
