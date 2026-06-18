#pragma once

#include "CoreMinimal.h"
#include "WheeledVehicles/FGTargetPoint.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "FIRTargetPoint.generated.h"

USTRUCT()
struct FICSITREFLECTION_API FFIRTargetPoint {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FVector Pos = FVector::ZeroVector;
	
	UPROPERTY(SaveGame)
	FRotator Rot = FRotator::ZeroRotator;

	UPROPERTY(SaveGame)
	float Speed = 0.0f;

	UPROPERTY(SaveGame)
	float Wait = 0.0f;

	FFIRTargetPoint() = default;
	FFIRTargetPoint(const FVector& Pos, const FRotator& Rot, float Speed, float Wait) : Pos(Pos), Rot(Rot), Speed(Speed), Wait(Wait) {}
	FFIRTargetPoint(AFGTargetPoint* Target) :
		Pos(Target->GetActorLocation()),
		Rot(Target->GetActorRotation()),
		Speed(Target->mTargetSpeed),
		Wait(Target->mWaitTime) {}

	AFGTargetPoint* ToWheeledTargetPoint(UObject* WorldContext) const {
		TSubclassOf<AFGTargetPoint> Clazz = nullptr;
		if (!Clazz) Clazz = LoadObject<UClass>(NULL, TEXT("/Game/FactoryGame/Buildable/Vehicle/BP_VehicleTargetPoint.BP_VehicleTargetPoint_C"));
		FActorSpawnParameters Params;
		Params.bDeferConstruction = true;
		AFGTargetPoint* Target = WorldContext->GetWorld()->SpawnActor<AFGTargetPoint>(Clazz, Pos, Rot, Params);
		Target->mTargetSpeed = Speed;
		Target->mWaitTime = Wait;
		return Cast<AFGTargetPoint>(UGameplayStatics::FinishSpawningActor(Target, FTransform(Rot.Quaternion(), Pos)));
	}
};
