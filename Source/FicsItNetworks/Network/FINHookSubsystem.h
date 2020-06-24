#pragma once
#include "FGSubsystem.h"
#include "FINNetworkTrace.h"
#include "Signals/FINSignal.h"
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
USTRUCT()
struct FFINHookData {
	GENERATED_BODY()

	UPROPERTY()
	TSet<FFINNetworkTrace> Listeners;

	UPROPERTY()
	TSet<UFINHook*> Hooks;
};

UCLASS()
class AFINHookSubsystem : public AFGSubsystem {
	GENERATED_BODY()
private:
	/**
	 * Contains the hook data of all objects which have hooks attached to them.
	 */
	UPROPERTY()
	TMap<UObject*, FFINHookData> Data;

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
	 * Retrieves the listener list of the given object.
	 * Creates the list if it doesn't exist.
	 *
	 * @param[in]	object	the object you want to get the listener list for.
	 * @return	reference to the listener list
	 */
	TSet<FFINNetworkTrace>& GetListeners(UObject* object);

	/**
	 * Sends the given signal to all listeners of the given object.
	 *
	 * @param[in]	object	the object that should emit the signal
	 * @param[in]	signal	the signal that the object shouls emit
	 */
	void EmitSignal(UObject* object, FFINSignal signal);
	
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

	/**
	 * Adds the given listener to the listener list of the given object.
	 * Will hook the given object if it is not already.
	 * The listener should be in a trace coming from sender for proper validation.
	 *
	 * @param[in]	sender		the object you want to add the listener to
	 * @param[in]	listener	the listener you want to add to the listener list
	 */
	void AddListener(UObject* sender, FFINNetworkTrace listener);

	/**
	 * Removes the given listener from the listener list of the given object.
	 * Will remove all hooks from the object if no listener is left.
	 * Will remove all traces that point to listener from the listener.
	 *
	 * @param[in]	sender		the object you want to remove the listener from
	 * @param[in]	listener	the listener you want to remove from the listener list
	 */
	void RemoveListener(UObject* sender, UObject* listener);
};
