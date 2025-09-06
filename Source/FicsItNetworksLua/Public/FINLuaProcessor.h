#pragma once

#include "CoreMinimal.h"
#include "AsyncWork.h"
#include "FINLuaReferenceCollector.h"
#include "FINLuaRuntimePersistence.h"
#include "FINLuaThreadedRuntime.h"
#include "Future.h"
#include "LuaComponentAPI.h"
#include "LuaEventAPI.h"
#include "FicsItFileSystem/Listener.h"
#include "FicsItKernel/Processor/Processor.h"
#include "FINLua/API/LuaFileSystemAPI.h"
#include "FINLuaProcessor.generated.h"

class AFINStateEEPROMLua;

class FICSITNETWORKSLUA_API LuaFileSystemListener : public CodersFileSystem::Listener {
private:
	class UFINLuaProcessor* Parent = nullptr;
public:
	LuaFileSystemListener(class UFINLuaProcessor* InParent) : Parent(InParent) {}
	
	virtual void onUnmounted(CodersFileSystem::Path path, TSharedRef<CodersFileSystem::Device> device) override;
	virtual void onNodeRemoved(CodersFileSystem::Path path, CodersFileSystem::NodeType type) override;
};

UCLASS()
class FICSITNETWORKSLUA_API UFINLuaProcessor : public UFINKernelProcessor {
	GENERATED_BODY()
private:
	UPROPERTY()
	FFINLuaThreadedRuntime Runtime;
	UPROPERTY()
	FFINLuaReferenceCollector ReferenceCollector;
	FFINLuaComponentNetworkAccessDelegates ComponentNetwork;
	FFINLuaEventSystem EventSystem;

	UPROPERTY(SaveGame)
	bool bIsFromSave = false;

public:
	UPROPERTY(SaveGame)
	FFINLuaRuntimePersistenceState RuntimeState;

	UFINLuaProcessor();

	// Begin UObject
	virtual void BeginDestroy() override;
	// End UObject
	
	// Begin IFGSaveInterface
	virtual void GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) override;
	virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	// End IFGSaveInterface

	// Begin Processor
	virtual void SetKernel(UFINKernelSystem* InKernel) override;
	virtual void Tick(float InDelta) override;
	virtual void Stop(bool bInIsCrash) override;
	virtual void Reset() override;
	// End Processor

	/**
	 * Allows to access the eeprom code used by the processor.
	 * None if no eeprom is currently set.
	 *
	 * @return the eeprom code used by the processor.
	 */
	TOptional<FString> GetEEPROM() const;

	/**
	 * Allows to change the EEPROMs Code.
	 *
	 * @return true if a compatible eeprom is available to store the given code.
	 */
	bool SetEEPROM(const FString& Code);

	/**
	 * Tries to pop a signal from the signal queue in the network controller
	 * and pushes the resulting values to the given lua stack.
	 *
	 * @param[in]	L	the stack were the values should get pushed to.
	 * @return	the count of values we have pushed.
	 */
	int DoSignal(lua_State* L);
};
