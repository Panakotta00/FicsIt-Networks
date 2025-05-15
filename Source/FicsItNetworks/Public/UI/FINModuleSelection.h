#pragma once

#include "CoreMinimal.h"
#include "FicsItReflection.h"
#include "FINModuleSystemPanel.h"
#include "FIVSEdObjectSelection.h"
#include "FIVSEdSearchListView.h"
#include "Widget.h"
#include "../../../FicsItNetworksMisc/Public/ModuleSystem/FINModuleSystemModule.h"
#include "FINModuleSelection.generated.h"

UCLASS(BlueprintType)
class FICSITNETWORKS_API UFINModuleSelection : public UWidget {
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn=true))
	UFINModuleSystemPanel* ModulePanel = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn=true))
	UFIVSEdTraceSelectionContext* SelectionContext = nullptr;

	// Begin UWidget
	virtual TSharedRef<SWidget> RebuildWidget() override {
		TArray<FFIRTrace> Roots;
		TArray<AActor*> Modules;
		ModulePanel->GetModules(Modules);
		for (auto module : Modules) {
			Roots.Add(FFIRTrace(module));
		}

		return SNew(SFIVSEdSearchListView<FFIRTrace>, Roots)
			.OnGetSearchableText_Lambda([](FFIRTrace Trace) -> FString {
				if (!Trace.GetUnderlyingPtr()) return TEXT("None");
				return FFicsItReflectionModule::Get().FindClass(Trace.Get()->GetClass())->GetDisplayName().ToString();
			})
			.OnGetElementWidget_Lambda([this](FFIRTrace Trace) -> TSharedRef<SWidget> {
				return SNew(SFIVSEdObjectWidget, Trace)
					.OnCreateDetailsWidget_Lambda([this](FFIRTrace Trace) {
						int x, y, rot;
						IFINModuleSystemModule::Execute_getModulePos(Trace.Get(), x, y, rot);
						const auto& style = FFIVSStyle::Get().GetWidgetStyle<FFIVSObjectWidgetStyle>(TEXT("ObjectWidget"));
						return SNew(SVerticalBox)
							+SVerticalBox::Slot().AutoHeight()[
								SNew(STextBlock)
								.Text(FFicsItReflectionModule::Get().FindClass(Trace.Get()->GetClass())->GetDisplayName())
								.Font(style.NickFont)
							]
							+SVerticalBox::Slot().AutoHeight()[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()[
									SNew(STextBlock)
									.Text(FText::FromString(FString::Printf(TEXT("X: %d"), y)))
									.Font(style.UUIDFont)
								]
								+SHorizontalBox::Slot()[
									SNew(STextBlock)
									.Text(FText::FromString(FString::Printf(TEXT("Y: %d"), x)))
									.Font(style.UUIDFont)
								]
							];
					});
			})
			.OnCommited_Lambda([this](FFIRTrace Trace) {
				SelectionContext->SelectNextObject(Trace.Get());
			});
	}
	// End UWidget
};
