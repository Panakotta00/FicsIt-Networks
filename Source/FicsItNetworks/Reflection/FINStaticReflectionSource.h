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
	TFunction<void(const FINTrace&, TArray<FINAny>&)> Function;
	TMap<int, FFINStaticFuncParamReg> Parameters;
	int Runtime;
};

struct FFINStaticPropReg {
	FString InternalName;
	FText DisplayName;
	FText Description;
	UFINProperty*(*PropConstructor)(UObject*);
	TFunction<FINAny(void*)>Get;
	TFunction<void(void*, const FINAny&)> Set;
	int Runtime;
};

struct FFINStaticClassReg {
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
	
public:
	static void AddClass(UClass* Class, const FString& InternalName, const FText& DisplayName, const FText& Description);
	static void AddFunction(UClass* Class, int FuncID, const FString& InternalName, const FText& DisplayName, const FText& Description, bool VarArgs,  const TFunction<void(const FINTrace&, TArray<FINAny>&)>& Func, int Runtime);
	template<typename T>
	static void AddFuncParam(UClass* Class, int FuncID, int ParamPos, const FString& InternalName, const FText& DisplayName, const FText& Description, int ParamType) {
		FFINStaticFuncParamReg& Reg = Classes.FindOrAdd(Class).Functions.FindOrAdd(FuncID).Parameters.FindOrAdd(ParamPos);
		Reg.InternalName = InternalName;
		Reg.DisplayName = DisplayName;
		Reg.Description = Description;
		Reg.ParamType = ParamType;
		Reg.PropConstructor = &T::PropConstructor;
	}
	template<typename T>
	static void AddProp(UClass* Class, int PropID, const FString& InternalName, const FText& DisplayName, const FText& Description, const TFunction<FINAny(void*)>& Get, int Runtime) {
		FFINStaticPropReg& Reg = Classes.FindOrAdd(Class).Properties.FindOrAdd(PropID);
		Reg.InternalName = InternalName;
		Reg.DisplayName = DisplayName;
		Reg.Description = Description;
		Reg.PropConstructor = &T::PropConstructor;
		Reg.Get = Get;
		Reg.Runtime = Runtime;
	}
	static void AddPropSetter(UClass* Class, int PropID, const TFunction<void(void*, const FINAny&)>& Set) {
		FFINStaticPropReg& Reg = Classes.FindOrAdd(Class).Properties.FindOrAdd(PropID);
		Reg.Set = Set;
	}

	// Begin UFINReflectionSource
	virtual bool ProvidesRequirements(UClass* Class) const override;
	virtual void FillData(FFINReflection* Ref, UFINClass* ToFillClass, UClass* Class) const override;
	// End UFINReflectionSource
};
