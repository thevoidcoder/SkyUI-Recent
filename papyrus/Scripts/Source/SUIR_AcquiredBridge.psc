Scriptname SUIR_AcquiredBridge Hidden

; Native API implemented in the SKSE plugin.
Int Function GetItemAcquiredTime(Int formID, Int uniqueID) Global Native

; Prefill all untracked inventory items with timestamps
; Call from console: SUIR_AcquiredBridge.PrefillInventory()
Int Function PrefillInventory() Global Native

; Utility wrapper for callers that only have a Form reference.
; Passes uniqueID=0, which is correct for all non-unique/non-enchanted items.
; For items with a real ExtraUniqueID the C++ side falls back to
; GetLatestAcquiredTime(formID), so this still returns a useful timestamp.
Int Function GetItemAcquiredTimeSimple(Form akItem) Global
    If akItem == None
        Return 0
    EndIf
    Return GetItemAcquiredTime(akItem.GetFormID(), 0)
EndFunction
