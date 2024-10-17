#pragma once

#include "FIVSNode.h"
#include "Kernel/FIVSScript.h"
#include "FIVSScriptNode.generated.h"

struct FFIVSRuntimeContext;

USTRUCT()
struct FFIVSNodeStatement {
	GENERATED_BODY()

	UPROPERTY()
	FGuid NodeId;

	FFIVSNodeStatement() = default;
	FFIVSNodeStatement(FGuid NodeId) : NodeId(NodeId) {}
	virtual ~FFIVSNodeStatement() = default;

	virtual void PreExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const { }
	virtual void ExecPin(FFIVSRuntimeContext& Context, FGuid ExecPin) const { }
	virtual bool IsVolatile() const { return false; }
};

UCLASS(Abstract)
class UFIVSScriptNode : public UFIVSNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	TArray<UFIVSPin*> Pins;

protected:
	/**
	 * Creates a new pin with the given info, adds it to the pin list and returns the pin.
	 */
	UFIVSPin* CreatePin(EFIVSPinType PinType, const FString& Name, const FText& InDisplayName, FFIVSPinDataType DataType = FFIVSPinDataType(FIR_ANY));
	UFIVSPin* CreateDefaultPin(EFIVSPinType PinType, const FName& Name, const FText& InDisplayName, FFIVSPinDataType DataType = FFIVSPinDataType(FIR_ANY));

	void DeletePin(UFIVSPin* Pin);
	void DeletePins(TArrayView<UFIVSPin*> Pins);

public:
	// Begin UFIVSNode
	virtual TArray<UFIVSPin*> GetNodePins() const override { return Pins; }
	// End UFIVSNode

	virtual TFIRInstancedStruct<FFIVSNodeStatement> CreateNodeStatement() { return TFIRInstancedStruct<FFIVSNodeStatement>(); }
};
