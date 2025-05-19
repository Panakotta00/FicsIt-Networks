#pragma once

#include "CoreMinimal.h"
#include "FIRSource.h"
#include "FIRSourceStatic.generated.h"

struct FICSITREFLECTION_API FFIRStaticFuncParamReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	int ParamType;
	UFIRProperty*(*PropConstructor)(UObject*, FName);
};

struct FICSITREFLECTION_API FFIRStaticFuncReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	bool VarArgs;
	TFunction<void(const FFIRExecutionContext&, TArray<FIRAny>&)> Function;
	int Runtime;
	int FuncType;
	TMap<int, FFIRStaticFuncParamReg> Parameters;
};

struct FICSITREFLECTION_API FFIRStaticPropReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	TFunction<FIRAny(const FFIRExecutionContext&)>Get;
	int Runtime;
	int PropType;
	UFIRProperty*(*PropConstructor)(UObject*, FName);
	TFunction<void(const FFIRExecutionContext&, const FIRAny&)> Set;
};

struct FICSITREFLECTION_API FFIRStaticSignalParamReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	UFIRProperty*(*PropConstructor)(UObject*, FName);
};

struct FICSITREFLECTION_API FFIRStaticSignalReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	bool bIsVarArgs;
	TMap<int, FFIRStaticSignalParamReg> Parameters;
};

struct FICSITREFLECTION_API FFIRStaticClassReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	TMap<int, FFIRStaticFuncReg> Functions;
	TMap<int, FFIRStaticPropReg> Properties;
	TMap<int, FFIRStaticSignalReg> Signals;
};

struct FICSITREFLECTION_API FFIRStaticStructReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	bool bConstructable;
	TMap<int, FFIRStaticFuncReg> Functions;
	TMap<int, FFIRStaticPropReg> Properties;

	FFIRStaticStructReg() = default;
	FFIRStaticStructReg(const FString& InternalName, const FText& DisplayName, const FText& Description, bool bConstructable) : InternalName(InternalName), DisplayName(DisplayName), Description(Description), bConstructable(bConstructable) {}
};

UCLASS()
class FICSITREFLECTION_API UFIRSourceStatic : public UFIRSource {
	GENERATED_BODY()
	
	static TMap<UClass*, FFIRStaticClassReg> Classes;
	static TMap<UScriptStruct*, FFIRStaticStructReg> Structs;
	
public:
	static void AddClass(UClass* Class, const FFIRStaticClassReg& ClassReg) {
		Classes.FindOrAdd(Class) = ClassReg;
	}
	static void AddStruct(UScriptStruct* Struct, const FFIRStaticStructReg& StructReg) {
		Structs.Add(Struct, StructReg);
	}
	static void AddFunction(UClass* Class, int FuncID, const FFIRStaticFuncReg& FuncReg) {
		Classes.Find(Class)->Functions.FindOrAdd(FuncID) = FuncReg;
	}
	static void AddFunction(UScriptStruct* Struct, int FuncID, const FFIRStaticFuncReg& FuncReg) {
		FFIRStaticStructReg* StructReg = Structs.Find(Struct);
		FFIRStaticFuncReg& Reg = StructReg->Functions.FindOrAdd(FuncID, FuncReg);
		Reg.InternalName.RemoveFromEnd(TEXT("_0"));
	}
	static void AddFuncParam(UClass* Class, int FuncID, int ParamPos, const FFIRStaticFuncParamReg& ParamReg) {
		Classes.FindOrAdd(Class).Functions.FindOrAdd(FuncID).Parameters.FindOrAdd(ParamPos) = ParamReg;
	}
	static void AddFuncParam(UScriptStruct* Struct, int FuncID, int ParamPos, const FFIRStaticFuncParamReg& ParamReg) {
		Structs.FindOrAdd(Struct).Functions.FindOrAdd(FuncID).Parameters.FindOrAdd(ParamPos) = ParamReg;
	}
	static void AddProp(UClass* Class, int PropID, const FFIRStaticPropReg& PropReg) {
		Classes.FindOrAdd(Class).Properties.FindOrAdd(PropID) = PropReg;
	}
	static void AddProp(UScriptStruct* Struct, int PropID, const FFIRStaticPropReg& PropReg) {
		Structs.FindOrAdd(Struct).Properties.FindOrAdd(PropID) = PropReg;
	}
	static void AddPropSetter(UClass* Class, int PropID, const TFunction<void(const FFIRExecutionContext&, const FIRAny&)>& Set) {
		FFIRStaticPropReg& Reg = Classes.FindOrAdd(Class).Properties.FindOrAdd(PropID);
		Reg.Set = Set;
	}
	static void AddPropSetter(UScriptStruct* Struct, int PropID, const TFunction<void(const FFIRExecutionContext&, const FIRAny&)>& Set) {
		FFIRStaticPropReg& Reg = Structs.FindOrAdd(Struct).Properties.FindOrAdd(PropID);
		Reg.Set = Set;
	}
	static void AddSignal(UClass* Class, int SignalID, const FFIRStaticSignalReg& SignalReg) {
		Classes.FindOrAdd(Class).Signals.FindOrAdd(SignalID) = SignalReg;
	}
	static void AddSignalParam(UClass* Class, int SignalID, int ParamPos, const FFIRStaticSignalParamReg& SignalParamReg) {
		Classes.FindOrAdd(Class).Signals.FindOrAdd(SignalID).Parameters.FindOrAdd(ParamPos) = SignalParamReg;
	}

	// Begin UFINReflectionSource
	virtual bool ProvidesRequirements(UClass* Class) const override;
	virtual bool ProvidesRequirements(UScriptStruct* Struct) const override;
	virtual UFIRClass* FillData(FFicsItReflectionModule* Ref, UFIRClass* ToFillClass, UClass* Class) override;
	virtual UFIRStruct* FillData(FFicsItReflectionModule* Ref, UFIRStruct* ToFillStruct, UScriptStruct* Struct) override;
	// End UFINReflectionSource
};
