#include "FILLogEntry.h"

#define LOCTEXT_NAMESPACE "FIL_LogEntry"

FText FFILLogEntry::GetVerbosityAsText() const {
	switch (Verbosity) {
		case FIN_Log_Verbosity_Debug: return LOCTEXT("VerbosityDebug", "Debug");
		case FIN_Log_Verbosity_Info: return LOCTEXT("VerbosityInfo", "Info");
		case FIN_Log_Verbosity_Warning: return LOCTEXT("VerbosityWarning", "Warning");
		case FIN_Log_Verbosity_Error: return LOCTEXT("VerbosityError", "Error");
		case FIN_Log_Verbosity_Fatal: return LOCTEXT("VerbosityFatal", "Fatal");
		default: return FText();
	}
}

FString FFILLogEntry::ToClipboardText() const {
	FString TimestampText = Timestamp.ToString();
	FString VerbosityText = GetVerbosityAsText().ToString();
	return FString::Printf(TEXT("%s [%s] %s"), *TimestampText, *VerbosityText, *Content);
}
