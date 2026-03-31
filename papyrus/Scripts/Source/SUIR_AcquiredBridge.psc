Scriptname SUIR_AcquiredBridge Hidden

; Native API implemented in the SKSE plugin.
Int Function GetItemAcquiredTime(Int formID, Int uniqueID) Global Native

; Utility function for UI contexts that only provide form IDs.
Int Function GetItemAcquiredTimeSimple(Form akItem) Global
    If akItem == None
        Return 0
    EndIf
    Return GetItemAcquiredTime(akItem.GetFormID(), 0)
EndFunction
