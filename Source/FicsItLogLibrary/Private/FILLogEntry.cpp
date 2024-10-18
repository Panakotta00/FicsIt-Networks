#include "FILLogEntry.h"

#define LOCTEXT_NAMESPACE "FIL_LogEntry"

FText FFILEntry::GetVerbosityAsText() const {
	switch (Verbosity) {
		case FIL_Verbosity_Debug: return LOCTEXT("VerbosityDebug", "Debug");
		case FIL_Verbosity_Info: return LOCTEXT("VerbosityInfo", "Info");
		case FIL_Verbosity_Warning: return LOCTEXT("VerbosityWarning", "Warning");
		case FIL_Verbosity_Error: return LOCTEXT("VerbosityError", "Error");
		case FIL_Verbosity_Fatal: return LOCTEXT("VerbosityFatal", "Fatal");
		default: return FText();
	}
}

FString FFILEntry::ToClipboardText() const {
	FString TimestampText = Timestamp.ToString();
	FString VerbosityText = GetVerbosityAsText().ToString();
	return FString::Printf(TEXT("%s [%s] %s"), *TimestampText, *VerbosityText, *Content);
}
