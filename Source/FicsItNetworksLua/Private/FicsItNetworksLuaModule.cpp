#include "FicsItNetworksLuaModule.h"

#include "CoreRedirects.h"
#include "FGDynamicStruct.h"
#include "FGGameMode.h"
#include "FINLuaRCO.h"
#include "Patching/NativeHookManager.h"
#include "FINItemStateEEPROMLua.h"

DEFINE_LOG_CATEGORY(LogFicsItNetworksLua);
DEFINE_LOG_CATEGORY(LogFicsItNetworksLuaReflection);
DEFINE_LOG_CATEGORY(LogFicsItNetworksLuaPersistence);

IMPLEMENT_GAME_MODULE(FFicsItNetworksLuaModule, FicsItNetworksLua);

void FFicsItNetworksLuaModule::StartupModule() {
	TArray<FCoreRedirect> redirects;
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworks.FINStateEEPROMLua"), TEXT("/Script/FicsItNetworksLua.FINStateEEPROMLua_Legacy")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_Class, TEXT("/Script/FicsItNetworksLua.FINStateEEPROMLua"), TEXT("/Script/FicsItNetworksLua.FINStateEEPROMLua_Legacy")});
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

UFINLuaGameInstanceModule::UFINLuaGameInstanceModule() {
	RemoteCallObjects.Add(UFINLuaRCO::StaticClass());
}

bool UFINLuaUtils::TryGetLuaEEPROM(const FFGDynamicStruct& Struct, FFINItemStateEEPROMLua& LuaEEPROM) {
	auto ptr = Struct.GetValuePtr<FFINItemStateEEPROMLua>();

	if (ptr == nullptr) {
		return false;
	}

	LuaEEPROM = *ptr;
	return true;
}

bool UFINLuaUtils::BreakFINDynamicStruct(const FFGDynamicStruct& inDynamicStruct, int32& out_structureValue) { return false; }

DEFINE_FUNCTION(UFINLuaUtils::execBreakFINDynamicStruct) {
	PARAM_PASSED_BY_REF(inDynamicStruct, FStructProperty, FFGDynamicStruct);
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	FStructProperty* Property = CastField<FStructProperty>(Stack.MostRecentProperty);
	P_FINISH;
	P_NATIVE_BEGIN;
	if (Property != nullptr && inDynamicStruct.IsValid() && inDynamicStruct.GetStruct()->IsChildOf(Property->Struct.Get())) {
		Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>(Stack.MostRecentPropertyContainer), inDynamicStruct.GetStructValueRaw());
		*reinterpret_cast<bool*>(RESULT_PARAM) = true;
	} else {
		*reinterpret_cast<bool*>(RESULT_PARAM) = false;
	}
	P_NATIVE_END;
}
