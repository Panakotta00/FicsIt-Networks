#include "FIVSCompileLua.h"

#include "FINNetworkComponent.h"
#include "FIVSEdObjectSelection.h"

FString FFIVSLuaScope::FindUniqueLocalName(FString Name) const {
	if (!LocalNames.Contains(Name)) return Name;
	FString underscore = Name.Append(TEXT("_"));
	for (int i = 2;; ++i) {
		FString extendedName = underscore;
		extendedName.AppendInt(i);
		if (!LocalNames.Contains(extendedName)) {
			return extendedName;
		}
	}
}

FString FIRValueToLuaLiteral(const FFIRAnyValue& Value) {
	switch (Value.GetType()) {
		case FIR_NIL:
			return TEXT("nil");
		case FIR_BOOL:
			return Value.GetBool() ? TEXT("true") : TEXT("false");
		case FIR_INT:
			return FString::Printf(TEXT("%lld"), Value.GetInt());
		case FIR_FLOAT:
			return FString::Printf(TEXT("%lg"), Value.GetFloat());
		case FIR_STR:
			return FString::Printf(TEXT("\"%s\""), *Value.GetString());
		case FIR_TRACE:
		case FIR_OBJ: {
			UObject* obj = UFINNetworkUtils::FindNetworkComponentFromObject(Value.GetObj().Get());
			if (!IsValid(obj)) {
				FString code;
				UObject* obj2 = UFINNetworkUtils::FindNetworkComponentFromObject(Value.GetTrace().GetStartPtr());
				if (IsValid(obj2)) {
					FGuid id = IFINNetworkComponent::Execute_GetID(obj2);
					code = FString::Printf(TEXT("component.proxy(\"%s\")"), *id.ToString());
				}
				code.Append(SFIVSEdTraceSelection::CompileTraceToLua(Value.GetTrace()));
				return code;
			}
			FGuid id = IFINNetworkComponent::Execute_GetID(obj);
			return FString::Printf(TEXT("component.proxy(\"%s\")"), *id.ToString());
		}
		case FIR_CLASS: {
			UClass* clazz = Value.GetClass();
			if (!IsValid(clazz)) return TEXT("nil");
			UFIRClass* FIRClass = FFicsItReflectionModule::Get().FindClass(clazz);
			if (!IsValid(FIRClass)) return TEXT("nil");
			return FString::Printf(TEXT("classes.%s"), *FIRClass->GetInternalName());
		}
		case FIR_STRUCT: {
			FFIRInstancedStruct Struct = Value.GetStruct();
			if (!Struct.GetData()) return TEXT("nil");
			UFIRStruct* FIRStruct = FFicsItReflectionModule::Get().FindStruct(Struct.GetStruct());
			if (!FIRStruct || !(FIRStruct->GetStructFlags() & FIR_Struct_Constructable)) return TEXT("nil");
			FStringBuilderBase str;
			str.Appendf(TEXT("structs.%s{"), *FIRStruct->GetInternalName());
			for (UFIRProperty* prop : FIRStruct->GetProperties()) {
				FFIRAnyValue propVal = prop->GetValue(FFIRExecutionContext(Struct.GetData()));
				str.Appendf(TEXT("%s=%s,"), *prop->GetInternalName(), *FIRValueToLuaLiteral(propVal));
			}
			str.Append(TEXT("}"));
			return str.ToString();
		}
		case FIR_ARRAY: {
			TArray<FFIRAnyValue> Array = Value.GetArray();
			FStringBuilderBase str;
			str.Append(TEXT("{"));
			for (const FFIRAnyValue& child : Array) {
				str.Appendf(TEXT("%s,"), *FIRValueToLuaLiteral(child));
			}
			str.Append(TEXT("}"));
			return str.ToString();
		}
		case FIR_ANY:
			return FIRValueToLuaLiteral(Value.GetAny());
		default: break;
	}
	return TEXT("nil");
}

FString FFIVSLuaCompilerContext::FindAndClaimLocalName(FString Name) {
	FString localName = CurrentScope->FindUniqueLocalName(Name);
	CurrentScope->LocalNames.Add(localName);
	return localName;
}

void FFIVSLuaCompilerContext::AddEntrance(UFIVSPin* ExecInputPin, int IndentationChange) {
	if (!IsValid(ExecInputPin)) return;
	fgcheck(ExecInputPin->GetPinType() & FIVS_PIN_EXEC);
	fgcheck(ExecInputPin->GetPinType() & FIVS_PIN_INPUT);
	CurrentScope->EntrancePoints.Add(ExecInputPin, {Code.Num(), CurrentScope, CurrentIndentation + IndentationChange});
}

FString FFIVSLuaCompilerContext::IndentString(FString String, int Level) const {
	if (Level < 0) Level = CurrentIndentation;
	bool bEndRemoved = String.RemoveFromEnd(TEXT("\n"));
	FString indent;
	for (int i = 0; i < Level; ++i) indent.AppendChar(TEXT('\t'));
	String = indent + String;
	indent.AppendChar(TEXT('\n'));
	String.ReplaceInline(TEXT("\n"), *indent);
	if (bEndRemoved) {
		String.AppendChar(TEXT('\n'));
	}
	return String;
}

void FFIVSLuaCompilerContext::AddPlain(const FString& Plain) {
	Code.Add(IndentString(Plain));
}

void FFIVSLuaCompilerContext::AddExpression(UFIVSPin* DataOutputPin, FString Expression) {
	fgcheck(DataOutputPin->GetPinType() & FIVS_PIN_DATA);
	fgcheck(DataOutputPin->GetPinType() & FIVS_PIN_OUTPUT);
	CurrentScope->RValues.Add(DataOutputPin, MoveTemp(Expression));
}

void FFIVSLuaCompilerContext::Indentation(int IndentationChange) {
	CurrentIndentation = FMath::Max(0, CurrentIndentation + IndentationChange);
}

void FFIVSLuaCompilerContext::EnterNewSection(int IndentationChange) {
	Indentation(IndentationChange);
	TSharedRef<FFIVSLuaScope> newScope = MakeShared<FFIVSLuaScope>(*CurrentScope);
	newScope->Parent = CurrentScope;
	CurrentScope = newScope;
}

void FFIVSLuaCompilerContext::LeaveSection(int IndentationChange) {
	Indentation(IndentationChange);
	TSharedPtr<FFIVSLuaScope> parent = CurrentScope->Parent;
	fgcheck(parent.IsValid());
	CurrentScope = parent.ToSharedRef();
}

void FFIVSLuaCompilerContext::ContinueCurrentSection(UFIVSPin* ExecOutputPin) {
	if (!IsValid(ExecOutputPin)) return;
	fgcheck(ExecOutputPin->GetPinType() & FIVS_PIN_EXEC);
	fgcheck(ExecOutputPin->GetPinType() & FIVS_PIN_OUTPUT);

	UFIVSPin* nextPin = ExecOutputPin->FindConnected();
	if (!IsValid(nextPin)) return;
	fgcheck(nextPin->ParentNode);

	if (FString* label = CurrentScope->Labels.Find(nextPin)) {
		AddPlain(FString::Printf(TEXT("goto %s\n"), **label));
		return;
	} else if (TTuple<int, TSharedRef<FFIVSLuaScope>, int>* segment = CurrentScope->EntrancePoints.Find(nextPin)) {
		FString newLabel = CurrentScope->FindUniqueLocalName(FString::Printf(TEXT("%s_%s"), *nextPin->ParentNode->GetName(), *nextPin->Name));
		if (Code.Num() <= segment->Get<0>()) {
			Code.Emplace();
		}
		Code[segment->Get<0>()] = IndentString(FString::Printf(TEXT("::%s::\n"), *newLabel), segment->Get<2>()).Append(Code[segment->Get<0>()]);
		TSharedPtr<FFIVSLuaScope> scope = CurrentScope;
		while (scope) {
			scope->Labels.Add(nextPin, newLabel);
			scope->LocalNames.Add(newLabel);
			if (scope == segment->Get<1>()) {
				break;
			}
			scope = scope->Parent;
		}
		AddPlain(FString::Printf(TEXT("goto %s\n"), *newLabel));
		return;
	}

	fgcheck(nextPin->ParentNode->Implements<UFIVSCompileLuaInterface>())
	Cast<IFIVSCompileLuaInterface>(nextPin->ParentNode)->CompileNodeToLua(*this);
}

TOptional<FString> FFIVSLuaCompilerContext::GetLValueExpression(UFIVSPin* DataInputPin) {
	fgcheck(IsValid(DataInputPin));
	fgcheck(DataInputPin->GetPinType() & FIVS_PIN_DATA);
	fgcheck(DataInputPin->GetPinType() & FIVS_PIN_INPUT);

	UFIVSPin* connected = DataInputPin->FindConnected();
	if (!IsValid(connected)) {
		return {};
	}

	TOptional<FString>* expression = CurrentScope->LValues.Find(connected);
	if (expression) return *expression;

	fgcheck(connected->ParentNode);
	fgcheck(connected->ParentNode->Implements<UFIVSCompileLuaInterface>())
	Cast<IFIVSCompileLuaInterface>(connected->ParentNode)->CompileNodeToLua(*this);

	expression = CurrentScope->LValues.Find(connected);
	if (expression) return *expression;

	CurrentScope->LValues.Add(connected, {});

	return {};
}

FString FFIVSLuaCompilerContext::GetRValueExpression(UFIVSPin* DataInputPin) {
	fgcheck(IsValid(DataInputPin));
	fgcheck(DataInputPin->GetPinType() & FIVS_PIN_DATA);
	fgcheck(DataInputPin->GetPinType() & FIVS_PIN_INPUT);
	UFIVSPin* connected = DataInputPin->FindConnected();
	if (!IsValid(connected)) {
		FFIRAnyValue literal = DataInputPin->GetLiteral();
		return FIRValueToLuaLiteral(literal);
	}
	FString* expression = CurrentScope->RValues.Find(connected);
	if (expression) {
		return *expression;
	}

	fgcheck(connected->ParentNode);
	fgcheck(connected->ParentNode->Implements<UFIVSCompileLuaInterface>())

	Cast<IFIVSCompileLuaInterface>(connected->ParentNode)->CompileNodeToLua(*this);

	expression = CurrentScope->RValues.Find(connected);
	if (expression) {
		return *expression;
	}

	return FIRValueToLuaLiteral(DataInputPin->GetLiteral());
}

void FFIVSLuaCompilerContext::AddRValue(UFIVSPin* DataOutputPin, const FString& RValue) {
	fgcheck(IsValid(DataOutputPin));
	fgcheck(DataOutputPin->GetPinType() & FIVS_PIN_DATA);
	fgcheck(DataOutputPin->GetPinType() & FIVS_PIN_OUTPUT);
	CurrentScope->RValues.Add(DataOutputPin, RValue);
}

void FFIVSLuaCompilerContext::AddLValue(UFIVSPin* DataOutputPin, const FString& LValue) {
	fgcheck(IsValid(DataOutputPin));
	fgcheck(DataOutputPin->GetPinType() & FIVS_PIN_DATA);
	fgcheck(DataOutputPin->GetPinType() & FIVS_PIN_OUTPUT);
	CurrentScope->RValues.Add(DataOutputPin, LValue);
	CurrentScope->LValues.Add(DataOutputPin, LValue);
}

FString FFIVSLuaCompilerContext::AddOutputPinAsVariable(UFIVSPin* DataOutputPin, FString Expression) {
	fgcheck(IsValid(DataOutputPin));
	fgcheck(DataOutputPin->GetPinType() & FIVS_PIN_DATA);
	fgcheck(DataOutputPin->GetPinType() & FIVS_PIN_OUTPUT);

	FString varName = FindAndClaimLocalName(DataOutputPin->GetName());
	AddPlain(FString::Printf(TEXT("local %s = %s\n"), *varName, *Expression));
	AddLValue(DataOutputPin, varName);
	return varName;
}

FString FFIVSLuaCompilerContext::FinalizeCode() {
	FStringBuilderBase finalCode;
	for (const FString& segment : Code) {
		finalCode.Append(segment);
	}
	finalCode.Append(TEXT("future.loop()\n"));
	return finalCode.ToString();
}

void FFIVSLuaCompilerContext::AddCompileError(const FFIVSCompileError& Error) {
	CompileErrors.Add(Error);
}
