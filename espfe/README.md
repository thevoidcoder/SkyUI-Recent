# ESPFE bridge plugin notes

Create an ESL-flagged ESP (`SkyUIRecentSort.esp`) containing only lightweight bridge records:

- Optional quest/script host record if your chosen deployment requires a quest-backed script start.
- Any globals or formlists needed by your I4 profile.
- No gameplay edits; no SkyUI record overrides.

This keeps the plugin load-order-safe and compact while delegating runtime logic to SKSE + I4.
