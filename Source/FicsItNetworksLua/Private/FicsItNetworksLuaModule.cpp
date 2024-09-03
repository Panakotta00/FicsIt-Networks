#include "FicsItNetworksLuaModule.h"

#include "FGGameMode.h"
#include "FINLuaRCO.h"
#include "Patching/NativeHookManager.h"

DEFINE_LOG_CATEGORY(LogFicsItNetworksLua);
DEFINE_LOG_CATEGORY(LogFicsItNetworksLuaReflection);
DEFINE_LOG_CATEGORY(LogFicsItNetworksLuaPersistence);

IMPLEMENT_GAME_MODULE(FFicsItNetworksLuaModule, FicsItNetworksLua);

void FFicsItNetworksLuaModule::StartupModule() {
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
