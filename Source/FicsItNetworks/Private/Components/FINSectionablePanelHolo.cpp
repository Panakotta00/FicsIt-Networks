// 


#include "Components/FINSectionablePanelHolo.h"

#include "FGBuildableFoundation.h"
#include "FGBuildableWall.h"



// Sets default values
AFINSectionablePanelHolo::AFINSectionablePanelHolo() {
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//mSnapPointIndicator = CreateDefaultSubobject<UStaticMeshComponent>("IndicatorSphere");
	//mSnapPointIndicator->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void AFINSectionablePanelHolo::BeginPlay() {
	Super::BeginPlay();
	//mSnapPointIndicator->SetMobility(EComponentMobility::Movable);
	//mSnapPointIndicator->SetRelativeLocation(FVector());
	//if(mSnapPointIndicatorMesh != nullptr) {
	//	mSnapPointIndicator->SetStaticMesh(mSnapPointIndicatorMesh);
	//}
	//if(mSnapPointIndicatorMaterial) {
	//	mSnapPointIndicator->SetMaterial(0, mSnapPointIndicatorMaterial);
	//}
	//mSnapPointIndicator->SetVisibility(true);
	//mSnapPointIndicator->SetRelativeScale3D(FVector(0.1, 0.1, 0.1));
	//mSnapPointIndicator->RegisterComponent();
}

// Called every frame
void AFINSectionablePanelHolo::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	//if(Snapped && mLocalSnappedAttachmentPoint != nullptr && mSnappedAttachmentPoint != nullptr) {
	//	mSnapPointIndicator->SetRelativeLocation(mLocalSnappedAttachmentPoint->RelativeTransform.GetLocation());
	//	mSnapPointIndicator->SetVisibility(true);
	//}else {
	//	//if(mCachedAttachmentPoints.Num() > mSelectedHologramAttachmentPointIndex) {
	//	//	FFGAttachmentPoint Point = mCachedAttachmentPoints[mSelectedHologramAttachmentPointIndex];
	//	//	mSnapPointIndicator->SetRelativeLocation(Point.RelativeTransform.GetLocation());
	//	//	mSnapPointIndicator->SetVisibility(true);
	//	//}else {
	//	//}
	//	mSnapPointIndicator->SetVisibility(false);
	//}
}

void AFINSectionablePanelHolo::CheckValidPlacement() {
	Super::CheckValidPlacement();
}

bool AFINSectionablePanelHolo::TrySnapToActor(const FHitResult& hitResult) {
	return Super::TrySnapToActor(hitResult);
}

#pragma optimize("", off)
//bool AFINSectionablePanelHolo::TrySnapToActor(const FHitResult& hitResult) {
//	bool bTrySnapToActor = Super::TrySnapToActor(hitResult);
//	if(mLocalSnappedAttachmentPoint != nullptr && mSnappedAttachmentPoint != nullptr) {
//		TSubclassOf<UFGAttachmentPointType> Type = mLocalSnappedAttachmentPoint->Type;
//		TSubclassOf<UFGAttachmentPointType> SnappedType = mSnappedAttachmentPoint->Type;
//		UFGAttachmentPointType* LocalObject = Type.GetDefaultObject();
//		UFGAttachmentPointType* RemoteObject = SnappedType.GetDefaultObject();
//		bool Pass = false;
//		for (auto SnapType : LocalObject->mAllowedAttachmentPointSnapTypes) {
//			if(RemoteObject->IsA(SnapType)) {
//				Pass = true;
//				break;
//			}
//		}
//		if(!Pass) {
//			mLocalSnappedAttachmentPoint = nullptr;
//			bTrySnapToActor = false;
//		}
//		if(bTrySnapToActor) {
//			
//		}
//	}else {
//		
//	}
//	Snapped = bTrySnapToActor;
//	return bTrySnapToActor;
//}

void AFINSectionablePanelHolo::SetHologramLocationAndRotation(const FHitResult& hitResult) {
	Super::SetHologramLocationAndRotation(hitResult);
}

void AFINSectionablePanelHolo::SetSnapToGuideLines(bool isEnabled) {
	Super::SetSnapToGuideLines(false);
}

int32 AFINSectionablePanelHolo::GetRotationStep() const {
	return Super::GetRotationStep();
}

void AFINSectionablePanelHolo::UpdateRotationValuesFromTransform() {
	Super::UpdateRotationValuesFromTransform();
}

bool AFINSectionablePanelHolo::IsHologramIdenticalToActor(AActor* actor, const FVector& hologramLocationOffset) const {
	//bool bIsHologramIdenticalToActor = Super::IsHologramIdenticalToActor(actor, hologramLocationOffset);
	return false;
}

const FFGAttachmentPoint* AFINSectionablePanelHolo::SelectCandidateForAttachment(const TArray<const FFGAttachmentPoint*>& Candidates, AFGBuildable* pBuildable,	const FFGAttachmentPoint& BuildablePoint, const FHitResult& HitResult) {
	//int mSelectedHologramAttachmentPointIndex; // eax
	//bool v6; // sf
	//int v7; // edx
	//
	//if ( Candidates.Num() <= 0 )
	//	return nullptr;
	//mSelectedHologramAttachmentPointIndex = this->mSelectedHologramAttachmentPointIndex;
	//if ( mSelectedHologramAttachmentPointIndex < 0 )
	//{
	//	for (auto Candidate : Candidates) {
	//		
	//	}
	//	
	//	do
	//	{
	//		v6 = Candidates->ArrayNum + mSelectedHologramAttachmentPointIndex < 0;
	//		mSelectedHologramAttachmentPointIndex += Candidates->ArrayNum;
	//		this->mSelectedHologramAttachmentPointIndex = mSelectedHologramAttachmentPointIndex;
	//	}
	//	while ( v6 );
	//}
	//v7 = mSelectedHologramAttachmentPointIndex % Candidates->ArrayNum;
	//this->mSelectedHologramAttachmentPointIndex = v7;
	//return *(const FFGAttachmentPoint **)&Candidates->AllocatorInstance.Data[8 * v7];
	
	const FFGAttachmentPoint* SelectCandidateForAttachment = Super::SelectCandidateForAttachment(Candidates, pBuildable, BuildablePoint, HitResult);
	return SelectCandidateForAttachment;
}
#pragma optimize("", on)

