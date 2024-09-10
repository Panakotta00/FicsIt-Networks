#pragma once

#include "CoreMinimal.h"
#include "FIVSNode_Event.h"
#include "FIVSNode_SignalEvent.generated.h"

UCLASS()
class UFIVSNode_SignalEvent : public UFIVSNode_Event {
	GENERATED_BODY()

	UPROPERTY()
	UFINSignal* Signal;

	UPROPERTY()
	UFIVSPin* ExecOut;

	UPROPERTY()
	UFIVSPin* SenderOut;

	UPROPERTY()
	FFINNetworkTrace Sender;

public:
	// Begin UFIVSNode
	virtual void InitPins() override;
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(FFIVSNodeProperties& Properties) const override;
	virtual void DeserializeNodeProperties(const FFIVSNodeProperties& Properties) override;
	virtual FString GetNodeName() const override;
	virtual TSharedPtr<SWidget> CreateDetailsWidget(TScriptInterface<IFIVSScriptContext_Interface> Context) override;
	// End UFIVSNode

	// Begin UFIVSScriptNode
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSScriptNode

	UFINSignal* GetSignal() { return Signal; }
	const FFINNetworkTrace& GetSender() { return Sender; }
};
