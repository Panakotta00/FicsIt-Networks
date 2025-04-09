// 

#pragma once

#include "CoreMinimal.h"
#include "FGAttachmentPoint.h"
#include "Runtime/CoreUObject/Public/Templates/SubclassOf.h"

#include "FINSectionablePanelAttachmentPointType.generated.h"

class UFGAttachmentPointType;
/**
 * 
 */
UCLASS()
class FICSITNETWORKS_API UFINSectionablePanelAttachmentPointType : public UFGAttachmentPointType {
	GENERATED_BODY()
	
	public:

	UFUNCTION(BlueprintCallable)
	TArray< TSubclassOf< UFGAttachmentPointType > > GetAllowedAttachmentPointSnapTypes();
	virtual bool CanAttach_Implementation(const FFGAttachmentPoint& point, const FFGAttachmentPoint& targetPoint) const override;
};


