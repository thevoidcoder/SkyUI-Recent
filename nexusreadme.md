SkyUI Sort by Acquisition
Sort your inventory by the order you picked things up

Overview: Ever opened your inventory and had no way to tell which items you just picked up? This mod adds a new Acquired column to the SkyUI player inventory that lets you sort all items by the order in which you obtained them - so the things you grabbed most recently always float to the top.

Features:
Acquisition-order sorting - a new "Acquired" column appears in all SkyUI inventory lists and is fully sortable ascending or descending.
Comprehensive item tracking - items are tracked across every acquisition path:
 
 
Manual world pickups
 
Items transferred from containers or NPCs
 
Harvested flora / ingredient plants
 
Items added via scripts or the AddItem console command
 
Save-persistent data - acquisition order is written into your save file via SKSE serialization and fully restored on load. Data seamlessly handles mod load-order changes via FormID remapping.
Papyrus API - a native function SUIR_AcquiredBridge.GetItemAcquiredTime(formID, uniqueID) is registered for use by other mods or personal scripts.
Address Library support - uses Address Library for SKSE Plugins, covering both Special Edition and Anniversary Edition game versions.
Lightweight - an ESL-flagged plugin (if required) uses zero of your 255 ESP slots.

Requirements
Skyrim Script Extender (SKSE)
SkyUI 6
Address Library for SKSE Plugins

InstallationMod Manager (recommended)
Install with Mod Organizer 2 or Vortex like any other mod. Enable and you're done.

Manual
Extract the archive and copy its contents into your Skyrim Special Edition Data folder. The structure should be:

Data/
  SKSE/Plugins/
    SkyUIRecentSort.dll

How It Works: When you pick up or receive an item, the SKSE plugin intercepts the game's internal inventory functions and stamps that item with an incremental acquisition counter. Every item in your inventory carries a number reflecting when you got it relative to everything else.

When SkyUI opens the inventory menu it calls skse.ExtendData(true) to request extended data for each item. The plugin intercepts this and injects an acquiredTime property directly into the item's data object. The plugin also dynamically injects the Acquired column definition into SkyUI's configuration at runtime via the Scaleform API, so no config.txt replacement is needed.

Important: Items that existed in your inventory before installing this mod will show a value of 0 and will sort together at the bottom of the list. Only items acquired after the mod is installed will have a meaningful sort order.

Compatibility
Fully compatible with other mods that modify or replace Interface/skyui/config.txt. The Acquired column is injected at runtime via the DLL, so it will appear alongside any other custom columns.
Compatible with all SkyUI updates.
Compatible with mod managers, ENBs, and other SKSE plugins.

Uninstallation: Disable or remove the mod files. The SKSE data block written to your save file is harmless to leave in place but can be cleaned with a save-file cleaner if desired. No config.txt changes need to be reverted.

Known Limitations
Items acquired before the mod was installed have no tracked order and sort as "0".
Stacked items of the same base form share a single acquisition timestamp (the most recent one). Individual stack members are not tracked separately.
The column value is a sort key, not a human-readable date/time string.

Source Code: Source is available on GitHub. Contributions and bug reports are welcome.

Credits
Bethesda - Skyrim
SKSE Team - Skyrim Script Extender
SkyUI Team - SkyUI
meh321 - Address Library for SKSE Plugins
CommonLibSSE-NG contributors - reverse-engineering framework