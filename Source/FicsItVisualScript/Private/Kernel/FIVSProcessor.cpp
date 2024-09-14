#include "Kernel/FIVSProcessor.h"

#include "FicsItKernel/Processor/FINStateEEPROMText.h"
#include "Kernel/FIVSCompiler.h"
#include "Network/FINNetworkCircuit.h"
#include "Script/FIVSGraph.h"
#include "Script/Library/FIVSNode_OnTick.h"

void UFIVSProcessor::Tick(float InDeltaTime) {
	if (!RuntimeContext) {
		GetKernel()->Crash(MakeShared<FFINKernelCrash>(TEXT("No Runtime Context found!")));
		return;
	}

	if (RuntimeContext->StackNum() < 1) {
		Reset();
		return;
	}

	RuntimeContext->NextStep();
}

void UFIVSProcessor::Reset() {
	RuntimeContext.Reset();

	if (!IsValid(EEPROM)) {
		GetKernel()->Crash(MakeShared<FFINKernelCrash>(TEXT("No EEPROM Found")));
		return;
	}

	uint32 hash = GetTypeHash(EEPROM->GetCode());
	if (hash != GraphHash) {
		GraphHash = hash;
		TickScript.Reset();

		UFIVSGraph* graph = NewObject<UFIVSGraph>();
		UFIVSSerailizationUtils::FIVS_DeserializeGraph(graph, EEPROM->GetCode(), false);
		TMap<UFIVSScriptNode*, FFIVSScript> scripts = FFIVSCompiler::CompileGraph(graph);
		for (const auto& [node, script] : scripts) {
			if (node->IsA<UFIVSNode_OnTick>()) {
				TickScript = script;
				break;
			}
		}
	}

	if (!TickScript) {
		GetKernel()->Crash(MakeShared<FFINKernelCrash>(TEXT("No OnTick Event found!")));
		return;
	}

	RuntimeContext = MakeShared<FFIVSRuntimeContext>(*TickScript, GetKernel());
	RuntimeContext->PushNode(TickScript->StartNode, FGuid(), true);
}

void UFIVSProcessor::Stop(bool bCond) {
	AFINSignalSubsystem* SigSubSys = AFINSignalSubsystem::GetSignalSubsystem(this);
	SigSubSys->IgnoreAll(this);
}

void UFIVSProcessor::SetEEPROM(AFINStateEEPROM* InEEPROM) {
	EEPROM = Cast<AFINStateEEPROMText>(InEEPROM);
	if (!EEPROM) {
		GetKernel()->Crash(MakeShared<FFINKernelCrash>(TEXT("No EEPROM set!")));
	}
}

void UFIVSProcessor::GetRelevantObjects_Implementation(TArray<FFINNetworkTrace>& OutObjects) {
	UObject* Component = GetKernel()->GetNetwork()->GetComponent().GetObject();
	for (UObject* Object : IFINNetworkCircuitNode::Execute_GetCircuit(Component)->GetComponents()) {
		OutObjects.Add(FFINNetworkTrace(Component) / Object);
	}
}

void UFIVSProcessor::GetRelevantClasses_Implementation(TArray<UFINClass*>& OutClasses) {
	for (const TPair<UClass*, UFINClass*>& Class : FFINReflection::Get()->GetClasses()) {
		OutClasses.Add(Class.Value);
	}
}

void UFIVSProcessor::GetRelevantStructs_Implementation(TArray<UFINStruct*>& OutStructs) {
	for (const TPair<UScriptStruct*, UFINStruct*>& Struct : FFINReflection::Get()->GetStructs()) {
		OutStructs.Add(Struct.Value);
	}
}
