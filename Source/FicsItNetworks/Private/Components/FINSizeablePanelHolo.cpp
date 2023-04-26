#include "Components/FINSizeablePanelHolo.h"
#include "FGColoredInstanceMeshProxy.h"
#include "Components/FINSizeablePanel.h"

AFINSizeablePanelHolo::AFINSizeablePanelHolo() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINSizeablePanelHolo::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
	
	// Clear Components
	for (UStaticMeshComponent* comp : Parts) {
		comp->UnregisterComponent();
		comp->SetActive(false);
		comp->DestroyComponent();
	}
	Parts.Empty();

	// Create Components
	if (mBuildClass) {
		UStaticMesh* UL = Cast<AFINSizeablePanel>(mBuildClass->GetDefaultObject())->PanelCornerMesh;
		UStaticMesh* UC = Cast<AFINSizeablePanel>(mBuildClass->GetDefaultObject())->PanelSideMesh;
		UStaticMesh* CC = Cast<AFINSizeablePanel>(mBuildClass->GetDefaultObject())->PanelCenterMesh;
		UStaticMesh* Con = Cast<AFINSizeablePanel>(mBuildClass->GetDefaultObject())->PanelConnectorMesh;
		AFINSizeablePanel::SpawnComponents(UStaticMeshComponent::StaticClass(), PanelWidth, PanelHeight, UL, UC, CC, Con, this, RootComponent, Parts);
		RootComponent->SetMobility(EComponentMobility::Movable);
		for (UStaticMeshComponent* Part : Parts) {
			Part->SetMobility(EComponentMobility::Movable);
			Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AFINSizeablePanelHolo::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if ((OldPanelHeight != PanelHeight || OldPanelWidth != PanelWidth)) {
		OldPanelHeight = PanelHeight;
		OldPanelWidth = PanelWidth;
		
		RerunConstructionScripts();
	}
}

void AFINSizeablePanelHolo::EndPlay(const EEndPlayReason::Type EndPlayReason) {
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

void AFINSizeablePanelHolo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINSizeablePanelHolo, PanelWidth);
	DOREPLIFETIME(AFINSizeablePanelHolo, PanelHeight);
}

bool AFINSizeablePanelHolo::DoMultiStepPlacement(bool isInputFromARelease) {
	if (bSnapped) {
		return true;
	} else {
		bSnapped = true;
		return false;
	}
}

int32 AFINSizeablePanelHolo::GetBaseCostMultiplier() const {
	return FGenericPlatformMath::Max((FMath::Abs(PanelWidth) * FMath::Abs(PanelHeight)) / 10, 1);
}

bool AFINSizeablePanelHolo::IsValidHitResult(const FHitResult& hitResult) const {
	if (bSnapped) return IsValid(hitResult.GetActor());
	//return hitResult.GetActor() && (hitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableWall>() || hitResult.GetActor()->GetClass()->IsChildOf<AFGBuildableFoundation>());
	return IsValid(hitResult.GetActor());
}

void AFINSizeablePanelHolo::SetHologramLocationAndRotation(const FHitResult& HitResult) {
	if (bSnapped) {
		FVector PlaneNormal = Normal;
		PlaneNormal.Normalize(0.01);
		FVector PlanePosition = GetActorLocation();
		FVector LineDirection = (HitResult.TraceEnd - HitResult.TraceStart);
		LineDirection.Normalize(0.01);
		FVector LinePosition = HitResult.TraceStart;
		float PlaneDotLine = FVector::DotProduct(PlaneNormal, LineDirection);
		if (PlaneDotLine == 0.0) {
			PanelHeight = 1;
			PanelWidth = 1;
		} else {
			double t = (FVector::DotProduct(PlaneNormal, PlanePosition) - FVector::DotProduct(PlaneNormal, LinePosition)) / PlaneDotLine;
			FVector ViewPostion = LinePosition + (LineDirection * t);
			FVector PanelDiagonal = ViewPostion - PlanePosition;
			FVector TestDiagonal = GetActorRotation().UnrotateVector(PanelDiagonal);
			PanelHeight = FMath::Clamp(FMath::RoundToInt((TestDiagonal.Z + (TestDiagonal.Z < 0 ? -10.0 : 10.0)) / 10.0), -10, 10);
			PanelWidth = FMath::Clamp(FMath::RoundToInt((TestDiagonal.Y + (TestDiagonal.Y < 0 ? -10.0 : 10.0)) / 10.0), -10, 10);
			if (PanelHeight == 0) PanelHeight = 1;
			if (PanelWidth == 0) PanelWidth = 1;
		}
	} else {
		Normal = HitResult.ImpactNormal;
		PanelHeight = 1;
		PanelWidth = 1;
		FTransform SnappedActorTransform = FTransform();
		FVector UpVector = FVector(1,0,0);
		if (AActor* SnappedActor = HitResult.GetActor()) {
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
		auto Location = HitResult.GetActor()->GetActorLocation();
		auto Rotation = HitResult.GetActor()->GetActorRotation();
		auto VectorInActorLocalSpace = Rotation.UnrotateVector(HitResult.ImpactPoint - Location);
		FVector GridPos = VectorInActorLocalSpace.GridSnap(5);
		FVector ResDiffPos = GridPos - VectorInActorLocalSpace;
		FVector ResDiffPosR = Rotation.RotateVector(ResDiffPos);
		FVector Res = HitResult.ImpactPoint + ResDiffPosR;
		SetActorLocationAndRotation(Res, NewQuat.Rotator());
	}
}

AActor* AFINSizeablePanelHolo::Construct(TArray<AActor*>& out_children, FNetConstructionID netConstructionID) {
	bSnapped = false;

	return Super::Construct(out_children, netConstructionID);
}

void AFINSizeablePanelHolo::CheckValidFloor() {}

void AFINSizeablePanelHolo::ConfigureActor(AFGBuildable* inBuildable) const {
	Super::ConfigureActor(inBuildable);
	
	AFINSizeablePanel* Panel = Cast<AFINSizeablePanel>(inBuildable);
	Panel->SetPanelSize(PanelWidth, PanelHeight);
}
