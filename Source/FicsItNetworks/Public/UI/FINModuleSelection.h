#pragma once

#include "CoreMinimal.h"
#include "FicsItReflection.h"
#include "FINModuleSystemPanel.h"
#include "FIVSEdObjectSelection.h"
#include "FIVSEdSearchListView.h"
#include "Widget.h"
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
				return SNew(SFIVSEdObjectWidget, Trace);
			})
			.OnCommited_Lambda([this](FFIRTrace Trace) {
				SelectionContext->SelectNextObject(Trace.Get());
			});
	}
	// End UWidget
};
