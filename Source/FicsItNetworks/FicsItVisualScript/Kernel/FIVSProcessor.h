#pragma once

#include "FIVSRuntimeContext.h"
#include "FicsItNetworks/FicsItKernel/Processor/Processor.h"
#include "FicsItNetworks/FicsItVisualScript/FIVSStateEEPROM.h"
#include "FicsItNetworks/FicsItVisualScript/Script/FIVSScriptNode.h"
#include "FIVSProcessor.generated.h"

UENUM()
enum EFIVSMicroStep {
	FIVS_STEP_PRENODE,
	FIVS_STEP_EVAL,
	FIVS_STEP_NODE,
	FIVS_STEP_NONE,
};

USTRUCT()
struct FFIVSMicroStep {
	GENERATED_BODY()
	
	EFIVSMicroStep StepType;
	UObject* NodeOrPin = nullptr;
	UObject* Callee = nullptr;

	FFIVSMicroStep() : StepType(FIVS_STEP_NONE) {}
	FFIVSMicroStep(EFIVSMicroStep InStepType, UObject* InNodeOrPin, UObject* InCallee) : StepType(InStepType), NodeOrPin(InNodeOrPin), Callee(InCallee) {}
};

UCLASS()
class UFIVSProcessor : public UFINKernelProcessor {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	UFIVSGraph* Graph = nullptr;

	TSharedPtr<FFIVSRuntimeContext> RuntimeContext;
	TArray<FFIVSMicroStep> MicroSteps;
	
public:
	// Begin UFINKernelProcessor
	virtual void Tick(float InDeltaTime) override {
		if (MicroSteps.Num() < 1) {
			Reset();
			return;
		}

		NextStep();
	}

	void NextStep() {
		FFIVSMicroStep Step = MicroSteps.Pop();
		switch (Step.StepType) {
		case FIVS_STEP_PRENODE: {
			TArray<UFIVSPin*> ToEvaluate = Cast<UFIVSScriptNode>(Step.NodeOrPin)->PreExecPin(Cast<UFIVSPin>(Step.Callee), *RuntimeContext.Get());
			MicroSteps.Push(FFIVSMicroStep(FIVS_STEP_NODE, Step.NodeOrPin, Step.Callee));
			for (UFIVSPin* Eval : ToEvaluate) {
				if (Eval->GetConnections().Num() > 0) MicroSteps.Push(FFIVSMicroStep(FIVS_STEP_EVAL, Eval, nullptr));
			}
			break;
		} case FIVS_STEP_EVAL: {
			UFIVSPin* Connected = Cast<UFIVSPin>(Step.NodeOrPin)->FindConnected();
			check(Connected); // TODO: FIVS Exception Handling
			if (Connected->ParentNode->IsPure()) {
				MicroSteps.Push(FFIVSMicroStep(FIVS_STEP_PRENODE, Connected->ParentNode, nullptr));
			}
			break;
		} case FIVS_STEP_NODE: {
			UFIVSPin* OutPin = Cast<UFIVSScriptNode>(Step.NodeOrPin)->ExecPin(Cast<UFIVSPin>(Step.Callee), *RuntimeContext.Get());
			if (OutPin) {
				UFIVSPin* NextPin = OutPin->FindConnected();
				if (NextPin) MicroSteps.Push(FFIVSMicroStep(FIVS_STEP_PRENODE, NextPin->ParentNode, nullptr));
			}
			break;
		} default: ;
		}
	}

	virtual void Reset() override {
		RuntimeContext = MakeShared<FFIVSRuntimeContext>(Graph, GetKernel());
		MicroSteps.Empty();
		for (UFIVSNode* Node : RuntimeContext->GetScript()->GetNodes()) {
			if (Node->IsA<UFIVSNodeTick>()) {
				MicroSteps.Add(FFIVSMicroStep(FIVS_STEP_PRENODE, Node, nullptr));
				break;
			}
		}
	}

	virtual void SetEEPROM(AFINStateEEPROM* InEEPROM) override {
		AFIVSStateEEPROM* FIVSEEPROM = Cast<AFIVSStateEEPROM>(InEEPROM);
		if (!FIVSEEPROM) {
			GetKernel()->Crash(MakeShared<FFINKernelCrash>(TEXT("No FIVS EEPROM set!")));
			return;
		}
		
		Graph = FIVSEEPROM->Graph;
	}
	// End UFINKernelProcessor
};
