#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "FGSaveInterface.h"
#include "FIRHookSubsystem.generated.h"

UCLASS(Abstract)
class FICSITREFLECTION_API UFIRHook : public UObject {
	GENERATED_BODY()
public:
    virtual void Register(UObject* sender) {}
	virtual void Unregister() {}
};

/**
 * Contains the listener list and a list of attached hook for a object.
 */
USTRUCT(BlueprintType)
struct FICSITREFLECTION_API FFIRHookData {
	GENERATED_BODY()

	UPROPERTY()
	TSet<UFIRHook*> Hooks;
};

UCLASS()
class FICSITREFLECTION_API AFIRHookSubsystem : public AModSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
private:
	/**
	 * Contains the hook data of all objects which have hooks attached to them.
	 */
	UPROPERTY()
	TMap<UObject*, FFIRHookData> Data;
	FCriticalSection DataLock;

	/**
	 * Contains the list of hooks accosiated with a class
	 */
	static TMap<UClass*, TSet<TSubclassOf<UFIRHook>>> HookRegistry;

public:
	/**
	 * Gets the loaded hook subsystem in the given world.
	 *
	 * @param[in]	WorldContext	the world context from were to load the hook subsystem.
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContext"))
    static AFIRHookSubsystem* GetHookSubsystem(UObject* WorldContext);

	/**
	 * Registers a new hook type for the given class.
	 *
	 * @param[in]	clazz	type you want to register the hook for
	 * @param[in]	hook	the hook type you want to register
	 */
	static void RegisterHook(UClass* clazz, TSubclassOf<UFIRHook> hook);

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
