#pragma once

#include "CoreMinimal.h"
#include "SlateBrush.h"
#include "Styling/SlateWidgetStyle.h"
#include "FIVSEdGraphViewerStyle.generated.h"

USTRUCT(BlueprintType)
struct FFIVSEdPinStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	static const FFIVSEdPinStyle& GetDefault();
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush ConnectionIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush ConnectionIconConnected;
};

USTRUCT(BlueprintType)
struct FFIVSEdNodeStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	static const FFIVSEdNodeStyle& GetDefault();
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush Outline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush ErrorOutline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FMargin OutlinePadding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush HeaderEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush HeaderInline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FMargin HeaderPadding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle HeaderTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush Background;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FMargin Padding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FVector2D CenterSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle OperatorTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FMargin OperatorPadding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
    FFIVSEdPinStyle ExecInputPinStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FFIVSEdPinStyle ExecOutputPinStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FFIVSEdPinStyle DataInputPinStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FFIVSEdPinStyle DataOutputPinStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	double PinOpacityDisabled = 0.8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle PinTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FMargin PinTextMargin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FMargin PinMargin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush PinOutline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FMargin PinPadding;
};

USTRUCT(BlueprintType)
struct FFIVSEdGraphViewerStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	static const FFIVSEdGraphViewerStyle& GetDefault();
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush Background;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor GridMinorColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor GridMajorColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush SelectionBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FFIVSEdNodeStyle NodeStyle;
};
