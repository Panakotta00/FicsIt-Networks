// 
#include "FINRestrictedModuleHolo.h"

// Sets default values
AFINRestrictedModuleHolo::AFINRestrictedModuleHolo() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AFINRestrictedModuleHolo::BeginPlay() {
	Super::BeginPlay();
	
}

// Called every frame
void AFINRestrictedModuleHolo::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

bool AFINRestrictedModuleHolo::TrySnapToActor(const FHitResult& hitResult) {
	Super::TrySnapToActor(hitResult);
	if(ValidTargets.Num() > 0) {
		bIsValid = false;
		AActor* Actor = hitResult.GetActor();
		if(IsValid(Actor)) {
			for (auto& Allowed : ValidTargets) {
				if (Actor->IsA(Allowed)) {
					bIsValid = true;
					break;
				}
			}
		}
	}
	return bIsValid;
}

