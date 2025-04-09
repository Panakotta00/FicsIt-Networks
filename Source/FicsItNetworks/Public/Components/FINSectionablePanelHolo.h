// 

#pragma once

#include "CoreMinimal.h"
#include "Hologram/FGGenericBuildableHologram.h"

#include "FINSectionablePanelHolo.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINSectionablePanelHolo : public AFGGenericBuildableHologram {
	protected:
		virtual bool IsHologramIdenticalToActor(AActor* actor, const FVector& hologramLocationOffset) const override;

	private:
		GENERATED_BODY()

	public:
		// Sets default values for this actor's properties
		AFINSectionablePanelHolo();

	protected:
		// Called when the game starts or when spawned
		virtual void BeginPlay() override;

	public:
		// Called every frame
		virtual void Tick(float DeltaTime) override;

	protected:
		virtual void CheckValidPlacement() override;

	public:
		virtual bool TrySnapToActor(const FHitResult& hitResult) override;
		virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
		virtual void SetSnapToGuideLines(bool isEnabled) override;
		virtual int32 GetRotationStep() const override;
		virtual void UpdateRotationValuesFromTransform() override;

	protected:
		virtual const FFGAttachmentPoint* SelectCandidateForAttachment(
			const TArray<const FFGAttachmentPoint*>& Candidates, AFGBuildable* pBuildable,
			const FFGAttachmentPoint& BuildablePoint, const FHitResult& HitResult) override;

	public:
		UPROPERTY(EditDefaultsOnly)
		UStaticMesh* mSnapPointIndicatorMesh;
	
		UPROPERTY(EditDefaultsOnly)
		UMaterialInterface* mSnapPointIndicatorMaterial;

	private:
		UStaticMeshComponent* mSnapPointIndicator;
		bool Snapped;
		
};
