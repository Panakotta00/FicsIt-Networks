#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "FINMicrocontrollerReference.generated.h"

UCLASS()
class FICSITNETWORKSMICROCONTROLLER_API UFINMicrocontrollerReference : public UActorComponent {
	GENERATED_BODY()
public:
	UPROPERTY()
	AFINMicrocontroller* Microcontroller = nullptr;

	UFUNCTION(BlueprintCallable)
	static AFINMicrocontroller* FindMicrocontroller(const AActor* Actor) {
		if (!IsValid(Actor)) return nullptr;
		auto ref = Actor->GetComponentByClass<UFINMicrocontrollerReference>();
		if (ref) {
			return ref->Microcontroller;
		}
		return nullptr;
	}
};
