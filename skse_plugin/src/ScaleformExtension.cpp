#include "AcquiredTracker.h"
#include "ScaleformExtension.h"

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <mutex>
#include <unordered_set>

namespace skyui_recent::scaleform
{
    namespace
    {
        std::mutex _deferredMutex;
        std::unordered_set<std::uint32_t> _deferredFormIDs;

        void ProcessDeferredLazyTracking()
        {
            std::unordered_set<std::uint32_t> toProcess;
            {
                std::lock_guard lock(_deferredMutex);
                toProcess = std::move(_deferredFormIDs);
                _deferredFormIDs.clear();
            }

            if (toProcess.empty()) {
                return;
            }

            auto* player = RE::PlayerCharacter::GetSingleton();
            if (!player) {
                SKSE::log::trace("ProcessDeferredLazyTracking: player is null");
                return;
            }

            auto inv = player->GetInventory();
            auto& tracker = AcquiredTracker::GetSingleton();
            int tracked = 0;

            for (const auto formID : toProcess) {
                // Find the form in inventory
                bool found = false;
                for (const auto& [item, data] : inv) {
                    if (item && item->GetFormID() == formID) {
                        found = true;
                        break;
                    }
                }

                if (found) {
                    tracker.MarkItemAdded(formID, 0);
                    tracked++;
                    SKSE::log::trace("ProcessDeferredLazyTracking: tracked {:08X}", formID);
                } else {
                    SKSE::log::trace("ProcessDeferredLazyTracking: {:08X} not in player inventory", formID);
                }
            }

            SKSE::log::info("ProcessDeferredLazyTracking: processed {} items, tracked {}", toProcess.size(), tracked);
        }

        class MenuEventHandler : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
        {
        public:
            static MenuEventHandler* GetSingleton()
            {
                static MenuEventHandler singleton;
                return &singleton;
            }

            RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
                                                   RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
            {
                if (!a_event) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                // Process deferred items when inventory-related menus close
                if (!a_event->opening) {
                    if (a_event->menuName == "InventoryMenu" ||
                        a_event->menuName == "ContainerMenu" ||
                        a_event->menuName == "BarterMenu") {
                        SKSE::log::trace("Menu closed: {}", a_event->menuName.c_str());
                        ProcessDeferredLazyTracking();
                    }
                }

                return RE::BSEventNotifyControl::kContinue;
            }

        private:
            MenuEventHandler() = default;
            MenuEventHandler(const MenuEventHandler&) = delete;
            MenuEventHandler(MenuEventHandler&&) = delete;
            MenuEventHandler& operator=(const MenuEventHandler&) = delete;
            MenuEventHandler& operator=(MenuEventHandler&&) = delete;
        };
    }
    void OnInventoryItem(
        RE::GFxMovieView*,
        RE::GFxValue*           a_object,
        RE::InventoryEntryData* a_item)
    {
        if (!a_item || !a_object || !a_object->IsObject()) {
            return;
        }

        auto* form = a_item->GetObject();
        if (!form) {
            return;
        }

        auto& tracker = AcquiredTracker::GetSingleton();

        const auto formID = form->GetFormID();
        SKSE::log::trace("OnInventoryItem: {:08X}", formID);

        auto ts = tracker.GetLatestAcquiredTime(formID);

        // Lazy tracking: queue items with ts=0 for deferred processing
        if (ts == 0) {
            std::lock_guard lock(_deferredMutex);
            _deferredFormIDs.insert(formID);
            SKSE::log::trace("OnInventoryItem: {:08X} queued for lazy tracking", formID);
        }

        RE::GFxValue val{ static_cast<double>(ts) };
        a_object->SetMember("acquiredTime", val);
    }

    void RegisterMenuEventHandler()
    {
        auto* ui = RE::UI::GetSingleton();
        if (ui) {
            ui->AddEventSink<RE::MenuOpenCloseEvent>(MenuEventHandler::GetSingleton());
            SKSE::log::info("ScaleformExtension: registered menu event handler for lazy tracking");
        }
    }
}
