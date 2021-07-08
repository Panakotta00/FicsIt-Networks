#pragma once

#include "CoreMinimal.h"
#include "FIVSEdStyle.generated.h"

USTRUCT(BlueprintType)
struct FFIVSEdStyle : public FSlateWidgetStyle {
	GENERATED_USTRUCT_BODY()

	static const FFIVSEdStyle& GetDefault();
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush DataPinIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush DataPinConnectedIcon;
};
