Scriptname SKI_ConfigBase extends Quest

; Minimal stub for compilation - implements SkyUI MCM base functionality

string[] property Pages auto
int property TOP_TO_BOTTOM = 1 autoReadOnly
int property OPTION_FLAG_DISABLED = 1 autoReadOnly

event OnConfigInit()
endEvent

event OnPageReset(string page)
endEvent

function SetCursorFillMode(int a_fillMode)
endFunction

int function AddHeaderOption(string text, int flags = 0)
    return 0
endFunction

int function AddTextOption(string text, string value, int flags = 0)
    return 0
endFunction

int function AddEmptyOption()
    return 0
endFunction

int function AddToggleOptionST(string stateName, string text, bool checked, int flags = 0)
    return 0
endFunction

bool function ShowMessage(string message, bool withCancel = true, string acceptLabel = "$Accept", string cancelLabel = "$Cancel")
    return true
endFunction

function SetInfoText(string text)
endFunction
