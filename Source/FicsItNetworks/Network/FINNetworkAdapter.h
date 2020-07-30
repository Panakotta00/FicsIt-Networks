#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FGSaveInterface.h"
#include "Buildables/FGBuildableFactory.h"
#include "FINAdvancedNetworkConnectionComponent.h"

#include "FINNetworkAdapter.generated.h"

class UFINNetworkAdapterReference;

/**
 * Used to store pre defined infromation about a network adapter attached to a referd type of actor.
 */
USTRUCT()
struct FICSITNETWORKS_API FFINAdapterSettings {
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
class FICSITNETWORKS_API AFINNetworkAdapter : public AActor, public IFGSaveInterface {
	GENERATED_BODY()

public:
	static TArray<TPair<UClass*, FFINAdapterSettings>> settings;
	static void RegistererAdapterSetting(UClass*, FFINAdapterSettings);
	static void RegistererAdapterSetting(FString BPPath, FFINAdapterSettings);
	static void RegisterAdapterSettings();

	/** the building this adapter is attached to */
	UPROPERTY(SaveGame)
	AFGBuildable* Parent = nullptr;
	
	/** the network connector of the adapter */
	UPROPERTY()
	UFINAdvancedNetworkConnectionComponent* Connector = nullptr;
	
	/** the network adapter reference attached to the parent factory */
	UPROPERTY()
	UFINNetworkAdapterReference* Attachment = nullptr;
	
	/** the visible mesh of the adapter */
	UPROPERTY()
	UStaticMeshComponent* ConnectorMesh = nullptr;

	AFINNetworkAdapter();
	~AFINNetworkAdapter();
	
	// Begin AActor
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
class FICSITNETWORKS_API UFINNetworkAdapterReference : public UActorComponent {
	GENERATED_BODY()

public:
	/** the reference to the actual network adapter */
	UPROPERTY()
	AFINNetworkAdapter* Ref = nullptr;

	UFINNetworkAdapterReference();
	~UFINNetworkAdapterReference();
};