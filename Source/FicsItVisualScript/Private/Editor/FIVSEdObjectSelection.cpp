#include "Editor/FIVSEdObjectSelection.h"

#include "AssetRegistryModule.h"
#include "FGBuildable.h"
#include "FicsItReflection.h"
#include "FINNetworkComponent.h"
#include "FINNetworkUtils.h"
#include "FINUMGWidget.h"
#include "FIRGlobalRegisterHelper.h"
#include "SBreadcrumbTrail.h"
#include "Editor/FIVSEdSearchListView.h"

class FAssetRegistryModule;
const FName FFIVSObjectWidgetStyle::TypeName = FName("FIVSComponentWidgetStyle");

FFIVSObjectWidgetStyle::FFIVSObjectWidgetStyle() {
	UUIDFont = FCoreStyle::GetDefaultFontStyle("Regular", FCoreStyle::RegularTextSize);
	NickFont = FCoreStyle::GetDefaultFontStyle("Regular", FCoreStyle::SmallTextSize);
}

const FFIVSObjectWidgetStyle& FFIVSObjectWidgetStyle::GetDefault() {
	static FFIVSObjectWidgetStyle DefaultStyle;
	return DefaultStyle;
}

void FFIVSObjectWidgetStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {}

const FName FFIVSTraceSelectionWidgetStyle::TypeName = FName("FIVSTraceSelectionWidgetStyle");

const FFIVSTraceSelectionWidgetStyle& FFIVSTraceSelectionWidgetStyle::GetDefault() {
	static FFIVSTraceSelectionWidgetStyle DefaultStyle;
	return DefaultStyle;
}

void FFIVSTraceSelectionWidgetStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {}

void SFIVSEdObjectWidget::Construct(const FArguments& InArgs, const FFIRTrace& Object) {
	Style = InArgs._Style;

	if (!Object.IsValidPtr()) {
		ChildSlot[
			SNew(SBox)
			.MinDesiredHeight(32)
			.MaxDesiredHeight(32)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("(none)")))
				.Font(Style->NickFont)
			]
		];
		return;
	}

	UObject* component = UFINNetworkUtils::FindNetworkComponentFromObject(Object.Get());
	UObject* obj = UFINNetworkUtils::RedirectIfPossible(Object).Get();

	UTexture2D* icon = nullptr;
	if (auto buildable = Cast<AFGBuildable>(obj)) {
		icon = UFGItemDescriptor::GetSmallIcon(buildable->GetBuiltWithDescriptor());
	}
	Brush = static_cast<FSlateBrush>(FSlateImageBrush(icon, FVector2D(32, 32)));

	TSharedPtr<SWidget> Details;
	if (InArgs._OnCreateDetailsWidget.IsBound()) {
		Details = InArgs._OnCreateDetailsWidget.Execute(Object);
	} else {
		TSharedPtr<SVerticalBox> VBox = SNew(SVerticalBox);
		if (IsValid(component)) {
			FString nick = IFINNetworkComponent::Execute_GetNick(component);
			if (!nick.IsEmpty()) {
				VBox->AddSlot()[
					SNew(STextBlock)
					.Text(FText::FromString(nick))
					.Font(Style->NickFont)
				];
			}
			VBox->AddSlot()[
				SNew(STextBlock)
				.Text(FText::FromString(IFINNetworkComponent::Execute_GetID(component).ToString()))
				.Font(Style->UUIDFont)
			];
		} else {
			UFIRClass* Class = FFicsItReflectionModule::Get().FindClass(obj->GetClass());
			VBox->AddSlot()[
				SNew(STextBlock)
				.Text(Class->GetDisplayName())
				.Font(Style->NickFont)
			];
		}
		Details = VBox;
	}

	ChildSlot[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot().AutoWidth()[
			SNew(SImage)
			.Image(&Brush)
		]
		+SHorizontalBox::Slot().FillWidth(1)
		.VAlign(VAlign_Center)[
			Details.ToSharedRef()
		]
	];
}

void SFIVSEdObjectSelection::Construct(const FArguments& InArgs, const TArray<FFIRTrace>& InObjects) {
	Style = InArgs._Style;
	OnSelectionChanged = InArgs._OnSelectionChanged;

	Objects = InObjects;
	Objects.Add(FFIRTrace());

	ChildSlot[
		SNew(SBorder)
		.Content()[
			SAssignNew(WidgetHolder, SBox)
		]
	];

	SelectedObject = InArgs._InitSelection;
	WidgetHolder->SetContent(SNew(SFIVSEdObjectWidget, InArgs._InitSelection));
}

FReply SFIVSEdObjectSelection::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	TMulticastDelegate<void(FFIRTrace)> delegate;
	delegate.AddLambda([this](FFIRTrace Trace) {
		SelectObject(Trace);
	});
	SFIVSEdTraceSelection::CreatePopup(GWorld, Objects, SelectedObject, delegate);
	return FReply::Handled();
}

void SFIVSEdObjectSelection::SelectObject(const FFIRTrace& Object) {
	SelectedObject = Object;
	WidgetHolder->SetContent(SNew(SFIVSEdObjectWidget, Object));
	bool _ = OnSelectionChanged.ExecuteIfBound(Object);
}

TSharedRef<SWidget> SFIVSEdObjectSelection::CreateSignalSearch() {
	return SNew(SFIVSEdSearchListView<FFIRTrace>, Objects)
		.OnGetSearchableText_Lambda([](FFIRTrace Trace) -> FString {
			if (!Trace.GetUnderlyingPtr()) return TEXT("None");
			return IFINNetworkComponent::Execute_GetID(*Trace).ToString() + TEXT(" ") + IFINNetworkComponent::Execute_GetNick(*Trace);
		})
		.OnGetElementWidget_Lambda([this](FFIRTrace Trace) -> TSharedRef<SWidget> {
			return SNew(SFIVSEdObjectWidget, Trace);
		})
		.OnCommited_Lambda([this](FFIRTrace Trace) {
			SelectObject(Trace);
			FSlateApplication::Get().DismissAllMenus();
		});
}

TMap<UClass*, TSharedRef<FFIVSTraceSelectionStep>> SFIVSEdTraceSelection::StepTypes;

void SFIVSEdTraceSelection::Construct(const FArguments& InArgs, const TArray<FFIRTrace>& InRoots) {
	Style = InArgs._Style;
	Roots = InRoots;

	ChildSlot[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().AutoHeight()[
			SAssignNew(BreadcrumbTrail, SBreadcrumbTrail<TArray<TStrongObjectPtr<UObject>>>)
			.OnCrumbClicked_Lambda([this](TArray<TStrongObjectPtr<UObject>> InChain) {
				SetPath(InChain);
			})
			.ShowLeadingDelimiter(true)
		]
		+SVerticalBox::Slot()
		.FillHeight(1)
		.Expose(SelectionSlot)
	];

	SelectionContext.Reset(NewObject<UFIVSEdTraceSelectionContext>());
	SelectionContext->Selection = SharedThis(this);

	SetPath(TraceToPath(InArgs._InitalPath));
}

void SFIVSEdTraceSelection::SelectNextObject(UObject* Object) {
	if (!Object) return;
	Path.Add(TStrongObjectPtr<UObject>(Object));
	FText Name = FFicsItReflectionModule::Get().FindClass(Object->GetClass())->GetDisplayName();
	BreadcrumbTrail->PushCrumb(Name, Path);
	TSharedRef<FFIVSTraceSelectionStep>* step = StepTypes.Find(Object->GetClass());
	if (step) {
		auto widget = step->Get().CreateSelectionWidget(SharedThis<SFIVSEdTraceSelection>(this), Object);
		SelectionSlot->AttachWidget(widget);
	} else {
		SelectionSlot->AttachWidget(SNullWidget::NullWidget);
	}
}

void SFIVSEdTraceSelection::SetPath(TArray<TStrongObjectPtr<UObject>> NewPath) {
	Path = {};
	BreadcrumbTrail->ClearCrumbs();
	BreadcrumbTrail->PushCrumb(FText::FromString(TEXT("Self")), Path);
	SelectionSlot->AttachWidget(
		SNew(SFIVSEdSearchListView<FFIRTrace>, Roots)
		.OnGetSearchableText_Lambda([](FFIRTrace Trace) -> FString {
			if (!Trace.Get()) return TEXT("None");
			FString str = IFINNetworkComponent::Execute_GetID(*Trace).ToString().Append(TEXT(" "));
			str.Append(IFINNetworkComponent::Execute_GetNick(*Trace)).Append(TEXT(" "));
			UFIRClass* Class = FFicsItReflectionModule::Get().FindClass(Trace.Get()->GetClass());
			str.Append(Class->GetInternalName()).Append(TEXT(" "));
			str.Append(Class->GetDisplayName().ToString()).Append(TEXT(" "));
			return str;
		})
		.OnGetElementWidget_Lambda([this](FFIRTrace Trace) -> TSharedRef<SWidget> {
			return SNew(SFIVSEdObjectWidget, Trace);
		})
		.OnCommited_Lambda([this](FFIRTrace Trace) {
			SelectNextObject(UFINNetworkUtils::RedirectIfPossible(Trace).Get());
		})
	);
	for (const auto& obj : NewPath) {
		SelectNextObject(obj.Get());
	}
}

void SFIVSEdTraceSelection::CreatePopup(UObject* WorldContext, const TArray<FFIRTrace>& Objs, const FFIRTrace& Initial, TMulticastDelegate<void(FFIRTrace)> Confirmed) {
	UWorld* World = WorldContext->GetWorld();
	UFINSlatePopup* widget = CreateWidget<UFINSlatePopup>(World->GetFirstPlayerController(), UFINSlatePopup::StaticClass());
	TSharedPtr<SFIVSEdTraceSelection> selection;
	widget->SetWrappedWidget(
		SNew(SBox)
		.MinDesiredWidth(300)
		.MinDesiredHeight(300)[
			SAssignNew(selection, SFIVSEdTraceSelection, Objs)
			.InitalPath(Initial)
		]
	);
	widget->OnConfirm.AddLambda([selection, Confirmed]() {
		Confirmed.Broadcast(PathToTrace(selection->Path));
	});
	UFGBlueprintFunctionLibrary::AddPopupWithContent(
		World->GetFirstPlayerController(),
		FText::FromString(TEXT("Select Trace/Object")),
		FText(),
		FPopupClosed(),
		widget
	);
}

FString SFIVSEdTraceSelection::CompileTraceToLua(FFIRTrace Trace) {
	FString str;
	while (Trace.IsValidPtr()) {
		if (Trace.GetPrev() == nullptr) break;

		auto prev = Trace.GetPrev()->Get();
		auto step = StepTypes.Find(prev->GetClass());
		if (step) {
			FString s;
			str = step->Get().CompileSubobject(prev, Trace.Get()) + s;
			str = s + str;
		}

		Trace = *Trace.GetPrev();
	}
	return str;
}

void UFIVSEdTraceSelectionContext::SelectNextObject(UObject* Subobject) {
	if (!Selection.IsValid()) return;
	Selection.Pin()->SelectNextObject(Subobject);
}

struct FFIVSTraceSelectionStepInterface : public FFIVSTraceSelectionStep {
	virtual TSharedRef<SWidget> CreateSelectionWidget(TSharedRef<SFIVSEdTraceSelection> Parent, UObject* Object) const override {
		UWidget* widget = IFIVSTraceSelectionStepInterface::Execute_CreateWidget(Object, Parent->SelectionContext.Get());
		return SNew(SFINUMGWidget, widget);
	}

	virtual FString CompileSubobject(UObject* Parent, UObject* InSubobject) const override {
		return IFIVSTraceSelectionStepInterface::Execute_CompileSubobject(Parent, InSubobject);
	}
};

static FFIRStaticGlobalRegisterFunc FIVS_LoadUObjectTraceSteps([]() {
	for (TObjectIterator<UClass> It; It; ++It) {
		if (It->ImplementsInterface(UFIVSTraceSelectionStepInterface::StaticClass())) {
			auto step = MakeShared<FFIVSTraceSelectionStepInterface>();
			SFIVSEdTraceSelection::StepTypes.Add(*It, step);
		}
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

#if WITH_EDITOR
TArray<FString> PathsToScan;
PathsToScan.Add(TEXT("/FicsItNetworks/"));
AssetRegistry.ScanPathsSynchronous(PathsToScan, true);

TArray<FAssetData> AssetData;
FARFilter Filter;
Filter.bRecursivePaths = true;
Filter.bRecursiveClasses = true;
Filter.PackagePaths.Add("/");
Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
Filter.ClassPaths.Add(UBlueprintGeneratedClass::StaticClass()->GetClassPathName());
Filter.ClassPaths.Add(UClass::StaticClass()->GetClassPathName());
AssetRegistry.GetAssets(Filter, AssetData);

for (const FAssetData& Asset : AssetData) {
	FString Path = Asset.GetObjectPathString();
	if (!Path.EndsWith("_C")) Path += "_C";
	UClass* Class = LoadClass<UObject>(NULL, *Path);
	if (!Class) {
		Class = LoadClass<UObject>(NULL, *Path);
	}
	if (!Class) continue;

	if (Class->ImplementsInterface(UFIVSTraceSelectionStepInterface::StaticClass())) {
		auto step = MakeShared<FFIVSTraceSelectionStepInterface>();
		SFIVSEdTraceSelection::StepTypes.Add(Class, step);
	}
}
#else
	TArray<FTopLevelAssetPath> BaseNames;
	BaseNames.Add(UObject::StaticClass()->GetClassPathName());
	TSet<FTopLevelAssetPath> Excluded;
	TSet<FTopLevelAssetPath> DerivedNames;
	AssetRegistry.GetDerivedClassNames(BaseNames, Excluded, DerivedNames);

	for (const FTopLevelAssetPath& ClassName : DerivedNames) {
		UClass* Class = TSoftClassPtr(FSoftObjectPath(ClassName)).LoadSynchronous();
		if (Class->GetClassFlags() & (CLASS_Abstract | CLASS_Hidden) || Class->GetName().StartsWith("SKEL_")) continue;
		if (Class->ImplementsInterface(UFIVSTraceSelectionStepInterface::StaticClass())) {
			auto step = MakeShared<FFIVSTraceSelectionStepInterface>();
			SFIVSEdTraceSelection::StepTypes.Add(Class, step);
		}
	}
#endif
});