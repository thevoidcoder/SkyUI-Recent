#include "PCH.h"
#include "ConfigInjector.h"

namespace skyui_recent::config_injector
{
    // Item-inventory views that should include the acquiredColumn.
    // Magic, crafting, and alchemy views are intentionally excluded.
    static constexpr const char* kItemViews[] = {
        "defaultItemView",
        "weaponView",
        "armorView",
        "magicItemView",
        "keysView"
    };

    // Menus whose SWFs load SkyUI's ListLayout config.
    static constexpr const char* kMenuNames[] = {
        "InventoryMenu",
        "ContainerMenu",
        "BarterMenu"
    };

    // -----------------------------------------------------------------------
    //  Helpers
    // -----------------------------------------------------------------------

    static void MakeStringArray(RE::GFxMovieView* view, RE::GFxValue* out,
                                const char* s1, const char* s2)
    {
        view->CreateArray(out);
        RE::GFxValue v1, v2;
        view->CreateString(&v1, s1);
        view->CreateString(&v2, s2);
        out->PushBack(v1);
        out->PushBack(v2);
    }

    static void MakeNumberArray(RE::GFxMovieView* view, RE::GFxValue* out,
                                double n1, double n2)
    {
        view->CreateArray(out);
        out->PushBack(RE::GFxValue{n1});
        out->PushBack(RE::GFxValue{n2});
    }

    // Build the state sub-object shared by state1/state2.
    // sortDescending controls whether the primary sort is DESCENDING|NUMERIC (true)
    // or just NUMERIC (false).
    static void BuildState(RE::GFxMovieView* view, RE::GFxValue* state,
                           bool sortDescending)
    {
        view->CreateObject(state);

        // label
        RE::GFxValue label;
        view->CreateObject(&label);
        RE::GFxValue labelText;
        view->CreateString(&labelText, "$ACQ");
        label.SetMember("text", labelText);
        label.SetMember("arrowDown", RE::GFxValue{true});
        state->SetMember("label", label);

        // entry
        RE::GFxValue entry;
        view->CreateObject(&entry);
        RE::GFxValue entryText;
        view->CreateString(&entryText, "@acquiredTime");
        entry.SetMember("text", entryText);
        state->SetMember("entry", entry);

        // sortAttributes = ["acquiredTime", "text"]
        RE::GFxValue sortAttrs;
        MakeStringArray(view, &sortAttrs, "acquiredTime", "text");
        state->SetMember("sortAttributes", sortAttrs);

        // sortOptions
        //   state1: [DESCENDING|NUMERIC (18), CASEINSENSITIVE (1)]
        //   state2: [NUMERIC (16),            CASEINSENSITIVE (1)]
        constexpr double NUMERIC        = 16.0;
        constexpr double DESCENDING     = 2.0;
        constexpr double CASEINSENSITIVE = 1.0;
        double primaryOpt = sortDescending ? (DESCENDING + NUMERIC) : NUMERIC;

        RE::GFxValue sortOpts;
        MakeNumberArray(view, &sortOpts, primaryOpt, CASEINSENSITIVE);
        state->SetMember("sortOptions", sortOpts);
    }

    // -----------------------------------------------------------------------
    //  Core injection logic
    // -----------------------------------------------------------------------

    static bool InjectAcquiredColumn(RE::GFxMovieView* view)
    {
        // 1. Locate ConfigManager class in the AS2 global scope.
        RE::GFxValue configMgr;
        if (!view->GetVariable(&configMgr, "_global.skyui.util.ConfigManager")) {
            SKSE::log::trace("ConfigInjector: ConfigManager class not found");
            return false;
        }

        // 2. Check load phase.
        //    0 = LOAD_NONE  – config hasn't started loading yet
        //    1 = LOAD_FILE  – config.txt parsed, waiting for Papyrus overrides
        //    2 = LOAD_PAPYRUS – finalised, configLoad already dispatched
        RE::GFxValue loadPhaseVal;
        configMgr.GetMember("_loadPhase", &loadPhaseVal);
        double loadPhase = loadPhaseVal.IsNumber() ? loadPhaseVal.GetNumber() : -1.0;
        if (loadPhase < 1.0) {
            SKSE::log::trace("ConfigInjector: config not ready (phase={})", loadPhase);
            return false;  // caller should retry
        }

        // 3. Navigate to _config.ListLayout.columns
        RE::GFxValue config, layout, columns;
        configMgr.GetMember("_config", &config);
        if (config.IsUndefined()) { SKSE::log::warn("ConfigInjector: _config undefined"); return false; }

        config.GetMember("ListLayout", &layout);
        if (layout.IsUndefined()) { SKSE::log::warn("ConfigInjector: ListLayout undefined"); return false; }

        layout.GetMember("columns", &columns);
        if (columns.IsUndefined()) { SKSE::log::warn("ConfigInjector: columns undefined"); return false; }

        // 4. Skip if acquiredColumn is already defined (another instance of our
        //    mod, or the user merged it into config.txt manually).
        RE::GFxValue existingAcq;
        columns.GetMember("acquiredColumn", &existingAcq);
        if (!existingAcq.IsUndefined()) {
            SKSE::log::info("ConfigInjector: acquiredColumn already present – skipping");
            return true;  // nothing to do, not an error
        }

        SKSE::log::info("ConfigInjector: injecting acquiredColumn (loadPhase={})", loadPhase);

        // 5. Build the column definition object ---------------------------------
        RE::GFxValue acqCol;
        view->CreateObject(&acqCol);

        // type = COL_TYPE_TEXT = 2
        acqCol.SetMember("type", RE::GFxValue{2.0});

        // name = "$LOOTED"
        RE::GFxValue nameStr;
        view->CreateString(&nameStr, "$LOOTED");
        acqCol.SetMember("name", nameStr);

        // states = 2
        acqCol.SetMember("states", RE::GFxValue{2.0});

        // width = 0.09
        acqCol.SetMember("width", RE::GFxValue{0.09});

        // border = [left=0, right=0, top=1.1, bottom=0]  (same as a_textBorder)
        RE::GFxValue border;
        view->CreateArray(&border);
        border.PushBack(RE::GFxValue{0.0});
        border.PushBack(RE::GFxValue{0.0});
        border.PushBack(RE::GFxValue{1.1});
        border.PushBack(RE::GFxValue{0.0});
        acqCol.SetMember("border", border);

        // state1 – descending sort first
        RE::GFxValue state1;
        BuildState(view, &state1, /*sortDescending=*/true);
        acqCol.SetMember("state1", state1);

        // state2 – ascending sort
        RE::GFxValue state2;
        BuildState(view, &state2, /*sortDescending=*/false);
        acqCol.SetMember("state2", state2);

        // Store column definition on the config object.
        columns.SetMember("acquiredColumn", acqCol);

        // 6. Append "acquiredColumn" to each item-view's columns array ----------
        RE::GFxValue views;
        layout.GetMember("views", &views);
        if (views.IsUndefined()) {
            SKSE::log::warn("ConfigInjector: views undefined");
            return true;  // column defined, views just can't be updated
        }

        RE::GFxValue colNameStr;
        view->CreateString(&colNameStr, "acquiredColumn");

        for (const char* viewName : kItemViews) {
            RE::GFxValue viewObj;
            views.GetMember(viewName, &viewObj);
            if (viewObj.IsUndefined())
                continue;

            RE::GFxValue viewCols;
            viewObj.GetMember("columns", &viewCols);
            if (viewCols.IsUndefined() || !viewCols.IsArray())
                continue;

            // Guard against duplicate push.
            bool alreadyPresent = false;
            auto size = viewCols.GetArraySize();
            for (std::uint32_t i = 0; i < size; ++i) {
                RE::GFxValue elem;
                viewCols.GetElement(i, &elem);
                if (elem.IsString() && std::string_view{elem.GetString()} == "acquiredColumn") {
                    alreadyPresent = true;
                    break;
                }
            }

            if (!alreadyPresent) {
                viewCols.PushBack(colNameStr);
                SKSE::log::trace("ConfigInjector: appended acquiredColumn to {}", viewName);
            }
        }

        // 7. If configLoad already fired, trigger a layout rebuild ---------------
        if (loadPhase >= 2.0) {
            RE::GFxValue eventDummy;
            configMgr.GetMember("_eventDummy", &eventDummy);
            if (!eventDummy.IsUndefined()) {
                RE::GFxValue eventObj;
                view->CreateObject(&eventObj);
                RE::GFxValue typeStr;
                view->CreateString(&typeStr, "configUpdate");
                eventObj.SetMember("type", typeStr);
                eventObj.SetMember("config", config);
                eventDummy.Invoke("dispatchEvent", nullptr, &eventObj, 1);
                SKSE::log::info("ConfigInjector: dispatched configUpdate");
            }
        }
        // If loadPhase == 1, the pending configLoad will pick up our changes.

        SKSE::log::info("ConfigInjector: injection complete");
        return true;
    }

    // -----------------------------------------------------------------------
    //  Retry scheduler – polls once per game frame until config is ready.
    //  Now monitors for config reloads and re-injects to handle mods that
    //  load config.txt after us.
    // -----------------------------------------------------------------------

    static std::unordered_map<std::string, double> _lastKnownPhase;
    static std::unordered_map<std::string, bool> _hasInjected;

    static void ScheduleInject(std::string menuName, int retries = 0);

    static void ScheduleInject(std::string menuName, int retries)
    {
        SKSE::GetTaskInterface()->AddTask(
            [name = std::move(menuName), retries]() {
                auto* ui = RE::UI::GetSingleton();
                if (!ui) return;

                auto view = ui->GetMovieView(name);
                if (!view) {
                    // Menu closed – clean up tracking
                    _lastKnownPhase.erase(name);
                    _hasInjected.erase(name);
                    return;
                }

                // Check current phase
                RE::GFxValue configMgr;
                if (view->GetVariable(&configMgr, "_global.skyui.util.ConfigManager")) {
                    RE::GFxValue phaseVal;
                    configMgr.GetMember("_loadPhase", &phaseVal);
                    double currentPhase = phaseVal.IsNumber() ? phaseVal.GetNumber() : -1.0;

                    // Detect config reload (phase went back to 0 or 1 after being at 2)
                    auto lastPhase = _lastKnownPhase[name];
                    if (_hasInjected[name] && currentPhase < lastPhase && currentPhase >= 0.0) {
                        SKSE::log::info("ConfigInjector: config reloaded on {} (phase {} -> {}), re-injecting",
                                        name, lastPhase, currentPhase);
                        _hasInjected[name] = false;
                    }
                    _lastKnownPhase[name] = currentPhase;
                }

                bool done = InjectAcquiredColumn(view.get());
                if (done) {
                    _hasInjected[name] = true;
                }

                if (!done && retries < 240) {
                    // Retry next frame (~4 s at 60 fps covers the 3 s timeout).
                    ScheduleInject(name, retries + 1);
                } else if (!done) {
                    SKSE::log::warn("ConfigInjector: gave up on {} after {} retries",
                                    name, retries);
                } else if (retries < 240) {
                    // Keep monitoring for config reloads even after successful injection
                    ScheduleInject(name, retries + 1);
                }
            });
    }

    // -----------------------------------------------------------------------
    //  Menu open listener
    // -----------------------------------------------------------------------

    class MenuWatcher final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        static MenuWatcher* GetSingleton()
        {
            static MenuWatcher instance;
            return &instance;
        }

        RE::BSEventNotifyControl ProcessEvent(
            const RE::MenuOpenCloseEvent* a_event,
            RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
        {
            if (!a_event || !a_event->opening)
                return RE::BSEventNotifyControl::kContinue;

            for (const char* name : kMenuNames) {
                if (a_event->menuName == name) {
                    SKSE::log::trace("ConfigInjector: {} opened", name);
                    ScheduleInject(std::string{name});
                    break;
                }
            }

            return RE::BSEventNotifyControl::kContinue;
        }

    private:
        MenuWatcher() = default;
        MenuWatcher(const MenuWatcher&) = delete;
        MenuWatcher& operator=(const MenuWatcher&) = delete;
    };

    // -----------------------------------------------------------------------
    //  Public API
    // -----------------------------------------------------------------------

    void Register()
    {
        if (auto* ui = RE::UI::GetSingleton()) {
            ui->AddEventSink(MenuWatcher::GetSingleton());
            SKSE::log::info("ConfigInjector: registered menu event listener");
        }
    }
}
