#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FINLabelContainerInterface.generated.h"

UINTERFACE()
class FICSITNETWORKSMISC_API UFINLabelContainerInterface : public UInterface {
	GENERATED_BODY()
};

class FICSITNETWORKSMISC_API IFINLabelContainerInterface {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FString GetLabel();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetLabel(const FString& InLabel);
};