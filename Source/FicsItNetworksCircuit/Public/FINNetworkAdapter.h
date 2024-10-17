#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FINAdvancedNetworkConnectionComponent.h"
#include "Buildables/FGBuildable.h"

#include "FINNetworkAdapter.generated.h"

class UFINNetworkAdapterReference;

/**
 * Used to store pre defined infromation about a network adapter attached to a referd type of actor.
 */
USTRUCT()
struct FICSITNETWORKSCIRCUIT_API FFINAdapterSettings {
	GENERATED_BODY()

public:
	FVector loc;
	FRotator rot;
	bool mesh;
	int maxCables;
};

/**
 * Allows to connect pre existing actors, buildables and factories to the computer network.
 */
UCLASS()
class FICSITNETWORKSCIRCUIT_API AFINNetworkAdapter : public AFGBuildable {
	GENERATED_BODY()

public:
	static TArray<TPair<UClass*, FFINAdapterSettings>> settings;
	static void RegistererAdapterSetting(UClass*, FFINAdapterSettings);
	static void RegistererAdapterSetting(FString BPPath, FFINAdapterSettings);
	static void RegisterAdapterSettings();

	static bool FindConnection(AActor* Actor, FVector HitLocation, FTransform& OutTransform, bool& OutMesh, int& OutMaxCables);
	
	/** the building this adapter is attached to */
	UPROPERTY(SaveGame, Replicated)
	AFGBuildable* Parent = nullptr;
	
	/** the network connector of the adapter */
	UPROPERTY(EditAnywhere, Replicated)
	UFINAdvancedNetworkConnectionComponent* Connector = nullptr;
	
	/** the network adapter reference attached to the parent factory */
	UPROPERTY()
	UFINNetworkAdapterReference* Attachment = nullptr;
	
	/** the visible mesh of the adapter */
	UPROPERTY(EditAnywhere)
	UFGColoredInstanceMeshProxy* ConnectorMesh = nullptr;

	AFINNetworkAdapter();
	~AFINNetworkAdapter();
	
	// Begin AActor
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	// End AActor

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	virtual bool NeedTransform_Implementation() override;
	virtual void GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) override;
	// End IFGSaveInterface
};

/**
 * Used to refer the network adapter in the parent factory
 */
UCLASS()
class FICSITNETWORKSCIRCUIT_API UFINNetworkAdapterReference : public UActorComponent {
	GENERATED_BODY()

public:
	/** the reference to the actual network adapter */
	UPROPERTY(SaveGame)
	AFINNetworkAdapter* Ref = nullptr;

	UFINNetworkAdapterReference();
	~UFINNetworkAdapterReference();
};