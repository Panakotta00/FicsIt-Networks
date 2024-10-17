#pragma once

#include "CoreMinimal.h"
#include "Module/GameInstanceModule.h"
#include "Modules/ModuleManager.h"
#include "FicsItNetworksLuaModule.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItNetworksLua, Verbose, All);
DECLARE_LOG_CATEGORY_EXTERN(LogFicsItNetworksLuaReflection, Fatal, All);
DECLARE_LOG_CATEGORY_EXTERN(LogFicsItNetworksLuaPersistence, Warning, All);

class FFicsItNetworksLuaModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	virtual bool IsGameModule() const override { return true; }
};

UCLASS()
class FICSITNETWORKSLUA_API UFINLuaGameInstanceModule : public UGameInstanceModule {
	GENERATED_BODY()

	UFINLuaGameInstanceModule();
};

UCLASS()
class UFINLuaUtils : public UObject {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	static bool TryGetLuaEEPROM(const FFGDynamicStruct& Struct, FFINItemStateEEPROMLua& LuaEEPROM);

	UFUNCTION( BlueprintPure, CustomThunk, Category = "Utilities|Dynamic Struct", meta = (NativeBreakFunc, CustomStructureParam = "out_structureValue" ) )
	static bool BreakFINDynamicStruct( const FFGDynamicStruct& inDynamicStruct, int32& out_structureValue );
	DECLARE_FUNCTION(execBreakFINDynamicStruct);
};