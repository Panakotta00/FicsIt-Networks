#include "FIVSScriptNode.h"

UFIVSPin* UFIVSScriptNode::CreatePin(EFIVSPinType PinType, FText Name, EFINNetworkValueType DataType) {
	UFIVSGenericPin* Pin = NewObject<UFIVSGenericPin>();
	Pin->ParentNode = this;
	Pin->PinType = PinType;
	Pin->Name = Name;
	Pin->PinDataType = DataType;
		
	Pins.Add(Pin);
	return Pin;
}

void UFIVSNodeBranch::InitPins() {
	ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString("Exec"));
	ExecTrue = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("True"));
	ExecFalse = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("False"));
	Condition = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Condition"), FIN_BOOL);
}

UFIVSPin* UFIVSNodeBranch::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	bool bCondition = Context.GetValue(Condition).GetBool();
	return bCondition ? ExecTrue : ExecFalse;
}

FFIVSNodeSignature UFIVSNodeBranch::GetSignature() {
	return FFIVSNodeSignature{{FIVS_PIN_EXEC_INPUT, {FIVS_PIN_DATA_INPUT, FIN_BOOL}, FIVS_PIN_EXEC_OUTPUT, FIVS_PIN_EXEC_OUTPUT}, FText::FromString(TEXT("Branch"))};
}

void UFIVSNodePrint::InitPins() {
	ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString("Exec"));
	ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("Out"));
	MessageIn = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Message"), FIN_STR);
}

TArray<UFIVSPin*> UFIVSNodePrint::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return TArray<UFIVSPin*>{MessageIn};
}

UFIVSPin* UFIVSNodePrint::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	FString Message = Context.GetValue(MessageIn).GetString();
	CodersFileSystem::SRef<CodersFileSystem::FileStream> serial = Context.GetKernelContext()->GetDevDevice()->getSerial()->open(CodersFileSystem::OUTPUT);
	if (serial) {
		*serial << TCHAR_TO_UTF8(*Message) << "\r\n";
		serial->close();
	}
	return ExecOut;
}

FFIVSNodeSignature UFIVSNodePrint::GetSignature() {
	return FFIVSNodeSignature{{FIVS_PIN_EXEC_INPUT,  {FIVS_PIN_DATA_INPUT, FIN_STR}, FIVS_PIN_EXEC_OUTPUT}, FText::FromString(TEXT("Print"))};
}

void UFIVSNodeTick::InitPins() {
	ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("Run"));
}

UFIVSPin* UFIVSNodeTick::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return ExecOut;
}

FFIVSNodeSignature UFIVSNodeTick::GetSignature() {
	return FFIVSNodeSignature{{FIVS_PIN_EXEC_OUTPUT}, FText::FromString(TEXT("On Tick"))};
}

void UFIVSNodeCallFunction::InitPins() {
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

TArray<UFIVSPin*> UFIVSNodeCallFunction::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	TArray<UFIVSPin*> EvalPins = InputPins;
	EvalPins.Add(Self);
	return EvalPins;
}

UFIVSPin* UFIVSNodeCallFunction::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
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

FFIVSNodeSignature UFIVSNodeCallFunction::SignatureFromFunction(UFINFunction* Function) {
	FFIVSNodeSignature Signature;
	Signature.Name = Function->GetDisplayName();
	Signature.Description = Function->GetDescription();
	Signature.Pins.Add(FIVS_PIN_EXEC_INPUT);
	Signature.Pins.Add(FIVS_PIN_EXEC_OUTPUT);
	Signature.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFINExpandedNetworkValueType(FIN_TRACE, Cast<UFINClass>(Function->GetOuter()))));
	for (UFINProperty* Param : Function->GetParameters()) {
		Signature.Pins.Add(FFIVSFullPinType(Param));
	}
	return Signature;
}

void UFIVSNodeProxy::InitPins() {
	ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString("Exec"));
	ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("Out"));
	AddrIn = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Address"), FIN_STR);
	CompOut = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString("Component"), FIN_TRACE);
}

UFIVSPin* UFIVSNodeProxy::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
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

FFIVSNodeSignature UFIVSNodeProxy::GetSignature() {
	return FFIVSNodeSignature{{FIVS_PIN_EXEC_INPUT, {FIVS_PIN_DATA_INPUT, FIN_STR}, FIVS_PIN_EXEC_OUTPUT, {FIVS_PIN_DATA_OUTPUT, {FIN_TRACE, FFINReflection::Get()->FindClass(UObject::StaticClass())}}}, FText::FromString(TEXT("Proxy"))};
}

void UFIVSNodeConvert::InitPins() {
	Input = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Input"), FromType);
	Output = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString("Output"), ToType);
}

TArray<UFIVSPin*> UFIVSNodeConvert::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return TArray<UFIVSPin*>{Input};
}

UFIVSPin* UFIVSNodeConvert::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	FFINAnyNetworkValue InputVal = Context.GetValue(Input);
	Context.SetValue(Output, FINCastNetworkValue(InputVal, ToType));
	return nullptr;
}

FFIVSNodeSignature UFIVSNodeConvert::SignatureFromTypes(EFINNetworkValueType FromType, EFINNetworkValueType ToType) {
	FFIVSNodeSignature Signature;
	Signature.Name = FText::FromString(TEXT("Convert ") + FINGetNetworkValueTypeName(FromType) + TEXT(" to ") + FINGetNetworkValueTypeName(ToType));
	if (FromType == FIN_TRACE || FromType == FIN_OBJ || FromType == FIN_CLASS)
		Signature.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFINExpandedNetworkValueType(FromType, FFINReflection::Get()->FindClass(UObject::StaticClass()))));
	else
		Signature.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FromType));
	Signature.Pins.Add((ToType != FIN_TRACE && ToType != FIN_OBJ && ToType != FIN_CLASS) ? FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, ToType) : FFIVSFullPinType(FIVS_PIN_DATA_OUTPUT, FFINExpandedNetworkValueType(ToType, FFINReflection::Get()->FindClass(UObject::StaticClass()))));
	return Signature;
}

void UFIVSNodeSetProperty::InitPins() {
	ExecIn = CreatePin(FIVS_PIN_EXEC_INPUT, FText::FromString("Exec"));
	ExecOut = CreatePin(FIVS_PIN_EXEC_OUTPUT, FText::FromString("Out"));
	InstanceIn = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Instance"), FIN_TRACE);
	EFINNetworkValueType Type = Property->GetType();
	if (Type == FIN_OBJ) Type = FIN_TRACE;
	DataIn = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Value"), Type);
}

TArray<UFIVSPin*> UFIVSNodeSetProperty::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return TArray<UFIVSPin*>{InstanceIn, DataIn};
}

UFIVSPin* UFIVSNodeSetProperty::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	FFINNetworkTrace Instance = Context.GetValue(InstanceIn).GetTrace();
	FFINAnyNetworkValue Data = Context.GetValue(DataIn);
	FFINExecutionContext ExecContext(UFINNetworkUtils::RedirectIfPossible(Instance));
	Property->SetValue(ExecContext, Data);
	return ExecOut;
}

void UFIVSNodeGetProperty::InitPins() {
	InstanceIn = CreatePin(FIVS_PIN_DATA_INPUT, FText::FromString("Instance"), FIN_TRACE);
	EFINNetworkValueType Type = Property->GetType();
	if (Type == FIN_OBJ) Type = FIN_TRACE;
	DataOut = CreatePin(FIVS_PIN_DATA_OUTPUT, FText::FromString("Value"), Type);
}

TArray<UFIVSPin*> UFIVSNodeGetProperty::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return TArray<UFIVSPin*>{InstanceIn};
}

UFIVSPin* UFIVSNodeGetProperty::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	FFINNetworkTrace Instance = Context.GetValue(InstanceIn).GetTrace();
	FFINExecutionContext ExecContext(UFINNetworkUtils::RedirectIfPossible(Instance));
	FFINAnyNetworkValue Value = Property->GetValue(ExecContext);
	Context.SetValue(DataOut, Value);
	return nullptr;
}

FFIVSNodeSignature UFIVSNodeGetProperty::SignatureFromProperty(UFINProperty* Property, bool bWrite) {
	FFIVSNodeSignature Signature;
	Signature.Name = Property->GetDisplayName();
	Signature.Description = Property->GetDescription();
	FFIVSFullPinType PinType(Property);
	Signature.Pins.Add(FFIVSFullPinType(FIVS_PIN_DATA_INPUT, FFINExpandedNetworkValueType(FIN_TRACE, FFINReflection::Get()->FindClass(UObject::StaticClass()))));
	if (bWrite) {
		Signature.Pins.Add(FIVS_PIN_EXEC_INPUT);
		Signature.Pins.Add(FIVS_PIN_EXEC_OUTPUT);
		PinType.PinType |= FIVS_PIN_INPUT;
	} else {
		PinType.PinType |= FIVS_PIN_OUTPUT;
	}
	Signature.Pins.Add(PinType);
	return Signature;
}
