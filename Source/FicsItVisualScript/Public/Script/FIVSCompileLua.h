#pragma once

#include "CoreMinimal.h"
#include "FicsItVisualScriptModule.h"
#include "FINNetworkUtils.h"
#include "FIVSNode.h"
#include "FIVSPin.h"
#include "Interface.h"
#include "FIVSCompileLua.generated.h"

class UFIVSPin;
struct FFIVSLuaCompilerContext;

UINTERFACE()
class UFIVSCompileLuaInterface : public UInterface {
	GENERATED_BODY()
};

class FICSITVISUALSCRIPT_API IFIVSCompileLuaInterface {
	GENERATED_BODY()
public:
	virtual bool IsLuaRootNode() const { return false; }
	virtual void CompileNodeToLua(FFIVSLuaCompilerContext& Context) const = 0;
};

struct FICSITVISUALSCRIPT_API FFIVSLuaScope {
	TSharedPtr<FFIVSLuaScope> Parent;

	TSet<FString> LocalNames;
	TMap<UFIVSPin*, TTuple<int, TSharedRef<FFIVSLuaScope>, int>> EntrancePoints;
	TMap<UFIVSPin*, FString> Labels;
	TMap<UFIVSPin*, FString> RValues;
	TMap<UFIVSPin*, TOptional<FString>> LValues;

	FString FindUniqueLocalName(FString Name) const;
};

FString FIRValueToLuaLiteral(const FFIRAnyValue& Value);

USTRUCT()
struct FICSITVISUALSCRIPT_API FFIVSLuaCompilerContext {
	GENERATED_BODY()
private:
	TSharedRef<FFIVSLuaScope> CurrentScope;
	TArray<FString> Code;
	int CurrentIndentation = 0;

public:
	FFIVSLuaCompilerContext() : CurrentScope(MakeShared<FFIVSLuaScope>()) {}

	FString FindAndClaimLocalName(FString Name);
	void AddEntrance(UFIVSPin* ExecInputPin, int IndentationChange = 0);
	FString IndentString(FString String, int Level = -1) const;
	void AddPlain(const FString& Plain);
	void AddExpression(UFIVSPin* DataOutputPin, FString Expression);
	void Indentation(int IndentationChange);
	void EnterNewSection(int IndentationChange = 1);
	void LeaveSection(int IndentationChange = -1);
	void ContinueCurrentSection(UFIVSPin* ExecOutputPin);
	TOptional<FString> GetLValueExpression(UFIVSPin* DataInputPin);
	FString GetRValueExpression(UFIVSPin* DataInputPin);
	void AddRValue(UFIVSPin* DataOutputPin, const FString& RValue);
	void AddLValue(UFIVSPin* DataOutputPin, const FString& LValue);
	FString AddOutputPinAsVariable(UFIVSPin* DataOutputPin, FString Expression);
	FString FinalizeCode();
};
