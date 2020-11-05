#pragma once

#include "SlateBrush.h"
#include "SlateWidgetStyle.h"
#include "SlateWidgetStyleContainerBase.h"
#include "FINReflectionUIStyle.generated.h"

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
};

UCLASS(BlueprintType, hidecategories=Object, MinimalAPI)
class UFINReflectionUIStyle : public USlateWidgetStyleContainerBase
{
public:
	GENERATED_BODY()

public:
	/** The actual data describing the button's appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, BlueprintReadWrite, meta=( ShowOnlyInnerProperties ))
	FFINReflectionUIStyleStruct ReflectionUIStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast< const struct FSlateWidgetStyle* >( &ReflectionUIStyle );
	}

public:
	MODDING_SHIPPING_FORCEINLINE ~UFINReflectionUIStyle() = default;
};
