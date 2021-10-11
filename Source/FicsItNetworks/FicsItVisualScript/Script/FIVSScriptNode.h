#pragma once

#include "FIVSNode.h"
#include "FicsItNetworks/FicsItVisualScript/Kernel/FIVSRuntimeContext.h"
#include "FicsItNetworks/Network/FINNetworkUtils.h"
#include "FicsItNetworks/Reflection/FINClass.h"
#include "FicsItNetworks/Reflection/FINReflection.h"
#include "FIVSScriptNode.generated.h"

UCLASS()
class UFIVSScriptNode : public UFIVSNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	TArray<UFIVSPin*> Pins;

protected:
	/**
	 * Creates a new pin with the given info, adds it to the pin list and returns the pin.
	 */
	UFIVSPin* CreatePin(EFIVSPinType PinType, FText Name, EFINNetworkValueType DataType = FIN_ANY);

public:
	/**
	 * Should create all pins of this node
	 */
	virtual void InitPins() {}
	
	/**
	 * Returns all pins of this node.
	 */
	virtual TArray<UFIVSPin*> GetNodePins() const override { return Pins; }
	
	/**
	 * Returns the name of this node.
	 */
	virtual FString GetNodeName() const { return TEXT("Unnamed Node"); }
	
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
	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) { return nullptr; }
};

UCLASS()
class UFIVSNodeBranch : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecTrue = nullptr;
	UPROPERTY()
	UFIVSPin* ExecFalse = nullptr;
	UPROPERTY()
	UFIVSPin* Condition = nullptr;

public:
	// Begin UFIVSGenericNode
	virtual void InitPins() override;

	virtual FString GetNodeName() const override { return "Branch"; }
	
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		return TArray<UFIVSPin*>{ Condition };
	}

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSGenericNode

	static FFIVSNodeSignature GetSignature();
};

UCLASS()
class UFIVSNodePrint : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	UPROPERTY()
	UFIVSPin* MessageIn = nullptr;

public:
	// Begin UFIVSGenericNode
	virtual void InitPins() override;

	virtual FString GetNodeName() const override { return "Print"; }

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSGenericNode

	static FFIVSNodeSignature GetSignature();
};

UCLASS()
class UFIVSNodeTick : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	
public:
	// Begin UFIVSGenericNode
	virtual void InitPins() override;

	virtual FString GetNodeName() const override { return "Event Tick"; }
	
	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;

	static FFIVSNodeSignature GetSignature();
	// End UFIVSScriptNode
};

UCLASS()
class UFIVSNodeCallFunction : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	UPROPERTY()
	UFIVSPin* Self = nullptr;
	UPROPERTY()
	TArray<UFIVSPin*> InputPins;
	UPROPERTY()
	TArray<UFIVSPin*> OutputPins;

	UPROPERTY()
	UFINFunction* Function = nullptr;
	
public:
	// Begin UFIVSScriptNode
	virtual void InitPins() override;

	virtual FString GetNodeName() const override { return Function->GetDisplayName().ToString(); }

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSScriptNode

	void SetFunction(UFINFunction* InFunction) {
		check(ExecIn == nullptr);
		Function = InFunction;
	}

	static FFIVSNodeSignature SignatureFromFunction(UFINFunction* Function);
};

UCLASS()
class UFIVSNodeProxy : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	UPROPERTY()
	UFIVSPin* AddrIn = nullptr;
	UPROPERTY()
	UFIVSPin* CompOut = nullptr;

public:
	// Begin UFIVSGenericNode
	virtual void InitPins() override;

	virtual FString GetNodeName() const override { return "Proxy"; }

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		return TArray<UFIVSPin*>{AddrIn};
	}

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSGenericNode

	static FFIVSNodeSignature GetSignature();
};

UCLASS()
class UFIVSNodeConvert : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* Input = nullptr;
	UPROPERTY()
	UFIVSPin* Output = nullptr;

public:
	EFINNetworkValueType FromType = FIN_NIL;
	EFINNetworkValueType ToType = FIN_NIL;

	// Begin UFIVSGenericNode
	virtual void InitPins() override;

	virtual FString GetNodeName() const override { return TEXT("Convert ") + FINGetNetworkValueTypeName(FromType) + TEXT(" to ") + FINGetNetworkValueTypeName(ToType); }

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSGenericNode

	static FFIVSNodeSignature SignatureFromTypes(EFINNetworkValueType FromType, EFINNetworkValueType ToType);
};

UCLASS()
class UFIVSNodeSetProperty : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	UPROPERTY()
	UFIVSPin* InstanceIn = nullptr;
	UPROPERTY()
	UFIVSPin* DataIn = nullptr;

	UPROPERTY()
	UFINProperty* Property = nullptr;

public:
	void SetProperty(UFINProperty* InProperty) {
		check(ExecIn == nullptr);
		Property = InProperty;
	}

	// Begin UFIVSGenericNode
	virtual void InitPins() override;

	virtual FString GetNodeName() const override { return TEXT("Set ") + Property->GetInternalName(); }

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSGenericNode
};


UCLASS()
class UFIVSNodeGetProperty : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* InstanceIn = nullptr;
	UPROPERTY()
	UFIVSPin* DataOut = nullptr;

	UPROPERTY()
	UFINProperty* Property = nullptr;

public:
	void SetProperty(UFINProperty* InProperty) {
		check(InstanceIn == nullptr);
		Property = InProperty;
	}

	// Begin UFIVSGenericNode
	virtual void InitPins() override;

	virtual FString GetNodeName() const override { return TEXT("Get ") + Property->GetInternalName(); }

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override;
	// End UFIVSGenericNode

	static FFIVSNodeSignature SignatureFromProperty(UFINProperty* Property, bool bWrite);
};