#pragma once

#include "CoreMinimal.h"
#include "FGPopupWidgetContent.h"
#include "ReflectionHelper.h"
#include "SCompoundWidget.h"
#include "StrongObjectPtr.h"
#include "Widget.h"
#include "FINUMGWidget.generated.h"

class SFINUMGWidget : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFINUMGWidget) {}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, UWidget* WidgetToWrap) {
		Widget.Reset(WidgetToWrap);
		ChildSlot[
			WidgetToWrap->TakeWidget()
		];
	}

private:
	TStrongObjectPtr<UWidget> Widget;
};

UCLASS()
class FICSITNETWORKSMISC_API UFINSlateWrapperWidget : public UUserWidget {
	GENERATED_BODY()
public:
	TSharedPtr<SWidget> SlateWidgetToWrap;

	void SetWrappedWidget(TSharedRef<SWidget> InSlateWidget) {
		SlateWidgetToWrap = InSlateWidget;
		Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
	}

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override {
		if (SlateWidgetToWrap.IsValid()) {
			return SlateWidgetToWrap.ToSharedRef();
		}
		return SNullWidget::NullWidget;
	}
};

UCLASS()
class FICSITNETWORKSMISC_API UFINSlatePopup : public UFGPopupWidgetContent {
	GENERATED_BODY()
public:
	TMulticastDelegate<void()> OnConfirm;

	UFINSlatePopup() {
		UFunction* func = StaticClass()->FindFunctionByName("GetShouldOkayBeEnabled");
		func->SetNativeFunc(&ShouldOkayBeEnabled_Internal);
		func->FunctionFlags |= FUNC_Native;

		func = StaticClass()->FindFunctionByName("NotifyPopupConfirmed");
		func->SetNativeFunc(&NotifyPopupConfirmed_Internal);
		func->FunctionFlags |= FUNC_Native;

		bIsPopup = true;
	}

	static void ShouldOkayBeEnabled_Internal(UObject* Context, FFrame& Stack, RESULT_DECL) {
		*(bool*)RESULT_PARAM = true;
		P_FINISH;
	}

	static DEFINE_FUNCTION(NotifyPopupConfirmed_Internal) {
		Cast<UFINSlatePopup>(Context)->OnConfirm.Broadcast();
		P_FINISH;
	}

	void SetWrappedWidget(TSharedRef<SWidget> InSlateWidget) {
		SlateWidgetToWrap = InSlateWidget;
		Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
	}

protected:
	TSharedPtr<SWidget> SlateWidgetToWrap;

	virtual TSharedRef<SWidget> RebuildWidget() override {
		if (SlateWidgetToWrap.IsValid()) {
			return SlateWidgetToWrap.ToSharedRef();
		}
		return SNullWidget::NullWidget;
	}
};
