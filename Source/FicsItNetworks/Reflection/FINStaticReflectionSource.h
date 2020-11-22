#pragma once

#include "FINReflectionSource.h"
#include "FINStaticReflectionSource.generated.h"

struct FFINStaticFuncParamReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	int ParamType;
	UFINProperty*(*PropConstructor)(UObject*);
};

struct FFINStaticFuncReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	bool VarArgs;
	TFunction<void(const FFINExecutionContext&, TArray<FINAny>&)> Function;
	int Runtime;
	int FuncType;
	TMap<int, FFINStaticFuncParamReg> Parameters;
};

struct FFINStaticPropReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	TFunction<FINAny(const FFINExecutionContext&)>Get;
	int Runtime;
	int PropType;
	UFINProperty*(*PropConstructor)(UObject*);
	TFunction<void(const FFINExecutionContext&, const FINAny&)> Set;
};

struct FFINStaticClassReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	TMap<int, FFINStaticFuncReg> Functions;
	TMap<int, FFINStaticPropReg> Properties;
};

struct FFINStaticStructReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	TMap<int, FFINStaticFuncReg> Functions;
	TMap<int, FFINStaticPropReg> Properties;
};

UCLASS()
class UFINStaticReflectionSource : public UFINReflectionSource {
	GENERATED_BODY()
	
	static TMap<UClass*, FFINStaticClassReg> Classes;
	static TMap<UScriptStruct*, FFINStaticStructReg> Structs;
	
public:
	static void AddClass(UClass* Class, const FFINStaticClassReg& ClassReg) {
		Classes.FindOrAdd(Class) = ClassReg;
	}
	static void AddStruct(UScriptStruct* Struct, const FFINStaticStructReg& StructReg) {
		Structs.FindOrAdd(Struct) = StructReg;
	}
	static void AddFunction(UClass* Class, int FuncID, const FFINStaticFuncReg& FuncReg) {
		Classes.Find(Class)->Functions.FindOrAdd(FuncID) = FuncReg;
	}
	static void AddFunction(UScriptStruct* Struct, int FuncID, const FFINStaticFuncReg& FuncReg) {
		Structs.Find(Struct)->Functions.FindOrAdd(FuncID) = FuncReg;
	}
	static void AddFuncParam(UClass* Class, int FuncID, int ParamPos, const FFINStaticFuncParamReg& ParamReg) {
		Classes.FindOrAdd(Class).Functions.FindOrAdd(FuncID).Parameters.FindOrAdd(ParamPos) = ParamReg;
	}
	static void AddFuncParam(UScriptStruct* Struct, int FuncID, int ParamPos, const FFINStaticFuncParamReg& ParamReg) {
		Structs.FindOrAdd(Struct).Functions.FindOrAdd(FuncID).Parameters.FindOrAdd(ParamPos) = ParamReg;
	}
	static void AddProp(UClass* Class, int PropID, const FFINStaticPropReg& PropReg) {
		Classes.FindOrAdd(Class).Properties.FindOrAdd(PropID) = PropReg;
	}
	static void AddProp(UScriptStruct* Struct, int PropID, const FFINStaticPropReg& PropReg) {
		Structs.FindOrAdd(Struct).Properties.FindOrAdd(PropID) = PropReg;
	}
	static void AddPropSetter(UClass* Class, int PropID, const TFunction<void(const FFINExecutionContext&, const FINAny&)>& Set) {
		FFINStaticPropReg& Reg = Classes.FindOrAdd(Class).Properties.FindOrAdd(PropID);
		Reg.Set = Set;
	}
	static void AddPropSetter(UScriptStruct* Struct, int PropID, const TFunction<void(const FFINExecutionContext&, const FINAny&)>& Set) {
		FFINStaticPropReg& Reg = Structs.FindOrAdd(Struct).Properties.FindOrAdd(PropID);
		Reg.Set = Set;
	}

	// Begin UFINReflectionSource
	virtual bool ProvidesRequirements(UClass* Class) const override;
	virtual bool ProvidesRequirements(UScriptStruct* Struct) const override;
	virtual void FillData(FFINReflection* Ref, UFINClass* ToFillClass, UClass* Class) const override;
	virtual void FillData(FFINReflection* Ref, UFINStruct* ToFillStruct, UScriptStruct* Struct) const override;
	// End UFINReflectionSource
};
