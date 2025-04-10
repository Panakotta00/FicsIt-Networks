//
#include "FINSectionablePanelAttachmentPointType.h"

TArray<TSubclassOf<UFGAttachmentPointType>> UFINSectionablePanelAttachmentPointType::GetAllowedAttachmentPointSnapTypes() {
	return mAllowedAttachmentPointSnapTypes;
}

bool UFINSectionablePanelAttachmentPointType::CanAttach_Implementation(const FFGAttachmentPoint& point,	const FFGAttachmentPoint& targetPoint) const {
	const TSubclassOf<UFGAttachmentPointType> Type = point.Type;
	const TSubclassOf<UFGAttachmentPointType> SnappedType = targetPoint.Type;
	UFGAttachmentPointType* LocalObject = Type.GetDefaultObject();
	const UFGAttachmentPointType* RemoteObject = SnappedType.GetDefaultObject();
	bool Pass = false;
	for (auto SnapType : LocalObject->mAllowedAttachmentPointSnapTypes) {
		if(RemoteObject->IsA(SnapType)) {
			Pass = true;
			break;
		}
	}
	return Pass;
	
	//return Super::CanAttach_Implementation(point, targetPoint);
}
