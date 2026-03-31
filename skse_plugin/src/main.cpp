#include "PCH.h"

#include "Hooks.h"
#include "PapyrusBindings.h"
#include "ScaleformExtension.h"
#include "Serialization.h"

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(
    const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name       = "SkyUIRecentSort";
    a_info->version    = 1;
    return !a_skse->IsEditor();
}

extern "C" __declspec(dllexport) auto SKSEPlugin_Version = []() {
    SKSE::PluginVersionData v;
    v.PluginVersion(REL::Version{ 1, 0, 0, 0 });
    v.PluginName("SkyUIRecentSort");
    v.AuthorName("");
    v.UsesAddressLibrary(true);
    // SKSE 2.2.6+ (AE >= 1.6.629) needs kVersionIndependentEx_NoStructUse
    // (bit 0 of versionIndependenceEx, raw SKSE offset 0x304).
    // Our CL PluginVersionData has supportEmail[256] vs SKSE's supportEmail[252],
    // so versionIndependenceEx physically overlaps supportEmail[252..255].
    v.supportEmail[252] = '\x01';
    return v;
}();

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

    SKSE::AllocTrampoline(1 << 8);
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
