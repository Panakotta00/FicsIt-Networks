#pragma once

#include "CoreMinimal.h"
#include "FINLuaReferenceCollector.h"
#include "FINLuaRuntime.h"
#include "FINLuaRuntimePersistence.h"
#include "Buildables/FGBuildable.h"
#include "FINMicrocontroller.generated.h"

class UFINMicrocontrollerReference;

UENUM(BlueprintType)
enum EFINMicrocontrollerState {
	FIN_Microcontroller_State_Stopped,
	FIN_Microcontroller_State_Running,
	FIN_Microcontroller_State_Failed,
};

UCLASS(Blueprintable)
class FICSITNETWORKSMICROCONTROLLER_API AFINMicrocontroller : public AFGBuildable {
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly)
	UFGInventoryComponent* Inventory;

	UPROPERTY(SaveGame)
	AActor* NetworkComponent = nullptr;

	UPROPERTY()
	UFINMicrocontrollerReference* Reference = nullptr;

	AFINMicrocontroller();

	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	// End AActor

	// Begin IFGSaveInterface
	virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	// End IFGSaveInterface

	UFUNCTION(BlueprintCallable)
	void ToggleRuntime();
	UFUNCTION(BlueprintCallable)
	void StartRuntime();
	UFUNCTION(BlueprintCallable)
	void StopRuntime();
	UFUNCTION(BlueprintCallable)
	void CrashRuntime(const FString& message);

	TOptional<FString> GetCode() const;
	UFUNCTION(BlueprintCallable)
	FString GetCode(const FString& Default) const;
	UFUNCTION(BlueprintCallable)
	void SetCode(const FString& Code);

	UFUNCTION(BlueprintCallable)
	FString GetDebugInfo() const;

	UFUNCTION(BlueprintCallable)
	EFINMicrocontrollerState GetStatus() const;

protected:
	void SetupRuntime();

	FFINLuaRuntime Runtime;
	UPROPERTY()
	FFINLuaReferenceCollector ReferenceCollector;
	UPROPERTY(SaveGame)
	FString Error;

	UPROPERTY(SaveGame)
	FFINLuaRuntimePersistenceState PersistenceState;
};
