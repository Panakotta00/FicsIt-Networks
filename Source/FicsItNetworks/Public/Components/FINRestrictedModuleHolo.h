// 

#pragma once

#include "CoreMinimal.h"
#include "FINModuleSystemHolo.h"

#include "FINRestrictedModuleHolo.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINRestrictedModuleHolo : public AFINModuleSystemHolo {
	GENERATED_BODY()

	public:
		// Sets default values for this actor's properties
		AFINRestrictedModuleHolo();

	protected:
		// Called when the game starts or when spawned
		virtual void BeginPlay() override;

	public:
		// Called every frame
		virtual void Tick(float DeltaTime) override;
	
		virtual bool TrySnapToActor(const FHitResult& hitResult) override;

		UPROPERTY(EditDefaultsOnly)
		TArray<TSubclassOf<AFGBuildable>> ValidTargets; 
};
