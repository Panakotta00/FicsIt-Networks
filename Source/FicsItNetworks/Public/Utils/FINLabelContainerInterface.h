#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FINLabelContainerInterface.generated.h"

UINTERFACE()
class UFINLabelContainerInterface : public UInterface {
	GENERATED_BODY()
};

class IFINLabelContainerInterface {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FString GetLabel();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetLabel(const FString& InLabel);
};