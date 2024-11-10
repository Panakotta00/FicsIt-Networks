#include "Kernel/FIVSProcessor.h"

#include "FGInventoryComponent.h"
#include "FicsItReflection.h"
#include "FINItemStateEEPROMText.h"
#include "FINNetworkCircuit.h"
#include "FINNetworkCircuitNode.h"
#include "FINSignalSubsystem.h"
#include "NetworkController.h"
#include "Kernel/FIVSCompiler.h"
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

	auto state = GetKernel()->GetEEPROM().GetItemState();
	const FFINItemStateEEPROMText* eeprom = state.GetValuePtr<FFINItemStateEEPROMText>();

	if (!eeprom) {
		GetKernel()->Crash(MakeShared<FFINKernelCrash>(TEXT("No EEPROM Found")));
		return;
	}

	uint32 hash = GetTypeHash(eeprom->Code);
	if (hash != GraphHash) {
		GraphHash = hash;
		TickScript.Reset();

		UFIVSGraph* graph = NewObject<UFIVSGraph>();
		UFIVSSerailizationUtils::FIVS_DeserializeGraph(graph, eeprom->Code, false);
		TMap<UFIVSScriptNode*, FFIVSScript> scripts = FFIVSCompiler::CompileGraph(graph);
		for (const auto& [node, script] : scripts) {
			if (node->IsA<UFIVSNode_OnTick>()) {
				TickScript = script;
				break;
			}
		}

		FFIVSLuaCompilerContext luaContext;
		for (UFIVSNode* node : graph->GetNodes()) {
			if (!node->Implements<UFIVSCompileLuaInterface>()) continue;
			if (!Cast<IFIVSCompileLuaInterface>(node)->IsLuaRootNode()) continue;
			Cast<IFIVSCompileLuaInterface>(node)->CompileNodeToLua(luaContext);
		}

		UE_LOG(LogFicsItVisualScript, Warning, TEXT("Compiled Lua Code:\n%s"), *FString(luaContext.FinalizeCode()))
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

void UFIVSProcessor::GetRelevantObjects_Implementation(TArray<FFIRTrace>& OutObjects) {
	UObject* Component = GetKernel()->GetNetwork()->GetComponent().GetObject();
	for (UObject* Object : IFINNetworkCircuitNode::Execute_GetCircuit(Component)->GetComponents()) {
		OutObjects.Add(FFIRTrace(Component) / Object);
	}
}

void UFIVSProcessor::GetRelevantClasses_Implementation(TArray<UFIRClass*>& OutClasses) {
	for (const TPair<UClass*, UFIRClass*>& Class : FFicsItReflectionModule::Get().GetClasses()) {
		OutClasses.Add(Class.Value);
	}
}

void UFIVSProcessor::GetRelevantStructs_Implementation(TArray<UFIRStruct*>& OutStructs) {
	for (const TPair<UScriptStruct*, UFIRStruct*>& Struct : FFicsItReflectionModule::Get().GetStructs()) {
		OutStructs.Add(Struct.Value);
	}
}
