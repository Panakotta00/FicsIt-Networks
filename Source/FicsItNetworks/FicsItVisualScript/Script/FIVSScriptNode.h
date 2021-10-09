#pragma once

#include "FIVSNode.h"
#include "FicsItNetworks/FicsItVisualScript/Kernel/FIVSRuntimeContext.h"
#include "FicsItNetworks/Network/FINNetworkUtils.h"
#include "FicsItNetworks/Reflection/FINClass.h"
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
	UFIVSPin* CreatePin(EFIVSPinType PinType, FText Name, EFINNetworkValueType DataType = FIN_ANY) {
		UFIVSGenericPin* Pin = NewObject<UFIVSGenericPin>();
		Pin->ParentNode = this;
		Pin->PinType = PinType;
		Pin->Name = Name;
		Pin->PinDataType = DataType;
		
		Pins.Add(Pin);
		return Pin;
	}
	
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
	virtual void InitPins() override {
		ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString("Exec"));
		ExecTrue = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("True"));
		ExecFalse = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("False"));
		Condition = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Condition"), FIN_BOOL);
	}
	
	virtual FString GetNodeName() const override { return "Branch"; }
	
	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		return TArray<UFIVSPin*>{ Condition };
	}

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		bool bCondition = Context.GetValue(Condition).GetBool();
		return bCondition ? ExecTrue : ExecFalse;
	}
	// End UFIVSGenericNode
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
	virtual void InitPins() override {
		ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString("Exec"));
		ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("Out"));
		MessageIn = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Message"), FIN_STR);
	}

	virtual FString GetNodeName() const override { return "Print"; }

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		return TArray<UFIVSPin*>{MessageIn};
	}

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		FString Message = Context.GetValue(MessageIn).GetString();
		CodersFileSystem::SRef<CodersFileSystem::FileStream> serial = Context.GetKernelContext()->GetDevDevice()->getSerial()->open(CodersFileSystem::OUTPUT);
		if (serial) {
			*serial << TCHAR_TO_UTF8(*Message) << "\r\n";
			serial->close();
		}
		return ExecOut;
	}
	// End UFIVSGenericNode
};

UCLASS()
class UFIVSNodeTick : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	
public:
	// Begin UFIVSGenericNode
	virtual void InitPins() override {
		ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("Run"));
	}
	
	virtual FString GetNodeName() const override { return "Event Tick"; }
	
	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		return ExecOut;
	}
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
	virtual void InitPins() override {
		ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString(TEXT("Exec")));
		ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString(TEXT("Run")));
		Self = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString(TEXT("Self")), FIN_TRACE); // TODO: Add trace-type differentiation etc (general thing that has to be added to graph system)
		for (UFINProperty* Param : Function->GetParameters()) {
			EFINRepPropertyFlags Flags = Param->GetPropertyFlags();
			if (Flags & FIN_Prop_Param) {
				EFINNetworkValueType Type = Param->GetType();
				if (Type == FIN_OBJ) Type = FIN_TRACE;
				if (Flags & FIN_Prop_OutParam) {
					OutputPins.Add(CreatePin(FIVS_PIN_DATA_OUTPUT, Param->GetDisplayName(), Type));
				} else {
					InputPins.Add(CreatePin(FIVS_PIN_DATA_INPUT, Param->GetDisplayName(), Type));
				}
			}
		}
	}

	virtual FString GetNodeName() const override { return Function->GetDisplayName().ToString(); }

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		TArray<UFIVSPin*> EvalPins = InputPins;
		EvalPins.Add(Self);
		return EvalPins;
	}
	
	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		TArray<FFINAnyNetworkValue> InputValues;
		FFINExecutionContext ExecContext( UFINNetworkUtils::RedirectIfPossible(Context.GetValue(Self).GetTrace()));
		for (UFIVSPin* InputPin : InputPins) {
			InputValues.Add(Context.GetValue(InputPin));
		}
		TArray<FFINAnyNetworkValue> OutputValues = Function->Execute(ExecContext, InputValues);
		for (int i = 0; i < FMath::Min(OutputValues.Num(), OutputPins.Num()); ++i) {
			Context.SetValue(OutputPins[i], OutputValues[i]);
		}
		return ExecOut;
	}
	// End UFIVSScriptNode

	void SetFunction(UFINFunction* InFunction) {
		check(ExecIn == nullptr);
		Function = InFunction;
	}
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
	virtual void InitPins() override {
		ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString("Exec"));
		ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("Out"));
		AddrIn = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Address"), FIN_STR);
		CompOut = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString("Component"), FIN_TRACE);
	}

	virtual FString GetNodeName() const override { return "Proxy"; }

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		return TArray<UFIVSPin*>{AddrIn};
	}

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		FString Addr = Context.GetValue(AddrIn).GetString();
		FGuid Guid;
		if (!FGuid::Parse(Addr, Guid)) {
			Context.GetKernelContext()->Crash(MakeShared<FFINKernelCrash>(TEXT("Address not valid!")));
			return nullptr;
		}
		FFINNetworkTrace Component = Context.GetKernelContext()->GetNetwork()->GetComponentByID(Guid);
		if (!Component.IsValid()) {
			Context.GetKernelContext()->Crash(MakeShared<FFINKernelCrash>(TEXT("Component not found!")));
		}
		Context.SetValue(CompOut, Component);
		return ExecOut;
	}
	// End UFIVSGenericNode
};

UCLASS()
class UFIVSNodeConvert : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	UPROPERTY()
	UFIVSPin* Input = nullptr;
	UPROPERTY()
	UFIVSPin* Output = nullptr;

public:
	EFINNetworkValueType FromType = FIN_NIL;
	EFINNetworkValueType ToType = FIN_NIL;

	// Begin UFIVSGenericNode
	virtual void InitPins() override {
		ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString("Exec"));
		ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("Out"));
		Input = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Input"), FromType);
		Output = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString("Output"), ToType);
	}

	virtual FString GetNodeName() const override { return TEXT("Convert ") + FINGetNetworkValueTypeName(FromType) + TEXT(" to ") + FINGetNetworkValueTypeName(ToType); }

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		return TArray<UFIVSPin*>{Input};
	}

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		FFINAnyNetworkValue InputVal = Context.GetValue(Input);
		Context.SetValue(Output, FINCastNetworkValue(InputVal, ToType));
		return ExecOut;
	}
	// End UFIVSGenericNode
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
	virtual void InitPins() override {
		ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString("Exec"));
		ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("Out"));
		InstanceIn = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Instance"), FIN_TRACE);
		EFINNetworkValueType Type = Property->GetType();
		if (Type == FIN_OBJ) Type = FIN_TRACE;
		DataIn = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Value"), Type);
	}

	virtual FString GetNodeName() const override { return TEXT("Set ") + Property->GetInternalName(); }

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		return TArray<UFIVSPin*>{InstanceIn, DataIn};
	}

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		FFINNetworkTrace Instance = Context.GetValue(InstanceIn).GetTrace();
		FFINAnyNetworkValue Data = Context.GetValue(DataIn);
		FFINExecutionContext ExecContext(UFINNetworkUtils::RedirectIfPossible(Instance));
		Property->SetValue(ExecContext, Data);
		return ExecOut;
	}
	// End UFIVSGenericNode
};


UCLASS()
class UFIVSNodeGetProperty : public UFIVSScriptNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	UFIVSPin* ExecIn = nullptr;
	UPROPERTY()
	UFIVSPin* ExecOut = nullptr;
	UPROPERTY()
	UFIVSPin* InstanceIn = nullptr;
	UPROPERTY()
	UFIVSPin* DataOut = nullptr;

	UPROPERTY()
	UFINProperty* Property = nullptr;

public:
	void SetProperty(UFINProperty* InProperty) {
		check(ExecIn == nullptr);
		Property = InProperty;
	}

	// Begin UFIVSGenericNode
	virtual void InitPins() override {
		ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString("Exec"));
		ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("Out"));
		InstanceIn = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Instance"), FIN_TRACE);
		EFINNetworkValueType Type = Property->GetType();
		if (Type == FIN_OBJ) Type = FIN_TRACE;
		DataOut = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString("Value"), Type);
	}

	virtual FString GetNodeName() const override { return TEXT("Get ") + Property->GetInternalName(); }

	virtual TArray<UFIVSPin*> PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		return TArray<UFIVSPin*>{InstanceIn};
	}

	virtual UFIVSPin* ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) override {
		FFINNetworkTrace Instance = Context.GetValue(InstanceIn).GetTrace();
		FFINExecutionContext ExecContext(UFINNetworkUtils::RedirectIfPossible(Instance));
		FFINAnyNetworkValue Value = Property->GetValue(ExecContext);
		Context.SetValue(DataOut, Value);
		return ExecOut;
	}
	// End UFIVSGenericNode
};