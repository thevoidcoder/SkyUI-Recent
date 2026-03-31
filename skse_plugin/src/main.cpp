#include "PCH.h"

#include "Hooks.h"
#include "PapyrusBindings.h"
#include "ScaleformExtension.h"
#include "Serialization.h"

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

    skyui_recent::hooks::Install();

    auto* ser = SKSE::GetSerializationInterface();
    ser->SetUniqueID(skyui_recent::serialization::kPluginID);
    ser->SetSaveCallback(skyui_recent::serialization::OnSave);
    ser->SetLoadCallback(skyui_recent::serialization::OnLoad);
    ser->SetRevertCallback(skyui_recent::serialization::OnRevert);

    SKSE::GetPapyrusInterface()->Register(skyui_recent::papyrus::Register);

    SKSE::GetScaleformInterface()->Register(skyui_recent::scaleform::OnInventoryItem);

    return true;
}
