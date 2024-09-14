#pragma once

#include "FIVSScript.h"
#include "FIVSValue.h"
#include "FicsItKernel/FicsItKernel.h"
#include "Script/FIVSNode.h"
#include "Script/FIVSScriptNode.h"
#include "FIVSRuntimeContext.generated.h"

UENUM()
enum EFIVSStackEntryType {
	FIVS_Stack_None,
	FIVS_Stack_PreNode,
	FIVS_Stack_Node,
};

USTRUCT()
struct FFIVSStackEntry {
	GENERATED_BODY()

	EFIVSStackEntryType Type;
	FGuid Node;
	FGuid Pin;
	bool bExec = false;

	FFIVSStackEntry() : Type(FIVS_Stack_None) {}
	FFIVSStackEntry(EFIVSStackEntryType Type, FGuid Node, FGuid Pin, bool bExec) : Type(Type), Node(Node), Pin(Pin), bExec(bExec) {}
};

USTRUCT()
struct FFIVSRuntimeContext {
	GENERATED_BODY()
private:
	UPROPERTY()
	TMap<FGuid, FFIVSValue> PinValues;

	UPROPERTY()
	TSet<FGuid> ExecutedNodes;
	UPROPERTY()
	TSet<FGuid> ExecutedVolatileNodes;

	UPROPERTY()
	UFINKernelSystem* KernelContext = nullptr;

	const FFIVSScript* Script = nullptr;

	TArray<FFIVSStackEntry> Stack;

public:
	FFIVSRuntimeContext() = default;
	FFIVSRuntimeContext(const FFIVSScript& Script, UFINKernelSystem* InKernelContext) : Script(&Script) {
		KernelContext = InKernelContext;
	}
	
	UFINKernelSystem* GetKernelContext() { return KernelContext; }

	void Push_EvaluatePin(FGuid PinSelf) {
		const FGuid* connectedPin = Script->PinConnections.Find(PinSelf);
		if (!connectedPin) return;
		const FGuid* connectedNode = Script->PinToNode.Find(*connectedPin);
		if (!connectedNode) return;
		PushNode(*connectedNode, *connectedPin, false);
	}

	void Push_EvaluatePin(TArrayView<const FGuid> Pins) {
		for (int i = Pins.Num()-1; i >= 0; i--) {
			Push_EvaluatePin(Pins[i]);
		}
	}

	void Push_ExecPin(FGuid PinSelf) {
		const FGuid* connectedPin = Script->PinConnections.Find(PinSelf);
		if (!connectedPin) return;
		const FGuid* connectedNode = Script->PinToNode.Find(*connectedPin);
		if (!connectedNode) return;
		PushNode(*connectedNode, *connectedPin, true);
	}

	void Push_ExecPin(TArrayView<const FGuid> Pins) {
		for (int i = Pins.Num()-1; i >= 0; i--) {
			Push_ExecPin(Pins[i]);
		}
	}

	void PushNode(FGuid Node, FGuid Pin, bool bExec) {
		PushStackEntry(FFIVSStackEntry(FIVS_Stack_PreNode, Node, Pin, bExec));
	}

	void PushStackEntry(const FFIVSStackEntry& Entry) {
		Stack.Push(Entry);
	}

	TOptional<FFIVSStackEntry> PopStackEntry() {
		if (Stack.IsEmpty()) return {};
		return Stack.Pop();
	}
	
	/**
	 * This function tries to find a value that is stored for the pin, like if a pin got evaluated,
	 * this function has to be used to get the value of the evaluation.
	 * If no value is set directly on the pin, it will try to follow the pin connection and get the value of the
	 * connected pin if it is available, otherwise it returns nil.
	 * If no value is set directly and there are no connections to other pins, returns the literal value of the pin.
	 */
	FFIVSValue* GetValue(FGuid Pin) {
		FFIVSValue* Value = PinValues.Find(Pin);
		if (Value) return Value;
		const FGuid* connected = Script->PinConnections.Find(Pin);
		if (connected) {
			Value = PinValues.Find(*connected);
			if (Value) return Value;
		}
		const FFINAnyNetworkValue* NetValue = Script->PinLiterals.Find(Pin);
		if (NetValue) {
			return &PinValues.Add(Pin, FFIVSValue::MakeRValue(*NetValue));
		}
		return nullptr;
	}

	const FFINAnyNetworkValue* TryGetRValue(FGuid Pin) {
		FFIVSValue* value = GetValue(Pin);
		if (!value) return {};
		if (!value->bIsLValue || value->LValuePin == Pin) {
			return &value->RValue;
		} else {
			return TryGetRValue(value->LValuePin);
		}
	}

	FFINAnyNetworkValue* TryGetLValue(FGuid Pin) {
		FFIVSValue* value = GetValue(Pin);
		if (!value) return {};
		if (!value->bIsLValue) return {};
		if (value->LValuePin == Pin) {
			return &value->RValue;
		}
		return TryGetLValue(value->LValuePin);
	}

	/**
	 * This function can be used to define the value that a data-output-pin currently returns
	 */
	void SetValue(FGuid Pin, FFIVSValue&& Value) {
		PinValues.FindOrAdd(Pin, MoveTemp(Value));
	}

	void SetValue(FGuid Pin, const FFINAnyNetworkValue& Value) {
		SetValue(Pin, FFIVSValue::MakeRValue(Value));
	}

	int32 StackNum() {
		return Stack.Num();
	}

	const FFIVSScript* GetScript() { return Script; }

	void NextStep();
};
