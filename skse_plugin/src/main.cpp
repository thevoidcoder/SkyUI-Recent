#include "PCH.h"

#include "AcquiredTracker.h"
#include "PapyrusBindings.h"
#include "ScaleformExtension.h"
#include "Serialization.h"

#include <chrono>

namespace skyui_recent::events
{
    class ContainerChangedSink final :
        public RE::BSTEventSink<RE::TESContainerChangedEvent>
    {
    public:
        static ContainerChangedSink* GetSingleton()
        {
            static ContainerChangedSink singleton;
            return &singleton;
        }

        RE::BSEventNotifyControl ProcessEvent(
            const RE::TESContainerChangedEvent* a_event,
            RE::BSTEventSource<RE::TESContainerChangedEvent>*) override
        {
            if (!a_event || a_event->itemCount <= 0) {
                return RE::BSEventNotifyControl::kContinue;
            }

            const auto* player = RE::PlayerCharacter::GetSingleton();
            if (!player || a_event->newContainer != player->GetFormID()) {
                return RE::BSEventNotifyControl::kContinue;
            }

            const auto now  = std::chrono::system_clock::now();
            const auto secs = std::chrono::time_point_cast<std::chrono::seconds>(now)
                                  .time_since_epoch()
                                  .count();

            AcquiredTracker::GetSingleton().MarkItemAdded(
                a_event->baseObj,
                a_event->uniqueID,
                static_cast<std::int64_t>(secs));

            SKSE::log::trace("Tracked formID {:08X} uniqueID {} at {}",
                a_event->baseObj, a_event->uniqueID, secs);

            return RE::BSEventNotifyControl::kContinue;
        }
    };

    void RegisterSinks()
    {
        auto* holder = RE::ScriptEventSourceHolder::GetSingleton();
        if (holder) {
            holder->AddEventSink<RE::TESContainerChangedEvent>(
                ContainerChangedSink::GetSingleton());
            SKSE::log::info("SkyUIRecentSort: TESContainerChangedEvent sink registered.");
        } else {
            SKSE::log::error("SkyUIRecentSort: ScriptEventSourceHolder is null — event sink NOT registered!");
        }
    }
}  // namespace skyui_recent::events

namespace
{
    void OnMessage(SKSE::MessagingInterface::Message* a_msg)
    {
        if (a_msg && a_msg->type == SKSE::MessagingInterface::kDataLoaded) {
            skyui_recent::events::RegisterSinks();
        }
    }
}  // namespace

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);

    if (const auto logDir = SKSE::log::log_directory()) {
        auto logPath = *logDir / "SkyUIRecentSort.log";
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath.string(), true);
        auto log  = std::make_shared<spdlog::logger>("SkyUIRecentSort", std::move(sink));
        log->set_level(spdlog::level::trace);
        log->flush_on(spdlog::level::trace);
        spdlog::set_default_logger(std::move(log));
    }

    SKSE::log::info("SkyUIRecentSort v0.1.0 loaded.");

    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);

    auto* ser = SKSE::GetSerializationInterface();
    ser->SetUniqueID(skyui_recent::serialization::kPluginID);
    ser->SetSaveCallback(skyui_recent::serialization::OnSave);
    ser->SetLoadCallback(skyui_recent::serialization::OnLoad);
    ser->SetRevertCallback(skyui_recent::serialization::OnRevert);

    SKSE::GetPapyrusInterface()->Register(skyui_recent::papyrus::Register);

    SKSE::GetScaleformInterface()->Register(skyui_recent::scaleform::OnInventoryItem);

    return true;
}
