#pragma once
#include "FGSaveInterface.h"
#include "FGSubsystem.h"
#include "FINNetworkTrace.h"
#include "Signals/FINSignalData.h"
#include "FINHookSubsystem.generated.h"

UCLASS(Abstract)
class UFINHook : public UObject {
	GENERATED_BODY()
public:
    virtual void Register(UObject* sender) {}
	virtual void Unregister() {}
};

/**
 * Contains the listener list and a list of attached hook for a object.
 */
USTRUCT(BlueprintType)
struct FICSITNETWORKS_API FFINHookData {
	GENERATED_BODY()

	UPROPERTY()
	TSet<UFINHook*> Hooks;
};

UCLASS()
class AFINHookSubsystem : public AFGSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
private:
	/**
	 * Contains the hook data of all objects which have hooks attached to them.
	 */
	UPROPERTY()
	TMap<UObject*, FFINHookData> Data;
	FCriticalSection DataLock;

	/**
	 * Contains the list of hooks accosiated with a class
	 */
	static TMap<UClass*, TSet<TSubclassOf<UFINHook>>> HookRegistry;

public:
	/**
	 * Gets the loaded hook subsystem in the given world.
	 *
	 * @param[in]	WorldContext	the world context from were to load the hook subsystem.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Hooks", meta = (WorldContext = "WorldContext"))
    static AFINHookSubsystem* GetHookSubsystem(UObject* WorldContext);

	/**
	 * Registers a new hook type for the given class.
	 *
	 * @param[in]	clazz	type you want to register the hook for
	 * @param[in]	hook	the hook type you want to register
	 */
	static void RegisterHook(UClass* clazz, TSubclassOf<UFINHook> hook);

	/**
	 * Attaches all hooks to the given object there are for the type of the given object.
	 *
	 * @param[in]	object	the object you want to attach the hooks to
	 */
	void AttachHooks(UObject* object);

	/**
	 * Removes all hook attachments from the given object.
	 *
	 * @param[in]	object	the object you want to remove all hooks from
	 */
	void ClearHooks(UObject* object);
};
