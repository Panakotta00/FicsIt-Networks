#include "Components/FINScreenHolo.h"
#include "FGColoredInstanceMeshProxy.h"
#include "Buildables/FGBuildableFoundation.h"
#include "Buildables/FGBuildableWall.h"
#include "Components/FINScreen.h"

AFINScreenHolo::AFINScreenHolo() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINScreenHolo::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
	
	ConstructParts();
}

void AFINScreenHolo::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if ((OldScreenHeight != ScreenHeight || OldScreenWidth != ScreenWidth)) {
		OldScreenHeight = ScreenHeight;
		OldScreenWidth = ScreenWidth;
		
		ConstructParts();
	}
}

void AFINScreenHolo::BeginPlay() {
	Super::BeginPlay();
	
	//RerunConstructionScripts(); TODO: Check if really needed
}

void AFINScreenHolo::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	SetActorTickEnabled(false);
	
	for (UStaticMeshComponent* Part : Parts) {
		Part->UnregisterComponent();
		Part->SetActive(false);
		Part->DestroyComponent();
	}
	Parts.Empty();
	SetActorHiddenInGame(true);
}

void AFINScreenHolo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINScreenHolo, ScreenWidth);
	DOREPLIFETIME(AFINScreenHolo, ScreenHeight);
}

bool AFINScreenHolo::DoMultiStepPlacement(bool isInputFromARelease) {
	if (bSnapped) {
		return true;
	} else {
		bSnapped = true;
		return false;
	}
}

int32 AFINScreenHolo::GetBaseCostMultiplier() const {
	return FMath::Abs(ScreenWidth) * FMath::Abs(ScreenHeight);
}

bool AFINScreenHolo::IsValidHitResult(const FHitResult& hitResult) const {
	if (bSnapped) return true;
	return hitResult.GetActor() && (hitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableWall>() || hitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableFoundation>());
}

void AFINScreenHolo::SetHologramLocationAndRotation(const FHitResult& hitResult) {
	if (bSnapped) {
		// Calculate new Screen Size
		FVector PlaneNormal = Normal;
		PlaneNormal.Normalize(0.01);
		FVector PlanePosition = GetActorLocation();
		FVector LineDirection = (hitResult.TraceEnd - hitResult.TraceStart);
		LineDirection.Normalize(0.01);
		FVector LinePosition = hitResult.TraceStart;
		float PlaneDotLine = FVector::DotProduct(PlaneNormal, LineDirection);
		if (PlaneDotLine == 0.0) {
			ScreenHeight = 1;
			ScreenWidth = 1;
		} else {
			double t = (FVector::DotProduct(PlaneNormal, PlanePosition) - FVector::DotProduct(PlaneNormal, LinePosition)) / PlaneDotLine;
			FVector ViewPostion = LinePosition + (LineDirection * t);
			FVector ScreenDiagonal = ViewPostion - PlanePosition;
			FVector TestDiagonal = GetActorRotation().UnrotateVector(ScreenDiagonal);
			ScreenHeight = FMath::Clamp(FMath::RoundToInt((TestDiagonal.Z + (TestDiagonal.Z < 0 ? -100.0 : 100.0)) / 100.0), -10, 10);
			ScreenWidth = FMath::Clamp(FMath::RoundToInt((TestDiagonal.Y + (TestDiagonal.Y < 0 ? -100.0 : 100.0)) / 100.0), -10, 10);
			if (ScreenHeight == 0) ScreenHeight = 1;
			if (ScreenWidth == 0) ScreenWidth = 1;
		}
	} else {
		Normal = hitResult.ImpactNormal;
		ScreenHeight = 1;
		ScreenWidth = 1;
		FTransform SnappedActorTransform = FTransform();
		FVector UpVector = FVector(1,0,0);
		if (AActor* SnappedActor = hitResult.GetActor()) {
			SnappedActorTransform = SnappedActor->GetActorTransform();
		}
		FQuat Quat;
		if (FVector::Coincident(UpVector * -1, Normal) || FVector::Coincident(UpVector, Normal)) {
			Quat = Normal.ToOrientationQuat();
		} else {
			FVector RotationAxis = FVector::CrossProduct(UpVector, Normal);
			RotationAxis.Normalize();
			float DotProduct = FVector::DotProduct(UpVector, Normal);
			float RotationAngle = acosf(DotProduct);
			Quat = FQuat(RotationAxis, RotationAngle);
		}
		FQuat NewQuat = Quat * FRotator(0, 0, GetScrollRotateValue()).Quaternion();
		SetActorLocationAndRotation(hitResult.ImpactPoint, NewQuat.Rotator());
	}
}

AActor* AFINScreenHolo::Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID) {
	bSnapped = false;

	return Super::Construct(out_children, netConstructionID);
}

void AFINScreenHolo::CheckValidFloor() {}

void AFINScreenHolo::ConfigureActor(AFGBuildable* inBuildable) const {
	Super::ConfigureActor(inBuildable);
	
	AFINScreen* Screen = Cast<AFINScreen>(inBuildable);
	Screen->ScreenHeight = ScreenHeight;
	Screen->ScreenWidth = ScreenWidth;
}

void AFINScreenHolo::ConstructParts() {
	// Clear Components
	for (UStaticMeshComponent* comp : Parts) {
		comp->UnregisterComponent();
		comp->SetActive(false);
		comp->DestroyComponent();
	}
	Parts.Empty();

	// Create Components
	if (mBuildClass) {
		UStaticMesh* MiddlePartMesh = Cast<AFINScreen>(mBuildClass->GetDefaultObject())->ScreenMiddle;
		UStaticMesh* EdgePartMesh = Cast<AFINScreen>(mBuildClass->GetDefaultObject())->ScreenEdge;
		UStaticMesh* CornerPartMesh = Cast<AFINScreen>(mBuildClass->GetDefaultObject())->ScreenCorner;
		AFINScreen::SpawnComponents(UStaticMeshComponent::StaticClass(), ScreenWidth, ScreenHeight, MiddlePartMesh, EdgePartMesh, CornerPartMesh, this, RootComponent, Parts);
		RootComponent->SetMobility(EComponentMobility::Movable);
		for (UStaticMeshComponent* Part : Parts) {
			Part->SetMobility(EComponentMobility::Movable);
			Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}
