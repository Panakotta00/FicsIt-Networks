#pragma once

#include "CoreMinimal.h"
#include "CoreStyle.h"
#include "FIRTrace.h"
#include "FIVSStyle.h"
#include "SBox.h"
#include "SBreadcrumbTrail.h"
#include "SCompoundWidget.h"
#include "SlateImageBrush.h"
#include "SlateWidgetStyle.h"
#include "SlateWidgetStyleAsset.h"
#include "StrongObjectPtr.h"
#include "FIVSEdObjectSelection.generated.h"

class SFIVSEdTraceSelection;

USTRUCT(BlueprintType)
struct FICSITVISUALSCRIPT_API FFIVSObjectWidgetStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	FFIVSObjectWidgetStyle();
	static const FFIVSObjectWidgetStyle& GetDefault();
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateFontInfo NickFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateFontInfo UUIDFont;
};

USTRUCT(BlueprintType)
struct FICSITVISUALSCRIPT_API FFIVSTraceSelectionWidgetStyle : public FSlateWidgetStyle {
	GENERATED_BODY()

	static const FFIVSTraceSelectionWidgetStyle& GetDefault();
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateFontInfo BreadCrumbFont;
};

class FICSITVISUALSCRIPT_API SFIVSEdObjectWidget : public SCompoundWidget {
public:
	DECLARE_DELEGATE_RetVal_OneParam(TSharedRef<SWidget>, FCreateDetailsWidget, const FFIRTrace&)

	SLATE_BEGIN_ARGS(SFIVSEdObjectWidget) :
		_Style(&FFIVSStyle::Get().GetWidgetStyle<FFIVSObjectWidgetStyle>(TEXT("ObjectWidget"))) {}
	SLATE_STYLE_ARGUMENT(FFIVSObjectWidgetStyle, Style)
	SLATE_EVENT(FCreateDetailsWidget, OnCreateDetailsWidget)
	SLATE_END_ARGS()

private:
	const FFIVSObjectWidgetStyle* Style = nullptr;
	FSlateBrush Brush;

public:
	void Construct(const FArguments& InArgs, const FFIRTrace& Object);
};

class FICSITVISUALSCRIPT_API SFIVSEdObjectSelection : public SCompoundWidget {
public:
	DECLARE_DELEGATE_OneParam(FSelectionChanged, FFIRTrace)
	
	SLATE_BEGIN_ARGS(SFIVSEdObjectSelection) :
		_Style(&FFIVSStyle::Get().GetWidgetStyle<FFIVSObjectWidgetStyle>(TEXT("ObjectWidget"))) {}
	SLATE_STYLE_ARGUMENT(FFIVSObjectWidgetStyle, Style)
	SLATE_EVENT(FSelectionChanged, OnSelectionChanged)
	SLATE_EVENT(SFIVSEdObjectWidget::FCreateDetailsWidget, OnCreateDetailsWidget)
	SLATE_ARGUMENT(FFIRTrace, InitSelection)
	SLATE_END_ARGS()

	const FFIVSObjectWidgetStyle* Style = nullptr;
	FSelectionChanged OnSelectionChanged;
	TArray<FFIRTrace> Objects;
	TSharedPtr<SBox> WidgetHolder;
	FFIRTrace SelectedObject;

	FSlotBase* Slot = nullptr;

public:
	void Construct(const FArguments& InArgs, const TArray<FFIRTrace>& InObjects);

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	void SelectObject(const FFIRTrace& Objects);

	TSharedRef<SWidget> CreateSignalSearch();
};

class FICSITVISUALSCRIPT_API SFIVSEdEditableObjectWidget : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFIVSEdEditableObjectWidget) :
		_Style(&FFIVSStyle::Get().GetWidgetStyle<FFIVSObjectWidgetStyle>(TEXT("ComponentWidget"))) {}
	SLATE_STYLE_ARGUMENT(FFIVSObjectWidgetStyle, Style)
	SLATE_END_ARGS()

private:
	const FFIVSObjectWidgetStyle* Style = nullptr;

public:
	void Construct(const FArguments& InArgs, const FFIRTrace& Object);
};

struct FICSITVISUALSCRIPT_API FFIVSTraceSelectionStep {
	virtual ~FFIVSTraceSelectionStep() = default;
	virtual TSharedRef<SWidget> CreateSelectionWidget(TSharedRef<SFIVSEdTraceSelection> Parent, UObject* Object) const { return SNullWidget::NullWidget; }
	virtual FString CompileSubobject(UObject* Parent, UObject* InSubobject) const { return FString(); }
};

class FICSITVISUALSCRIPT_API SFIVSEdTraceSelection : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFIVSEdTraceSelection) :
		_Style(&FFIVSStyle::Get().GetWidgetStyle<FFIVSTraceSelectionWidgetStyle>(TEXT("TraceSelectionWidget"))) {}
		SLATE_STYLE_ARGUMENT(FFIVSTraceSelectionWidgetStyle, Style)
		SLATE_ARGUMENT(FFIRTrace, InitalPath)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TArray<FFIRTrace>& InRoots);

	void SelectNextObject(UObject* Object);
	void SetPath(TArray<TStrongObjectPtr<UObject>> NewPath);

	TStrongObjectPtr<UFIVSEdTraceSelectionContext> SelectionContext;

	TArray<TStrongObjectPtr<UObject>> Path;

	static void CreatePopup(UObject* WorldContext, const TArray<FFIRTrace>& Objs, const FFIRTrace& Initial, TMulticastDelegate<void(FFIRTrace)> Confirmed);
	static FFIRTrace PathToTrace(const TArray<TStrongObjectPtr<UObject>>& Path) {
		FFIRTrace trace;
		for (auto element : Path) {
			if (trace.IsValidPtr()) {
				trace = trace / element.Get();
			} else {
				trace = FFIRTrace(element.Get());
			}
		}
		return trace;
	}
	static TArray<TStrongObjectPtr<UObject>> TraceToPath(FFIRTrace Trace) {
		TArray<TStrongObjectPtr<UObject>> path;
		while (Trace.IsValidPtr()) {
			path.Add(TStrongObjectPtr<UObject>(Trace.Get()));
			if (!Trace.GetPrev()) break;
			Trace = *Trace.GetPrev();
		}
		Algo::Reverse(path);
		return path;
	}
	static FString CompileTraceToLua(FFIRTrace Trace);

private:
	const FFIVSTraceSelectionWidgetStyle* Style = nullptr;
	TArray<FFIRTrace> Roots;

	TSharedPtr<SBreadcrumbTrail<TArray<TStrongObjectPtr<UObject>>>> BreadcrumbTrail;
	SVerticalBox::FSlot* SelectionSlot = nullptr;

public:
	static TMap<UClass*, TSharedRef<FFIVSTraceSelectionStep>> StepTypes;
};

UCLASS(BlueprintType)
class FICSITVISUALSCRIPT_API UFIVSEdTraceSelectionContext : public UObject {
	GENERATED_BODY()
public:
	TWeakPtr<SFIVSEdTraceSelection> Selection;

	UFUNCTION(BlueprintCallable)
	void SelectNextObject(UObject* Subobject);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFIVSTraceSelectionObjectSelected, UObject*, SelectedObject);

UINTERFACE(BlueprintType)
class FICSITVISUALSCRIPT_API UFIVSTraceSelectionStepInterface : public UInterface {
	GENERATED_BODY()
};

class FICSITVISUALSCRIPT_API IFIVSTraceSelectionStepInterface {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent)
	UWidget* CreateWidget(UFIVSEdTraceSelectionContext* Context);

	UFUNCTION(BlueprintImplementableEvent)
	FString CompileSubobject(UObject* Subobject);
};
