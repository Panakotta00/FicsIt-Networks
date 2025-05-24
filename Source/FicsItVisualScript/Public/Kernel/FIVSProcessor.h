#pragma once

#include "FINLuaReferenceCollector.h"
#include "FINLuaRuntimePersistence.h"
#include "FINLuaThreadedRuntime.h"
#include "LuaComponentAPI.h"
#include "LuaEventAPI.h"
#include "FicsItKernel/Processor/Processor.h"
#include "Script/FIVSScriptContext.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSProcessor.generated.h"

class AFINStateEEPROMText;

UCLASS()
class UFIVSProcessor : public UFINKernelProcessor, public IFIVSScriptContext_Interface {
	GENERATED_BODY()

public:
	UFIVSProcessor();

	// Begin AActor
	virtual void BeginDestroy() override;
	// End AActor

	// Begin UFINKernelProcessor
	virtual void Tick(float InDeltaTime) override;
	virtual void Reset() override;
	virtual void Stop(bool) override;
	// End UFINKernelProcessor

	// Begin IFGSaveGameInterfaces
	virtual void GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) override;
	virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	// End IFGSaveGameInterface

	// Begin IFIVSScriptContext_Interface
	virtual void GetRelevantObjects_Implementation(TArray<FFIRTrace>& OutObjects) override;
	virtual void GetRelevantClasses_Implementation(TArray<UFIRClass*>& OutClasses) override;
	virtual void GetRelevantStructs_Implementation(TArray<UFIRStruct*>& OutStructs) override;
	virtual FFIVSOnScriptCompiled& GetOnScriptCompiledEvent() override;
	// End IFVSScriptContext_Interface

	UPROPERTY(SaveGame)
	FFINLuaRuntimePersistenceState RuntimeState;

private:
	uint32 GraphHash = 0;

	FString LuaCode;

	UPROPERTY()
	FFINLuaThreadedRuntime Runtime;
	UPROPERTY()
	FFINLuaReferenceCollector ReferenceCollector;
	FFINLuaComponentNetworkAccessDelegates ComponentNetwork;
	FFINLuaEventSystem EventSystem;

	FFIVSOnScriptCompiled OnScriptCompiled;
};
