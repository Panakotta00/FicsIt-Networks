#pragma once

#include "FIVSNode.h"
#include "Kernel/FIVSRuntimeContext.h"
#include "Reflection/FINClass.h"
#include "FIVSScriptNode.generated.h"

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
	UFIVSPin* CreatePin(EFIVSPinType PinType, const FString& Name, const FText& InDisplayName, FFIVSPinDataType DataType = FFIVSPinDataType(FIN_ANY));
	UFIVSPin* CreateDefaultPin(EFIVSPinType PinType, const FName& Name, const FText& InDisplayName, FFIVSPinDataType DataType = FFIVSPinDataType(FIN_ANY));

	void DeletePin(UFIVSPin* Pin);
	void DeletePins(TArrayView<UFIVSPin*> Pins);

public:
	// Begin UFIVSNode
	virtual TArray<UFIVSPin*> GetNodePins() const override { return Pins; }
	// End UFIVSNode
	
	/**
	 * This function will be called ahead of the main node execution code and is mainly intended to decide
	 * ahead of time, which data-input-pins have to get evaluated for the nodes proper function.
	 * This has to be done since evaluating input-pins may cause further FIVS-Code to be run, which is
	 * unsafe in regards to runtime speed, meaning we have to have the ability to halt execution while evaluating
	 * input pins of nodes.
	 * @param[in]	ExecPin		The execution pin that caused this node to be called. May be null on pure nodes.
	 * @param[in]	Context		The runtime context for this script, which also includes temporary variables for this node.
	 * @returns The data-input-pins which have to get evaluated ahead of actual node-execution
	 */
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) { return TArray<UFIVSPin*>(); }

	/**
	 * This function will be called after the PreExecPin function and the evaluation of all data-input-pins which have
	 * been defined to be necesery. This function will handle actual node execution and logic. It also decides
	 * which execution output pin will continue the script execution.
	 * @param[in]	ExecPin		The execution pin that caused this node to be called. May be null on pure nodes.
	 * @param[out]	Context		The runtime context for this script, which also includes temporary variables for this node.
	 * @returns The execution-pin which will continue the script execution. May be null if script-sequence should stop or node has no exec-output-pins. (like pure nodes)
	 */
	virtual TArray<UFIVSPin*> ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) { return {}; }
};
