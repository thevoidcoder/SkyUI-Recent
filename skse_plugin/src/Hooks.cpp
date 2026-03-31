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
            // Capture data BEFORE calling original; a_extraList may be consumed by it.
            RE::FormID formID = 0;
            std::uint16_t uniqueID = 0;
            if (a_object && a_this == RE::PlayerCharacter::GetSingleton()) {
                formID = a_object->GetFormID();
                if (a_extraList) {
                    if (const auto* extra = a_extraList->GetByType<RE::ExtraUniqueID>()) {
                        uniqueID = extra->uniqueID;
                    }
                }
            }

            _AddObjectToContainer(a_this, a_object, a_extraList, a_count, a_fromRefr);

            if (formID != 0) {
                AcquiredTracker::GetSingleton().MarkItemAdded(formID, uniqueID);
                SKSE::log::trace("Tracked (container) {:08X} uid={}", formID, uniqueID);
            }
        }

        void PickUpObject(RE::Actor* a_this, RE::TESObjectREFR* a_object,
                          std::uint32_t a_count, bool a_arg3, bool a_playSound)
        {
            // Capture data BEFORE calling original; a_object (world ref) is
            // removed from the scene by the original and becomes a dangling ptr.
            RE::FormID baseFormID = 0;
            std::uint16_t uniqueID = 0;
            if (a_object && a_this == RE::PlayerCharacter::GetSingleton()) {
                if (auto* base = a_object->GetBaseObject()) {
                    baseFormID = base->GetFormID();
                    if (const auto* extra = a_object->extraList.GetByType<RE::ExtraUniqueID>()) {
                        uniqueID = extra->uniqueID;
                    }
                }
            }

            _PickUpObject(a_this, a_object, a_count, a_arg3, a_playSound);

            if (baseFormID != 0) {
                AcquiredTracker::GetSingleton().MarkItemAdded(baseFormID, uniqueID);
                SKSE::log::trace("Tracked (pickup) {:08X} uid={}", baseFormID, uniqueID);
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
