#pragma once

#include "CoreMinimal.h"
#include "SlateBasics.h"
#include "FINReflectionUIStyle.generated.h"

USTRUCT(BlueprintType)
struct FFINSplitterStyle : public FSplitterStyle {
	GENERATED_USTRUCT_BODY()

	virtual void GetResources( TArray< const FSlateBrush* >& OutBrushes ) const override;

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	static const FFINSplitterStyle& GetDefault();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush HandleIconBrush;
};

USTRUCT(BlueprintType)
struct FFINReflectionUIStyleStruct : public FSlateWidgetStyle {
	GENERATED_USTRUCT_BODY()

	FFINReflectionUIStyleStruct();

	virtual ~FFINReflectionUIStyleStruct() {}

	virtual void GetResources( TArray< const FSlateBrush* >& OutBrushes ) const override;

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	static const FFINReflectionUIStyleStruct& GetDefault();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush MemberFunc;
	FFINReflectionUIStyleStruct& SetMemberFunc( const FSlateBrush& InMemberFunc ){ MemberFunc = InMemberFunc; return *this; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush ClassFunc;
	FFINReflectionUIStyleStruct& SetClassFunc( const FSlateBrush& InClassFunc){ ClassFunc = InClassFunc; return *this; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush StaticFunc;
	FFINReflectionUIStyleStruct& SetStaticFunc( const FSlateBrush& InStaticFunc ){ StaticFunc = InStaticFunc; return *this; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush MemberAttrib;
	FFINReflectionUIStyleStruct& SetMemberAttrib( const FSlateBrush& InMemberAttrib ){ MemberAttrib = InMemberAttrib; return *this; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush ClassAttrib;
	FFINReflectionUIStyleStruct& SetClassAttrib( const FSlateBrush& InClassAttrib ){ ClassAttrib = InClassAttrib; return *this; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush Signal;
	FFINReflectionUIStyleStruct& SetSignal( const FSlateBrush& InSignal ){ Signal = InSignal; return *this; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle InternalNameTextStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle DisplayNameTextStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle DescriptionTextStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle FlagsTextStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle ParameterListTextStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle DataTypeTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor ClickableDataTypeColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor PropertyColor = FSlateColor(FColor::FromHex("e59445"));
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor FunctionColor = FSlateColor(FColor::FromHex("5dafc5"));
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor SignalColor = FSlateColor(FColor::FromHex("5bb71d"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor RuntimeFlagColor = FSlateColor(FColor::FromHex("830000"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor VarArgsFlagColor = FSlateColor(FColor::FromHex("bb2828"));
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor VarRetsFlagColor = FSlateColor(FColor::FromHex("bb2828"));
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor ReadOnlyFlagColor = FSlateColor(FColor::FromHex("ffcc00"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor OutFlagColor = FSlateColor(FColor::FromHex("63ff97"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTableRowStyle SearchTreeRowStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTableRowStyle HirachyTreeRowStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTableRowStyle EntryListRowStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FEditableTextBoxStyle SearchInputStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FFINSplitterStyle SplitterStyle;
};

UCLASS(BlueprintType, hidecategories=Object, MinimalAPI)
class UFINReflectionUIStyle : public USlateWidgetStyleContainerBase {
public:
	GENERATED_BODY()

public:
	UPROPERTY(Category=Appearance, EditAnywhere, BlueprintReadWrite, meta=(ShowOnlyInnerProperties))
	FFINReflectionUIStyleStruct ReflectionUIStyle;

	virtual const FSlateWidgetStyle* const GetStyle() const override {
		return &ReflectionUIStyle;
	}
};
