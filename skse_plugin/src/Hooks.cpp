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
            SKSE::log::trace("AddObjToContainer called actor={} obj={}", fmt::ptr(a_this), fmt::ptr(a_object));
            RE::FormID formID = 0;
            if (a_object && a_this == RE::PlayerCharacter::GetSingleton()) {
                formID = a_object->GetFormID();
                SKSE::log::trace("AddObjToContainer formID={:08X}", formID);
            }

            _AddObjectToContainer(a_this, a_object, a_extraList, a_count, a_fromRefr);

            if (formID != 0) {
                AcquiredTracker::GetSingleton().MarkItemAdded(formID, 0);
                SKSE::log::trace("Tracked (container) {:08X}", formID);
            }
        }

        void PickUpObject(RE::Actor* a_this, RE::TESObjectREFR* a_object,
                          std::uint32_t a_count, bool a_arg3, bool a_playSound)
        {
            RE::FormID baseFormID = 0;
            if (a_object && a_this == RE::PlayerCharacter::GetSingleton()) {
                if (auto* base = a_object->GetBaseObject()) {
                    baseFormID = base->GetFormID();
                    SKSE::log::trace("PickUpObject: baseFormID={:08X}", baseFormID);
                }
            }

            _PickUpObject(a_this, a_object, a_count, a_arg3, a_playSound);

            if (baseFormID != 0) {
                AcquiredTracker::GetSingleton().MarkItemAdded(baseFormID, 0);
                SKSE::log::trace("Tracked (pickup) {:08X}", baseFormID);
            }
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
