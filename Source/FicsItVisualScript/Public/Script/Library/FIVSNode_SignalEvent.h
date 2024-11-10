#pragma once

#include "CoreMinimal.h"
#include "FIVSCompileLua.h"
#include "FIVSNode_Event.h"
#include "FIVSNode_SignalEvent.generated.h"

UCLASS()
class UFIVSNode_SignalEvent : public UFIVSNode_Event, public IFIVSCompileLuaInterface {
	GENERATED_BODY()

	UPROPERTY()
	UFIRSignal* Signal;

	UPROPERTY()
	UFIVSPin* ExecOut;

	UPROPERTY()
	UFIVSPin* SenderOut;

	UPROPERTY()
	TArray<UFIVSPin*> Parameters;

	UPROPERTY()
	FFIRTrace Sender;

public:
	UFIVSNode_SignalEvent();

	// Begin UFIVSNode
	virtual void GetNodeActions(TArray<FFIVSNodeAction>& Actions) const override;
	virtual void SerializeNodeProperties(const TSharedRef<FJsonObject>& Properties) const override;
	virtual void DeserializeNodeProperties(const TSharedPtr<FJsonObject>& Properties) override;
	virtual TSharedPtr<SWidget> CreateDetailsWidget(TScriptInterface<IFIVSScriptContext_Interface> Context) override;
	// End UFIVSNode

	// Begin IFIVSCompileLuaInterface
	virtual bool IsLuaRootNode() const override { return true; }
	virtual void CompileNodeToLua(FFIVSLuaCompilerContext& Context) const override;
	// End IFVISCompileLuaInterface

	void SetSignal(UFIRSignal* InSignal);
};
