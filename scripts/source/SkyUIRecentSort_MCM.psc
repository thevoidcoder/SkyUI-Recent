Scriptname SkyUIRecentSort_MCM extends SKI_ConfigBase

; Native function from SKSE plugin
int function FillExistingItemTimestamps() global native

; MCM version
int function GetVersion()
    return 1
endFunction

event OnConfigInit()
    Pages = new string[1]
    Pages[0] = "$SRS_PageGeneral"
endEvent

event OnPageReset(string page)
    SetCursorFillMode(TOP_TO_BOTTOM)
    
    AddHeaderOption("$SRS_HeaderTimestamps")
    AddTextOption("$SRS_InfoText", "", OPTION_FLAG_DISABLED)
    AddEmptyOption()
    AddToggleOptionST("STATE_FillTimestamps", "$SRS_FillTimestamps", false)
    AddEmptyOption()
    AddTextOption("$SRS_WarningText", "", OPTION_FLAG_DISABLED)
endEvent

state STATE_FillTimestamps
    event OnSelectST()
        bool confirmed = ShowMessage("$SRS_ConfirmFill", true, "$SRS_Yes", "$SRS_No")
        if confirmed
            int itemsProcessed = FillExistingItemTimestamps()
            if itemsProcessed > 0
                ShowMessage("$SRS_Success{" + itemsProcessed + "}", false, "$SRS_OK")
            elseif itemsProcessed == 0
                ShowMessage("$SRS_AlreadyFilled", false, "$SRS_OK")
            else
                ShowMessage("$SRS_Error", false, "$SRS_OK")
            endif
        endif
    endEvent
    
    event OnDefaultST()
        ; Nothing to default
    endEvent
    
    event OnHighlightST()
        SetInfoText("$SRS_FillTimestamps_Info")
    endEvent
endState
