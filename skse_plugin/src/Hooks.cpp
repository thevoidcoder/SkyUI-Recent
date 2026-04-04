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

        using PlayPickupSoundAndMessage_t = void(RE::TESBoundObject*, std::int32_t, bool, bool, void*);
        std::uintptr_t _PlayPickupSoundAndMessage_Flora1 = 0;
        std::uintptr_t _PlayPickupSoundAndMessage_Flora2 = 0;
        std::uintptr_t _PlayPickupSoundAndMessage_AddItemFunctor = 0;
        std::uintptr_t _PlayPickupSoundAndMessage_AddItem = 0;

        void AddObjectToContainer(RE::Actor* a_this, RE::TESBoundObject* a_object,
                                  RE::ExtraDataList* a_extraList, std::int32_t a_count,
                                  RE::TESObjectREFR* a_fromRefr)
        {
            RE::FormID formID = 0;
            if (a_object && a_this == RE::PlayerCharacter::GetSingleton()) {
                // Skip if UI not available or game is loading - prevents cell loading issues
                auto* ui = RE::UI::GetSingleton();
                if (!ui || ui->GameIsPaused()) {
                    _AddObjectToContainer(a_this, a_object, a_extraList, a_count, a_fromRefr);
                    return;
                }
                
                formID = a_object->GetFormID();
                const char* source = a_fromRefr ? "container/NPC" : "direct/quest";
                SKSE::log::trace("AddObjToContainer: player receiving {:08X} x{} from {}",
                                formID, a_count, source);
            }

            _AddObjectToContainer(a_this, a_object, a_extraList, a_count, a_fromRefr);

            if (formID != 0) {
                AcquiredTracker::GetSingleton().MarkItemAdded(formID, 0);
                SKSE::log::trace("Tracked (AddObjToContainer) {:08X}", formID);
            }
        }

        void PickUpObject(RE::Actor* a_this, RE::TESObjectREFR* a_object,
                          std::uint32_t a_count, bool a_arg3, bool a_playSound)
        {
            RE::FormID baseFormID = 0;
            if (a_object && a_this == RE::PlayerCharacter::GetSingleton()) {
                // Skip if UI not available or game is loading
                auto* ui = RE::UI::GetSingleton();
                if (!ui || ui->GameIsPaused()) {
                    _PickUpObject(a_this, a_object, a_count, a_arg3, a_playSound);
                    return;
                }
                
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

        void PlayPickupSoundAndMessage_Flora1(RE::TESBoundObject* a_object, std::int32_t a_count, bool a3, bool a4, void* a5)
        {
            reinterpret_cast<PlayPickupSoundAndMessage_t*>(_PlayPickupSoundAndMessage_Flora1)(a_object, a_count, a3, a4, a5);
            if (a_object) {
                AcquiredTracker::GetSingleton().MarkItemAdded(a_object->GetFormID(), 0);
                SKSE::log::trace("Tracked (flora) {:08X} x{}", a_object->GetFormID(), a_count);
            }
        }

        void PlayPickupSoundAndMessage_Flora2(RE::TESBoundObject* a_object, std::int32_t a_count, bool a3, bool a4, void* a5)
        {
            reinterpret_cast<PlayPickupSoundAndMessage_t*>(_PlayPickupSoundAndMessage_Flora2)(a_object, a_count, a3, a4, a5);
            if (a_object) {
                AcquiredTracker::GetSingleton().MarkItemAdded(a_object->GetFormID(), 0);
                SKSE::log::trace("Tracked (flora) {:08X} x{}", a_object->GetFormID(), a_count);
            }
        }

        void PlayPickupSoundAndMessage_AddItemFunctor(RE::TESBoundObject* a_object, std::int32_t a_count, bool a3, bool a4, void* a5)
        {
            reinterpret_cast<PlayPickupSoundAndMessage_t*>(_PlayPickupSoundAndMessage_AddItemFunctor)(a_object, a_count, a3, a4, a5);
            if (a_object) {
                AcquiredTracker::GetSingleton().MarkItemAdded(a_object->GetFormID(), 0);
                SKSE::log::trace("Tracked (additem) {:08X} x{}", a_object->GetFormID(), a_count);
            }
        }

        void PlayPickupSoundAndMessage_AddItem(RE::TESBoundObject* a_object, std::int32_t a_count, bool a3, bool a4, void* a5)
        {
            reinterpret_cast<PlayPickupSoundAndMessage_t*>(_PlayPickupSoundAndMessage_AddItem)(a_object, a_count, a3, a4, a5);
            if (a_object) {
                AcquiredTracker::GetSingleton().MarkItemAdded(a_object->GetFormID(), 0);
                SKSE::log::trace("Tracked (additem2) {:08X} x{}", a_object->GetFormID(), a_count);
            }
        }
    }

    void Install()
    {
        REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_PlayerCharacter[0] };
        _AddObjectToContainer = vtbl.write_vfunc(0x5A, AddObjectToContainer);
        _PickUpObject          = vtbl.write_vfunc(0xCC, PickUpObject);

        REL::Relocation<std::uintptr_t> floraHook{ RELOCATION_ID(14692, 14864) };
        auto& trampoline = SKSE::GetTrampoline();
        _PlayPickupSoundAndMessage_Flora1 = trampoline.write_call<5>(
            floraHook.address() + 0x260, reinterpret_cast<std::uintptr_t>(PlayPickupSoundAndMessage_Flora1));
        _PlayPickupSoundAndMessage_Flora2 = trampoline.write_call<5>(
            floraHook.address() + 0x3CD, reinterpret_cast<std::uintptr_t>(PlayPickupSoundAndMessage_Flora2));

        REL::Relocation<std::uintptr_t> addItemFunctorHook{ RELOCATION_ID(55946, 56490) };
        _PlayPickupSoundAndMessage_AddItemFunctor = trampoline.write_call<5>(
            addItemFunctorHook.address() + 0x1C6, reinterpret_cast<std::uintptr_t>(PlayPickupSoundAndMessage_AddItemFunctor));

        REL::Relocation<std::uintptr_t> addItemHook{ RELOCATION_ID(15887, 16127) };
        _PlayPickupSoundAndMessage_AddItem = trampoline.write_call<5>(
            addItemHook.address() + 0x182, reinterpret_cast<std::uintptr_t>(PlayPickupSoundAndMessage_AddItem));

        SKSE::log::info("SkyUIRecentSort: pickup hooks installed.");
    }
}
