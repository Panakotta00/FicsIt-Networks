#pragma once

#include "CoreMinimal.h"
#include "FINStructInterfaces.h"
#include "FINLabelContainerInterface.generated.h"

USTRUCT()
struct FICSITNETWORKSMISC_API FFINLabelContainerInterface {
	GENERATED_BODY()

	FIN_STRUCT_INTERFACE(FFINLabelContainerInterface)

	virtual FString GetLabel() const { return FString(); }

	virtual void SetLabel(const FString& InLabel) {}
};
