# SkyUI Sort by Acquisition Add-on

A SkyUI 6-compatible support mod that adds an **Acquired** sorting column to the player inventory **without editing SkyUI's original SWF files**.

## What this repository contains

- `skse_plugin/`: SKSE plugin that tracks inventory acquisition times using game events and persists data across saves.
- `papyrus/`: optional Papyrus bridge script and API facade to support UI lookup and debugging.
- `interface/i4/`: Inventory Interface Information Injector (I4) injection definition for the new inventory column.
- `espfe/`: plugin authoring notes for the lightweight ESPFE bridge record set.

## High-level design

1. **Data Tracking Layer (SKSE plugin)**
   - Hooks inventory change notifications for the player.
   - On item gain, stores a UNIX timestamp keyed by owner+item+instance identity.
   - Persists all tracked data via SKSE serialization.

2. **Bridge Layer (Papyrus + Native function registration)**
   - Exposes `GetItemAcquiredTime(...)` to UI-facing code.
   - Provides an optional Papyrus script to fetch and normalize values for HUD/menu use.

3. **UI Injection Layer (I4)**
   - Injects a new sortable column named `Acquired` into SkyUI inventory item lists.
   - Uses the native/Papyrus bridge function for each row's sort value.

4. **Plugin Compatibility Layer (ESPFE)**
   - A compact ESL-flagged plugin for glue records (quests/global/script property links if needed by the final packaged mod).

## Notes

- This project intentionally avoids touching SkyUI SWFs to remain compatible with SkyUI updates.
- Timestamps are sortable numeric values; display formatting can be done in ActionScript, I4 formatter rules, or Papyrus.
