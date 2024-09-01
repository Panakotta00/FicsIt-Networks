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
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FFINLogEntry"), TEXT("/Script/FicsItLogLibrary.FFILLogEntry")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.EFINLogVerbosity"), TEXT("/Script/FicsItLogLibrary.EFILLogVerbosity")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.UFINLog"), TEXT("/Script/FicsItLogLibrary.UFILLogContainer")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Struct, TEXT("/Script/FicsItNetworks.FFINLogScope"), TEXT("/Script/FicsItLogLibrary.FFILLogScope")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.UFINLogLibrary"), TEXT("/Script/FicsItLogLibrary.UFILogLibrary")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Enum, TEXT("/Script/FicsItNetworks.EFINLogOptions"), TEXT("/Script/FicsItLogLibrary.EFILLogOptions")});

	FCoreRedirects::AddRedirectList(redirects, "FicsIt-Log-Library");
}

void FFicsItLogLibraryModule::ShutdownModule() {
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FFicsItLogLibraryModule, FicsItLogLibrary)