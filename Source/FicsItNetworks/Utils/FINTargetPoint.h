#pragma once

#include "FGWheeledVehicle.h"
#include "FGTargetPoint.h"
#include "FINTargetPoint.generated.h"

USTRUCT()
struct FFINTargetPoint {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FVector Pos;
	
	UPROPERTY(SaveGame)
	FRotator Rot;

	UPROPERTY(SaveGame)
	float Speed;

	UPROPERTY(SaveGame)
	float Wait;

	FFINTargetPoint() = default;
	FFINTargetPoint(const FVector& Pos, const FRotator& Rot, float Speed, float Wait) : Pos(Pos), Rot(Rot), Speed(Speed), Wait(Wait) {}
	FFINTargetPoint(AFGTargetPoint* Target) :
		Pos(Target->GetActorLocation()),
		Rot(Target->GetActorRotation()),
		Speed(Target->GetTargetSpeed()),
		Wait(Target->GetWaitTime()) {}

	AFGTargetPoint* ToWheeledTargetPoint(UObject* WorldContext) const {
		TSubclassOf<AFGTargetPoint> Clazz = nullptr;
		if (!Clazz) Clazz = LoadObject<UClass>(NULL, TEXT("/Game/FactoryGame/Buildable/Vehicle/BP_VehicleTargetPoint.BP_VehicleTargetPoint_C"));
		FActorSpawnParameters Params;
		Params.bDeferConstruction = true;
		AFGTargetPoint* Target = WorldContext->GetWorld()->SpawnActor<AFGTargetPoint>(Clazz, Pos, Rot, Params);
		Target->SetTargetSpeed(Speed);
		Target->SetWaitTime(Wait);
		return Cast<AFGTargetPoint>(UGameplayStatics::FinishSpawningActor(Target, FTransform(Rot.Quaternion(), Pos)));
	}
};
