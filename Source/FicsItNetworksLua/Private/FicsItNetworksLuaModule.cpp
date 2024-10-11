#include "FicsItNetworksLuaModule.h"

#include "CoreRedirects.h"
#include "FGGameMode.h"
#include "FINLuaRCO.h"
#include "Patching/NativeHookManager.h"

DEFINE_LOG_CATEGORY(LogFicsItNetworksLua);
DEFINE_LOG_CATEGORY(LogFicsItNetworksLuaReflection);
DEFINE_LOG_CATEGORY(LogFicsItNetworksLuaPersistence);

IMPLEMENT_GAME_MODULE(FFicsItNetworksLuaModule, FicsItNetworksLua);

void FFicsItNetworksLuaModule::StartupModule() {
	TArray<FCoreRedirect> redirects;
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINStateEEPROMLua"), TEXT("/Script/FicsItNetworksLua.FINStateEEPROMLua")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINComputerProcessorLua"), TEXT("/Script/FicsItNetworksLua.FINComputerProcessorLua")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINLuaProcessor"), TEXT("/Script/FicsItNetworksLua.FINLuaProcessor")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINLuaProcessorStateStorage"), TEXT("/Script/FicsItNetworksLua.FINLuaProcessorStateStorage")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.EFINMetaRuntimeState"), TEXT("/Script/FicsItNetworksLua.EFINReflectionMetaRuntimeState")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FFINBlueprintPropertyMeta"), TEXT("/Script/FicsItNetworksLua.FFINReflectionPropertyMeta")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FFINBlueprintFunctionMetaParameter"), TEXT("/Script/FicsItNetworksLua.FFINReflectionFunctionParameterMeta")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FFINBlueprintFunctionMeta"), TEXT("/Script/FicsItNetworksLua.FFINReflectionFunctionMeta")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FFINBlueprintSignalMeta"), TEXT("/Script/FicsItNetworksLua.FFINReflectionSignalMeta")});

	FCoreRedirects::AddRedirectList(redirects, "FicsItNetworksLua");

	FCoreDelegates::OnPostEngineInit.AddStatic([]() {
#if !WITH_EDITOR
		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGGameMode::PostLogin, (void*)GetDefault<AFGGameMode>(), [](AFGGameMode* gm, APlayerController* pc) {
			if (gm->HasAuthority() && !gm->IsMainMenuGameMode()) {
				gm->RegisterRemoteCallObjectClass(UFINLuaRCO::StaticClass());
			}
		});
#endif
	});
}

void FFicsItNetworksLuaModule::ShutdownModule() {
}
