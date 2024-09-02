#include "FicsItLogLibrary.h"

#include "FILLogContainer.h"
#include "FILLogScope.h"
#include "UObject/CoreRedirects.h"

#define LOCTEXT_NAMESPACE "FFicsItLogLibraryModule"

void UFILogLibrary::Log(TEnumAsByte<EFILLogVerbosity> Verbosity, FString Message, TEnumAsByte<EFILLogOptions> Options) {
	switch (Options) {
	case FIL_Option_Where: {
		FString Where = FFILLogScope::Where();
		if (!Where.IsEmpty()) Where.AppendChar(L' ');
		FFILLogScope::GetLog()->PushLogEntry(Verbosity, Where.Append(Message));
		break;
	} default:
		FFILLogScope::GetLog()->PushLogEntry(Verbosity, *Message);
	}
}

void FFicsItLogLibraryModule::StartupModule() {
	TArray<FCoreRedirect> redirects;
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINLogEntry"), TEXT("/Script/FicsItLogLibrary.FILEntry")});
	auto redirectFINLogVerbosity = FCoreRedirect{ECoreRedirectFlags::Type_Enum, TEXT("/Script/FicsItNetworks.FINLogVerbosity"), TEXT("/Script/FicsItLogLibrary.FILLogVerbosity")};
	redirectFINLogVerbosity.ValueChanges = {
		{TEXT("FIN_Log_Verbosity_Debug"), "FIL_Verbosity_Debug"},
		{TEXT("FIN_Log_Verbosity_Info"), "FIL_Verbosity_Info"},
		{TEXT("FIN_Log_Verbosity_Warning"), "FIL_Verbosity_Warning"},
		{TEXT("FIN_Log_Verbosity_Error"), "FIL_Verbosity_Error"},
		{TEXT("FIN_Log_Verbosity_Fatal"), "FIL_Verbosity_Fatal"},
	};
	redirects.Add(redirectFINLogVerbosity);
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINLog"), TEXT("/Script/FicsItLogLibrary.FILLogContainer")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FINLogScope"), TEXT("/Script/FicsItLogLibrary.FILLogScope")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINLogLibrary"), TEXT("/Script/FicsItLogLibrary.FILogLibrary")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Enum, TEXT("/Script/FicsItNetworks.FINLogOptions"), TEXT("/Script/FicsItLogLibrary.FILLogOptions")});

	FCoreRedirects::AddRedirectList(redirects, "FicsIt-Log-Library");
}

void FFicsItLogLibraryModule::ShutdownModule() {
    
}

void UFILRCO::LogRehandleAllEntries_Implementation(UFILLogContainer* Log) {
	Log->RehandleAllEntries();
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FFicsItLogLibraryModule, FicsItLogLibrary)