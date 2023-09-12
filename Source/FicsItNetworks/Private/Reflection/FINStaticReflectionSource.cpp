#include "Reflection/FINStaticReflectionSource.h"

#include "FGBuildableDoor.h"
#include "Reflection/FINArrayProperty.h"
#include "Reflection/FINBoolProperty.h"
#include "Reflection/FINClassProperty.h"
#include "Reflection/FINFloatProperty.h"
#include "Reflection/FINFuncProperty.h"
#include "Reflection/FINIntProperty.h"
#include "Reflection/FINObjectProperty.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStrProperty.h"
#include "Reflection/FINStructProperty.h"
#include "Reflection/FINTraceProperty.h"
#include "Reflection/ReflectionHelper.h"
#include "FINGlobalRegisterHelper.h"

#include "Computer/FINComputerGPUT1.h"
#include "Network/FINFuture.h"
#include "Network/FINNetworkConnectionComponent.h"
#include "Utils/FINTargetPoint.h"
#include "Utils/FINTimeTableStop.h"
#include "Utils/FINTrackGraph.h"
#include "Utils/FINRailroadSignalBlock.h"
#include "Reflection/FINStaticReflectionSourceHooks.h"

#include "FGFactoryConnectionComponent.h"
#include "FGPipeConnectionComponent.h"
#include "FGGameState.h"
#include "FGHealthComponent.h"
#include "FGItemCategory.h"
#include "FGLocomotive.h"
#include "FGPipeSubsystem.h"
#include "FGPowerCircuit.h"
#include "FGPowerConnectionComponent.h"
#include "FGPowerInfoComponent.h"
#include "FGRailroadSubsystem.h"
#include "FGRailroadTimeTable.h"
#include "FGRailroadTrackConnectionComponent.h"
#include "FGRailroadVehicleMovementComponent.h"
#include "FGResourceSinkSubsystem.h"
#include "FGSignLibrary.h"
#include "FGTrainStationIdentifier.h"
#include "Buildables/FGBuildableCircuitSwitch.h"
#include "Buildables/FGBuildableDockingStation.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableLightsControlPanel.h"
#include "Buildables/FGBuildableLightSource.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "Buildables/FGBuildablePipelinePump.h"
#include "Buildables/FGBuildablePipeReservoir.h"
#include "Buildables/FGBuildablePixelSign.h"
#include "Buildables/FGBuildablePowerStorage.h"
#include "Buildables/FGBuildablePriorityPowerSwitch.h"
#include "Buildables/FGBuildableRailroadSignal.h"
#include "Buildables/FGBuildableRailroadStation.h"
#include "Buildables/FGBuildableRailroadSwitchControl.h"
#include "Buildables/FGBuildableResourceSink.h"
#include "Buildables/FGBuildableSignBase.h"
#include "Buildables/FGBuildableSpeedSign.h"
#include "Buildables/FGBuildableTrainPlatform.h"
#include "Buildables/FGBuildableTrainPlatformCargo.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "WheeledVehicles/FGTargetPointLinkedList.h"
#include "WheeledVehicles/FGWheeledVehicle.h"

TMap<UClass*, FFINStaticClassReg> UFINStaticReflectionSource::Classes;
TMap<UScriptStruct*, FFINStaticStructReg> UFINStaticReflectionSource::Structs;

bool UFINStaticReflectionSource::ProvidesRequirements(UClass* Class) const {
	return Classes.Contains(Class);
}

bool UFINStaticReflectionSource::ProvidesRequirements(UScriptStruct* Struct) const {
	return Structs.Contains(Struct);
}

void UFINStaticReflectionSource::FillData(FFINReflection* Ref, UFINClass* ToFillClass, UClass* Class) const {
	const FFINStaticClassReg* ClassReg = Classes.Find(Class);
	if (!ClassReg) return;
	ToFillClass->InternalName = ClassReg->InternalName;
	ToFillClass->DisplayName = ClassReg->DisplayName;
	ToFillClass->Description = ClassReg->Description;
	ToFillClass->Parent = Ref->FindClass(Class->GetSuperClass());
	if (ToFillClass->Parent == ToFillClass) ToFillClass->Parent = nullptr;

	for (const TPair<int, FFINStaticFuncReg>& KVFunc : ClassReg->Functions) {
		const FFINStaticFuncReg& Func = KVFunc.Value;
		UFINFunction* FINFunc = NewObject<UFINFunction>(ToFillClass);
		FINFunc->InternalName = Func.InternalName;
		FINFunc->DisplayName = Func.DisplayName;
		FINFunc->Description = Func.Description;
		if (Func.VarArgs) FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_VarArgs;
		switch (Func.Runtime) {
		case 0:
			FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Sync;
			break;
		case 1:
			FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Parallel;
			break;
		case 2:
			FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Async;
			break;
		default:
			break;
		}
		switch (Func.FuncType) {
		case 1:
			FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_ClassFunc;
			break;
		case 2:
			FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_StaticFunc;
			break;
		default:
			FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_MemberFunc;
			break;
		}

		TArray<int> ParamPos;
		Func.Parameters.GetKeys(ParamPos);
		ParamPos.Sort();
		for (int Pos : ParamPos) {
			const FFINStaticFuncParamReg& Param = Func.Parameters[Pos];
			UFINProperty* FINProp = Param.PropConstructor(FINFunc);
			FINProp->InternalName = Param.InternalName;
			FINProp->DisplayName = Param.DisplayName;
			FINProp->Description = Param.Description;
			FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Param;
			switch (Param.ParamType) {
				case 2:
					FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_RetVal;
				case 1:
					FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_OutParam;
					break;
				default: break;
			}
			FINFunc->Parameters.Add(FINProp);
		}

		auto NFunc = Func.Function;
		FINFunc->NativeFunction = [Func](const FFINExecutionContext& Ctx, const TArray<FINAny>& InValues) -> TArray<FINAny> {
			TArray<FINAny> Parameters;
			TArray<int> Pos;
			Func.Parameters.GetKeys(Pos);
			Pos.Sort();
			int j = 0;
			if (Pos.Num() > 0) for (int i = 0; i <= Pos[Pos.Num()-1]; ++i) {
				const FFINStaticFuncParamReg* Reg = Func.Parameters.Find(i);
				if (Reg && Reg->ParamType == 0) {
					Parameters.Add(InValues[j++]);
				} else {
					Parameters.Add(FINAny());
				}
			}
			for (; j < InValues.Num(); j++) Parameters.Add(InValues[j]);
			Func.Function(Ctx, Parameters);
			
			TArray<FINAny> OutValues;
			if (Pos.Num() > 0) for (int i = 0; i <= Pos[Pos.Num()-1]; ++i) {
				const FFINStaticFuncParamReg* Reg = Func.Parameters.Find(i);
				if (Reg && Reg->ParamType > 0) {
					OutValues.Add(Parameters[i]);
					j++;
				}
			}
			for (; j < Parameters.Num();) OutValues.Add(Parameters[j++]);
			return OutValues;
		};
		
		ToFillClass->Functions.Add(FINFunc);
	}

	for (const TPair<int, FFINStaticPropReg>& KVProp : ClassReg->Properties) {
		const FFINStaticPropReg& Prop = KVProp.Value;
		UFINProperty* FINProp = Prop.PropConstructor(ToFillClass);
		FINProp->InternalName = Prop.InternalName;
		FINProp->DisplayName = Prop.DisplayName;
		FINProp->Description = Prop.Description;
		FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Attrib;
		if (UFINFuncProperty* FINFuncProp = Cast<UFINFuncProperty>(FINProp)) {
			FINFuncProp->GetterFunc.GetterFunc = Prop.Get;
			if ((bool)Prop.Set) FINFuncProp->SetterFunc.SetterFunc = Prop.Set;
			else FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_ReadOnly;
		}
		switch (Prop.Runtime) {
		case 0:
			FINProp->PropertyFlags = (FINProp->PropertyFlags & ~FIN_Prop_Runtime) | FIN_Prop_Sync;
			break;
		case 1:
			FINProp->PropertyFlags = (FINProp->PropertyFlags & ~FIN_Prop_Runtime) | FIN_Prop_Parallel;
			break;
		case 2:
			FINProp->PropertyFlags = (FINProp->PropertyFlags & ~FIN_Prop_Runtime) | FIN_Prop_Async;
			break;
		default:
			break;
		}
		switch (Prop.PropType) {
		case 1:
			FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_ClassProp;
			break;
		default:
			break;
		}
		ToFillClass->Properties.Add(FINProp);
	}

	for (const TPair<int, FFINStaticSignalReg>& KVSignal : ClassReg->Signals) {
		const FFINStaticSignalReg& Signal = KVSignal.Value;
		UFINSignal* FINSignal = NewObject<UFINSignal>(ToFillClass);
		FINSignal->InternalName = Signal.InternalName;
		FINSignal->DisplayName = Signal.DisplayName;
		FINSignal->Description = Signal.Description;
		FINSignal->bIsVarArgs = Signal.bIsVarArgs;

		TArray<int> ParamPos;
		Signal.Parameters.GetKeys(ParamPos);
		ParamPos.Sort();
		for (int Pos : ParamPos) {
			const FFINStaticSignalParamReg& Param = Signal.Parameters[Pos];
			UFINProperty* FINProp = Param.PropConstructor(FINSignal);
			FINProp->InternalName = Param.InternalName;
			FINProp->DisplayName = Param.DisplayName;
			FINProp->Description = Param.Description;
			FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Param;
			FINSignal->Parameters.Add(FINProp);
		}
		ToFillClass->Signals.Add(FINSignal);
	}
}

void UFINStaticReflectionSource::FillData(FFINReflection* Ref, UFINStruct* ToFillStruct, UScriptStruct* Struct) const {
	const FFINStaticStructReg* StructReg = Structs.Find(Struct);
	if (!StructReg) return;
	ToFillStruct->InternalName = StructReg->InternalName;
	ToFillStruct->DisplayName = StructReg->DisplayName;
	ToFillStruct->Description = StructReg->Description;
	ToFillStruct->Parent = Ref->FindStruct(Cast<UScriptStruct>(Struct->GetSuperStruct()));
	if (ToFillStruct->Parent == ToFillStruct) ToFillStruct->Parent = nullptr;

	for (const TPair<int, FFINStaticFuncReg>& KVFunc : StructReg->Functions) {
		const FFINStaticFuncReg& Func = KVFunc.Value;
		UFINFunction* FINFunc = NewObject<UFINFunction>(ToFillStruct);
		FINFunc->InternalName = Func.InternalName;
		FINFunc->DisplayName = Func.DisplayName;
		FINFunc->Description = Func.Description;
		if (Func.VarArgs) FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_VarArgs;
		switch (Func.Runtime) {
		case 0:
			FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Sync;
			break;
		case 1:
			FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Parallel;
			break;
		case 2:
			FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Async;
			break;
		default:
			break;
		}
		switch (Func.FuncType) {
		case 1:
			FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_ClassFunc;
			break;
		case 2:
			FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_StaticFunc;
			break;
		default:
			FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_MemberFunc;
			break;
		}

		TArray<int> ParamPos;
		Func.Parameters.GetKeys(ParamPos);
		ParamPos.Sort();
		for (int Pos : ParamPos) {
			const FFINStaticFuncParamReg& Param = Func.Parameters[Pos];
			UFINProperty* FINProp = Param.PropConstructor(FINFunc);
			FINProp->InternalName = Param.InternalName;
			FINProp->DisplayName = Param.DisplayName;
			FINProp->Description = Param.Description;
			FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Param;
			switch (Param.ParamType) {
				case 2:
					FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_RetVal;
				case 1:
					FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_OutParam;
					break;
				default: break;
			}
			FINFunc->Parameters.Add(FINProp);
		}

		auto NFunc = Func.Function;
		FINFunc->NativeFunction = [Func](const FFINExecutionContext& Ctx, const TArray<FINAny>& Params) -> TArray<FINAny> {
			TArray<FINAny> Parameters;
			TArray<int> Pos;
			Func.Parameters.GetKeys(Pos);
			Pos.Sort();
			int j = 0;
			if (Pos.Num() > 0) for (int i = 0; i <= Pos[Pos.Num()-1]; ++i) {
				const FFINStaticFuncParamReg* Reg = Func.Parameters.Find(i);
				if (Reg && Reg->ParamType == 0) {
					Parameters.Add(Params[j++]);
				} else {
					Parameters.Add(FINAny());
				}
			}
			Func.Function(Ctx, Parameters);

			TArray<FINAny> OutValues;
			if (Pos.Num() > 0) for (int i = 0; i <= Pos[Pos.Num()-1]; ++i) {
				const FFINStaticFuncParamReg* Reg = Func.Parameters.Find(i);
				if (Reg && Reg->ParamType > 0) {
					OutValues.Add(Parameters[i]);
				}
			}
			return OutValues;
		};
		
		ToFillStruct->Functions.Add(FINFunc);
	}

	for (const TPair<int, FFINStaticPropReg>& KVProp : StructReg->Properties) {
		const FFINStaticPropReg& Prop = KVProp.Value;
		UFINProperty* FINProp = Prop.PropConstructor(ToFillStruct);
		FINProp->InternalName = Prop.InternalName;
		FINProp->DisplayName = Prop.DisplayName;
		FINProp->Description = Prop.Description;
		FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Attrib;
		if (UFINFuncProperty* FINFuncProp = Cast<UFINFuncProperty>(FINProp)) {
			FINFuncProp->GetterFunc.GetterFunc = Prop.Get;
			if ((bool)Prop.Set) FINFuncProp->SetterFunc.SetterFunc = Prop.Set;
			else FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_ReadOnly;
		}
		switch (Prop.Runtime) {
		case 0:
			FINProp->PropertyFlags = (FINProp->PropertyFlags & ~FIN_Prop_Runtime) | FIN_Prop_Sync;
			break;
		case 1:
			FINProp->PropertyFlags = (FINProp->PropertyFlags & ~FIN_Prop_Runtime) | FIN_Prop_Parallel;
			break;
		case 2:
			FINProp->PropertyFlags = (FINProp->PropertyFlags & ~FIN_Prop_Runtime) | FIN_Prop_Async;
			break;
		default:
			break;
		}
		switch (Prop.PropType) {
		case 1:
			FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_ClassProp;
			break;
		default:
			break;
		}
		ToFillStruct->Properties.Add(FINProp);
	}
}


#define TypeClassName(Type) FIN_StaticRef_ ## Type
#define NSName "FIN_StaticReflection"
#define FINRefLocText(KeyName, Value) FInternationalization::ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText(Value, TEXT(NSName), KeyName)
#define FINRefTypeLocText(KeyName, Value) FINRefLocText(*(FString(TName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define BeginClass(Type, InternalName, DisplayName, Description) \
	namespace TypeClassName(Type) { \
		using T = Type; \
		constexpr auto TName = TEXT(#Type) ; \
		UClass* GetUType() { return T::StaticClass(); } \
		FORCEINLINE T* GetFromCtx(const FFINExecutionContext& Ctx) { return Cast<T>(Ctx.GetObject()); } \
		FFINStaticGlobalRegisterFunc RegClass([](){ \
			UFINStaticReflectionSource::AddClass(T::StaticClass(), FFINStaticClassReg{TEXT(InternalName), FINRefTypeLocText("DisplayName", DisplayName), FINRefTypeLocText("Description", Description)}); \
		});
#define EndClass() };
#define TypeStructName(Type) FIN_StaticRef_ ## Type
#define BeginStruct(Type, InternalName, DisplayName, Description) \
	namespace TypeStructName(Type) { \
		using T = Type; \
		constexpr auto TName = TEXT(#Type) ; \
		UScriptStruct* GetUType() { return TBaseStructure<T>::Get(); } \
		FORCEINLINE T* GetFromCtx(const FFINExecutionContext& Ctx) { return static_cast<T*>(Ctx.GetGeneric()); } \
		FFINStaticGlobalRegisterFunc RegStruct([](){ \
			UFINStaticReflectionSource::AddStruct(GetUType(), FFINStaticStructReg{TEXT(InternalName), FINRefTypeLocText("DisplayName", DisplayName), FINRefTypeLocText("Description", Description)}); \
		});
#define EndStruct() };
#define GetClassFunc [](){ return T::StaticClass(); }
#define FuncClassName(Prefix, Func) FIN_StaticRefFunc_ ## Prefix ## _ ## Func
#define FINRefFuncLocText(KeyName, Value) FINRefLocText(*(FString(TName) + TEXT("_") + FString(FName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define BeginFuncRT(Prefix, InternalName, DisplayName, Description, Varargs, FuncType, Runtime) \
	namespace FuncClassName(Prefix, InternalName) { \
		constexpr int F = __COUNTER__; \
		constexpr auto FName = TEXT(#InternalName) ; \
		void Execute(const FFINExecutionContext& Ctx, TArray<FINAny>& Params); \
		FFINStaticGlobalRegisterFunc RegClass([](){ \
			UFINStaticReflectionSource::AddFunction(GetUType(), F, FFINStaticFuncReg{TEXT(#InternalName), FINRefFuncLocText("DisplayName", DisplayName), FINRefFuncLocText("Description", Description), Varargs, &Execute, Runtime, FuncType}); \
			TArray<FINAny> Params; \
			Execute(FINTrace(nullptr), Params); \
		}); \
		void Execute(const FFINExecutionContext& Ctx, TArray<FINAny>& Params) { \
		static bool _bGotReg = false;
#define GET_MACRO(_0, VAL,...) VAL
#define BeginFunc(InternalName, DisplayName, Description, ...) BeginFuncRT(Member, InternalName, DisplayName, Description, false, 0, GET_MACRO(0 , ##__VA_ARGS__, 1) ) \
		T* self = GetFromCtx(Ctx);
#define BeginFuncVA(InternalName, DisplayName, Description, ...) BeginFuncRT(Member, InternalName, DisplayName, Description, true, 0, GET_MACRO(0, ##__VA_ARGS__, 1) ) \
		T* self = GetFromCtx(Ctx);
#define BeginClassFunc(InternalName, DisplayName, Description, VA, ...) BeginFuncRT(Class, InternalName, DisplayName, Description, VA, 1, GET_MACRO(0, ##__VA_ARGS__, 1) ) \
		TSubclassOf<T> self = Cast<UClass>(Ctx.GetObject());
#define BeginStaticFunc(InternalName, DisplayName, Description, VA, ...) BeginFuncRT(Static, InternalName, DisplayName, Description, VA, 2, GET_MACRO(0, ##__VA_ARGS__, 1) )
#define Body() \
			if (self && _bGotReg) {
#define EndFunc() \
			else if (!_bGotReg) _bGotReg = true; \
			} \
		} \
	};
#define PropClassName(Prefix, Prop) FIN_StaticRefProp_ ## Prefix ## _ ## Prop
#define FINRefPropLocText(KeyName, Value) FINRefLocText(*(FString(TName) + TEXT("_") + FString(PName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define BeginPropRT(Prefix, Type, InternalName, DisplayName, Description, PropType, Runtime) \
	namespace PropClassName(Prefix, InternalName) { \
		const int P = __COUNTER__; \
		constexpr auto PName = TEXT(#InternalName) ; \
		using PT = Type; \
		FINAny Get(const FFINExecutionContext& Ctx); \
		FFINStaticGlobalRegisterFunc RegProp([](){ \
			UFINStaticReflectionSource::AddProp(GetUType(), P, FFINStaticPropReg{TEXT(#InternalName), FINRefPropLocText("DisplayName", DisplayName), FINRefPropLocText("Description", Description), &Get, Runtime, PropType, &PT::PropConstructor}); \
		}); \
		FINAny Get(const FFINExecutionContext& Ctx) {
#define BeginProp(Type, InternalName, DisplayName, Description, ...) BeginPropRT(Member, Type, InternalName, DisplayName, Description, 0, GET_MACRO(0, ##__VA_ARGS__, 1) ) \
	T* self = GetFromCtx(Ctx);
#define BeginClassProp(Type, InternalName, DisplayName, Description, ...) BeginPropRT(Class, Type, InternalName, DisplayName, Description, 1, GET_MACRO(0, ##__VA_ARGS__, 1) ) \
	TSubclassOf<T> self = Cast<UClass>(Ctx.GetObject());
#define Return \
		return (FINAny)
#define PropSet() \
		} \
		void Set(const FFINExecutionContext& Ctx, const FINAny& Val); \
		FFINStaticGlobalRegisterFunc RegPropSet([](){ \
			UFINStaticReflectionSource::AddPropSetter(GetUType(), P, &Set); \
		}); \
		void Set(const FFINExecutionContext& Ctx, const FINAny& AnyVal) { \
			T* self = GetFromCtx(Ctx); \
			PT::CppType Val = PT::Get(AnyVal);
#define EndProp() \
		} \
	};

#define FINRefParamLocText(ParamName, KeyName, Value) FINRefLocText(*(FString(TName) + TEXT("_") + FString(FName) + TEXT("_") + TEXT(ParamName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define InVal(Pos, Type, InternalName, DisplayName, Description) \
	Type::CppType InternalName = Type::CppType(); \
	if (!_bGotReg) { UFINStaticReflectionSource::AddFuncParam(GetUType(), F, Pos, FFINStaticFuncParamReg{TEXT(#InternalName), FINRefParamLocText(#InternalName, "DisplayName", DisplayName), FINRefParamLocText(#InternalName, "Description", Description), 0, &Type::PropConstructor});  } \
	else InternalName = Type::Get(Params[Pos]);
#define OutVal(Pos, Type, InternalName, DisplayName, Description) \
	FINAny& InternalName = _bGotReg ? Params[Pos] : *(FINAny*)nullptr; \
	if (!_bGotReg) { UFINStaticReflectionSource::AddFuncParam(GetUType(), F, Pos, FFINStaticFuncParamReg{TEXT(#InternalName), FINRefParamLocText(#InternalName, "DisplayName", DisplayName), FINRefParamLocText(#InternalName, "Description", Description), 1, &Type::PropConstructor}); }
#define RetVal(Pos, Type, InternalName, DisplayName, Description) \
	FINAny& InternalName = _bGotReg ? Params[Pos] : *(FINAny*)nullptr; \
	if (!_bGotReg) { UFINStaticReflectionSource::AddFuncParam(GetUType(), F, Pos, FFINStaticFuncParamReg{TEXT(#InternalName), FINRefParamLocText(#InternalName, "DisplayName", DisplayName), FINRefParamLocText(#InternalName, "Description", Description), 3, &Type::PropConstructor}); }

#define FINRefSignalLocText(KeyName, Value) FINRefLocText(*(FString(TName) + TEXT("_") + FString(SName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define FINRefSignalParamLocText(ParamName, KeyName, Value) FINRefLocText(*(FString(TName) + TEXT("_") + FString(SName) + TEXT("_") + TEXT(ParamName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define SignalClassName(Prop) FIN_StaticRefSignal_ ## Prop
#define BeginSignal(InternalName, DisplayName, Description, ...) \
	namespace SignalClassName(InternalName) { \
		const int S = __COUNTER__; \
		constexpr auto SName = TEXT(#InternalName) ; \
		FFINStaticGlobalRegisterFunc RegSignal([](){ \
			UFINStaticReflectionSource::AddSignal(GetUType(), S, FFINStaticSignalReg{TEXT(#InternalName), FINRefSignalLocText("DisplayName", DisplayName), FINRefSignalLocText("Description", Description), GET_MACRO(0, ##__VA_ARGS__, false)});
#define SignalParam(Pos, Type, InternalName, DisplayName, Description) \
			UFINStaticReflectionSource::AddSignalParam(GetUType(), S, Pos, FFINStaticSignalParamReg{TEXT(#InternalName), FINRefSignalParamLocText(#InternalName, "DisplayName", DisplayName), FINRefSignalParamLocText(#InternalName, "Description", Description), &Type::PropConstructor});
#define EndSignal() \
		}); \
	};

#define Hook(HookClass) \
	FFINStaticGlobalRegisterFunc Hook([](){ \
		AFINHookSubsystem::RegisterHook(GetUType(), HookClass::StaticClass()); \
	});

#define TFS(Str) FText::FromString( Str )

struct RInt {
	typedef FINInt CppType;
	static FINInt Get(const FFINAnyNetworkValue& Any) { return Any.GetInt(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		return NewObject<UFINIntProperty>(Outer);
	}
};

struct RFloat {
	typedef FINFloat CppType;
	static FINFloat Get(const FINAny& Any) { return Any.GetFloat(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		return NewObject<UFINFloatProperty>(Outer);
	}
};

struct RBool {
	typedef FINBool CppType;
	static FINBool Get(const FINAny& Any) { return Any.GetBool(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		return NewObject<UFINBoolProperty>(Outer);
	}
};

struct RString {
	typedef FINStr CppType;
	static FINStr Get(const FINAny& Any) { return Any.GetString(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		return NewObject<UFINStrProperty>(Outer);
	}
};

template<typename T>
struct RClass {
	typedef FINClass CppType;
	static FINClass Get(const FINAny& Any) { return Any.GetClass(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		UFINClassProperty* Prop = NewObject<UFINClassProperty>(Outer);
		Prop->Subclass = T::StaticClass();
		return Prop;
	}
};

template<typename T>
struct RObject {
	typedef TWeakObjectPtr<T> CppType;
	static CppType Get(const FINAny& Any) { return CppType(Cast<T>(Any.GetObj().Get())); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		UFINObjectProperty* Prop = NewObject<UFINObjectProperty>(Outer);
		Prop->Subclass = T::StaticClass();
		return Prop;
	}
};

template<typename T>
struct RTrace {
	typedef FINTrace CppType;
	static FINTrace Get(const FINAny& Any) { return Any.GetTrace(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		UFINTraceProperty* Prop = NewObject<UFINTraceProperty>(Outer);
		Prop->Subclass = T::StaticClass();
		return Prop;
	}
};
 
template<typename T>
struct RStruct {
	typedef T CppType;
	static T Get(const FINAny& Any) { return Any.GetStruct().Get<T>(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		UFINStructProperty* FINProp = NewObject<UFINStructProperty>(Outer);
		FINProp->Struct = TBaseStructure<T>::Get();
		return FINProp;
	}
};

template<typename T>
struct RArray {
	typedef FINArray CppType;
	static FINArray Get(const FINAny& Any) { return Any.GetArray(); }
	static UFINProperty* PropConstructor(UObject* Outer) {
		UFINArrayProperty* FINProp = NewObject<UFINArrayProperty>(Outer);
		FINProp->InnerType = T::PropConstructor(FINProp);
		return FINProp;
	}
};

class FStaticReflectionSourceHelper {
public:
	static void AFGBuildableDoor_Update(AFGBuildableDoor* InDoor, EDoorConfiguration InConfig) {
		InDoor->OnRep_DoorConfiguration();
	}
};

BeginClass(UObject, "Object", "Object", "The base class of every object.")
BeginProp(RInt, hash, "Hash", "A Hash of this object. This is a value that nearly uniquely identifies this object.") {
	Return (int64)GetTypeHash(self);
} EndProp()
BeginProp(RString, internalName, "internalName", "The unreal engine internal name of this object.") {
	Return (FINStr) self->GetName();
} EndProp()
BeginProp(RString, internalPath, "internalPath", "The unreal engine internal path name of this object.") {
	Return (FINStr) self->GetPathName();
} EndProp()
BeginFunc(getHash, "Get Hash", "Returns a hash of this object. This is a value that nearly uniquely identifies this object.") {
	OutVal(0, RInt, hash, "Hash", "The hash of this object.");
	Body()
	hash = (int64)GetTypeHash(self);
} EndFunc()
BeginFunc(getType, "Get Type", "Returns the type (aka class) of this object.") {
	OutVal(0, RObject<UFINClass>, type, "Type", "The type of this object");
	Body()
	if (self) type = (FINObj)FFINReflection::Get()->FindClass(self->GetClass());
} EndFunc()
BeginClassProp(RInt, hash, "Hash", "A Hash of this object. This is a value that nearly uniquely identifies this object.") {
	Return (int64)GetTypeHash(self);
} EndProp()
BeginClassProp(RString, internalName, "internalName", "The unreal engine internal name of this object.") {
	Return (FINStr) self->GetName();
} EndProp()
BeginClassProp(RString, internalPath, "internalPath", "The unreal engine internal path name of this object.") {
	Return (FINStr) self->GetPathName();
} EndProp()
BeginClassFunc(getHash, "Get Hash", "Returns the hash of this class. This is a value that nearly uniquely idenfies this object.", false) {
	OutVal(0, RInt, hash, "Hash", "The hash of this class.");
	Body()
	hash = (int64) GetTypeHash(self);
} EndFunc()
BeginClassFunc(getType, "Get Type", "Returns the type (aka class) of this class instance.", false) {
	OutVal(0, RObject<UFINClass>, type, "Type", "The type of this class instance");
	Body()
    if (self) type = (FINObj)FFINReflection::Get()->FindClass(self);
} EndFunc()
EndClass()

BeginClass(UFINBase, "ReflectionBase", "Reflection Base", "The base class for all things of the reflection system.")
BeginProp(RString, name, "Name", "The internal name.") {
	Return self->GetInternalName();
} EndProp()
BeginProp(RString, displayName, "Display Name", "The display name used in UI which might be localized.") {
	Return self->GetDisplayName().ToString();
} EndProp()
BeginProp(RString, description, "Description", "The description of this base.") {
	Return self->GetDescription().ToString();
} EndProp()
EndClass()

BeginClass(UFINStruct, "Struct", "Struct", "Reflection Object that holds information about structures.")
BeginFunc(getParent, "Get Parent", "Returns the parent type of this type.", false) {
	OutVal(0, RObject<UFINClass>, parent, "Parent", "The parent type of this type.");
	Body()
    if (self) parent = (FINObj)self->GetParent();
} EndFunc()
BeginFunc(getProperties, "Get Properties", "Returns all the properties of this type.") {
	OutVal(0, RArray<RObject<UFINProperty>>, properties, "Properties", "The properties this specific type implements (excluding properties from parent types).")
	Body()
	TArray<FINAny> Props;
	for (UFINProperty* Prop : self->GetProperties(false)) Props.Add((FINObj)Prop);
	properties = Props;
} EndFunc()
BeginFunc(getAllProperties, "Get All Properties", "Returns all the properties of this and parent types.") {
	OutVal(0, RArray<RObject<UFINProperty>>, properties, "Properties", "The properties this type implements including properties from parent types.")
    Body()
    TArray<FINAny> Props;
	for (UFINProperty* Prop : self->GetProperties(true)) Props.Add((FINObj)Prop);
	properties = Props;
} EndFunc()
BeginFunc(getFunctions, "Get Functions", "Returns all the functions of this type.") {
	OutVal(0, RArray<RObject<UFINFunction>>, functions, "Functions", "The functions this specific type implements (excluding properties from parent types).")
    Body()
    TArray<FINAny> Funcs;
	for (UFINFunction* Func : self->GetFunctions(false)) Funcs.Add((FINObj)Func);
	functions = Funcs;
} EndFunc()
BeginFunc(getAllFunctions, "Get All Functions", "Returns all the functions of this and parent types.") {
	OutVal(0, RArray<RObject<UFINProperty>>, functions, "Functions", "The functions this type implements including functions from parent types.")
    Body()
    TArray<FINAny> Funcs;
	for (UFINFunction* Func : self->GetFunctions(true)) Funcs.Add((FINObj)Func);
	functions = Funcs;
} EndFunc()
BeginFunc(isChildOf, "Is Child Of", "Allows to check if this struct is a child struct of the given struct or the given struct it self.") {
	InVal(0, RObject<UFINStruct>, parent, "Parent", "The parent struct you want to check if this struct is a child of.")
    OutVal(1, RBool, isChild, "Is Child", "True if this struct is a child of parent.")
    Body()
    if (self && parent.IsValid()) isChild = self->IsChildOf(Cast<UFINStruct>(parent.Get()));
} EndFunc()
EndClass()

BeginClass(UFINClass, "Class", "Class", "Object that contains all information about a type.")
BeginFunc(getSignals, "Get Signals", "Returns all the signals of this type.") {
	OutVal(0, RArray<RObject<UFINSignal>>, signals, "Signals", "The signals this specific type implements (excluding properties from parent types).")
    Body()
    TArray<FINAny> Sigs;
	for (UFINSignal* Sig : self->GetSignals(false)) Sigs.Add((FINObj)Sig);
	signals = Sigs;
} EndFunc()
BeginFunc(getAllSignals, "Get All Signals", "Returns all the signals of this and its parent types.") {
	OutVal(0, RArray<RObject<UFINSignal>>, signals, "Signals", "The signals this type and all it parents implement.")
    Body()
    TArray<FINAny> Sigs;
	for (UFINSignal* Sig : self->GetSignals(true)) Sigs.Add((FINObj)Sig);
	signals = Sigs;
} EndFunc()
EndClass()

BeginClass(UFINProperty, "Property", "Property", "A Reflection object that holds information about properties and parameters.")
BeginProp(RInt, dataType, "Data Type", "The data type of this property.\n0: nil, 1: bool, 2: int, 3: float, 4: str, 5: object, 6: class, 7: trace, 8: struct, 9: array, 10: anything") {
	Return (FINInt)self->GetType().GetValue();
} EndProp()
BeginProp(RInt, flags, "Flags", "The property bit flag register defining some behaviour of it.\n\nBits and their meaing (least significant bit first):\nIs this property a member attribute.\nIs this property read only.\nIs this property a parameter.\nIs this property a output paramter.\nIs this property a return value.\nCan this property get accessed in syncrounus runtime.\nCan this property can get accessed in parallel runtime.\nCan this property get accessed in asynchronus runtime.\nThis property is a class attribute.") {
	Return (FINInt) self->GetPropertyFlags();
} EndProp()
EndClass()

BeginClass(UFINArrayProperty, "ArrayProperty", "Array Property", "A reflection object representing a array property.")
BeginFunc(getInner, "Get Inner", "Returns the inner type of this array.") {
	OutVal(0, RObject<UFINProperty>, inner, "Inner", "The inner type of this array.")
	Body()
	inner = (FINObj) self->GetInnerType();
} EndFunc()
EndClass()

BeginClass(UFINObjectProperty, "ObjectProperty", "Object Property", "A reflection object representing a object property.")
BeginFunc(getSubclass, "Get Subclass", "Returns the subclass type of this object. Meaning, the stored objects need to be of this type.") {
	OutVal(0, RObject<UFINClass>, subclass, "Subclass", "The subclass of this object.")
    Body()
    subclass = (FINObj) FFINReflection::Get()->FindClass(self->GetSubclass());
} EndFunc()
EndClass()

BeginClass(UFINTraceProperty, "TraceProperty", "Trace Property", "A reflection object representing a trace property.")
BeginFunc(getSubclass, "Get Subclass", "Returns the subclass type of this trace. Meaning, the stored traces need to be of this type.") {
	OutVal(0, RObject<UFINClass>, subclass, "Subclass", "The subclass of this trace.")
    Body()
    subclass = (FINObj) FFINReflection::Get()->FindClass(self->GetSubclass());
} EndFunc()
EndClass()

BeginClass(UFINClassProperty, "ClassProperty", "Class Property", "A reflection object representing a class property.")
BeginFunc(getSubclass, "Get Subclass", "Returns the subclass type of this class. Meaning, the stored classes need to be of this type.") {
	OutVal(0, RObject<UFINClass>, subclass, "Subclass", "The subclass of this class property.")
    Body()
    subclass = (FINObj) FFINReflection::Get()->FindClass(self->GetSubclass());
} EndFunc()
EndClass()

BeginClass(UFINStructProperty, "StructProperty", "Struct Property", "A reflection object representing a struct property.")
BeginFunc(getSubclass, "Get Subclass", "Returns the subclass type of this struct. Meaning, the stored structs need to be of this type.") {
	OutVal(0, RObject<UFINStruct>, subclass, "Subclass", "The subclass of this struct.")
    Body()
    subclass = (FINObj) FFINReflection::Get()->FindStruct(self->GetInner());
} EndFunc()
EndClass()

BeginClass(UFINFunction, "Function", "Function", "A reflection object representing a function.")
BeginFunc(getParameters, "Get Parameters", "Returns all the parameters of this function.") {
	OutVal(0, RArray<RObject<UFINProperty>>, parameters, "Parameters", "The parameters this function.")
    Body()
    TArray<FINAny> ParamArray;
	for (UFINProperty* Param : self->GetParameters()) ParamArray.Add((FINObj)Param);
	parameters = ParamArray;
} EndFunc()
BeginProp(RInt, flags, "Flags", "The function bit flag register defining some behaviour of it.\n\nBits and their meaing (least significant bit first):\nIs this function has a variable amount of input parameters.\nCan this function get called in syncrounus runtime.\nCan this function can get called in parallel runtime.\nCan this function get called in asynchronus runtime.\nIs this function a member function.\nThe function is a class function.\nThe function is a static function.\nThe function has a variable amount of return values.") {
	Return (FINInt) self->GetFunctionFlags();
} EndProp()
EndClass()

BeginClass(UFINSignal, "Signal", "Signal", "A reflection object representing a signal.")
BeginFunc(getParameters, "Get Parameters", "Returns all the parameters of this signal.") {
	OutVal(0, RArray<RObject<UFINProperty>>, parameters, "Parameters", "The parameters this signal.")
    Body()
    TArray<FINAny> ParamArray;
	for (UFINProperty* Param : self->GetParameters()) ParamArray.Add((FINObj)Param);
	parameters = ParamArray;
} EndFunc()
BeginProp(RBool, isVarArgs, "Is VarArgs", "True if this signal has a variable amount of arguments.") {
	Return (FINBool) self->IsVarArgs();
} EndProp()
EndClass()

BeginClass(AActor, "Actor", "Actor", "This is the base class of all things that can exist within the world by them self.")
BeginProp(RStruct<FVector>, location, "Location", "The location of the actor in the world.") {
	Return self->GetActorLocation();
} EndProp()
BeginProp(RStruct<FVector>, scale, "Scale", "The scale of the actor in the world.") {
	Return self->GetActorScale();
} EndProp()
BeginProp(RStruct<FRotator>, rotation, "Rotation", "The rotation of the actor in the world.") {
	Return self->GetActorRotation();
} EndProp()
BeginFunc(getPowerConnectors, "Get Power Connectors", "Returns a list of power connectors this actor might have.") {
	OutVal(0, RArray<RTrace<UFGPowerConnectionComponent>>, connectors, "Connectors", "The power connectors this actor has.");
	Body()
	FINArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFGPowerConnectionComponent::StaticClass())) continue;
		UObject* Connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(Connector))) continue;
		Output.Add(Ctx.GetTrace() / Connector);
	}
	connectors = Output;
} EndFunc()
BeginFunc(getFactoryConnectors, "Get Factory Connectors", "Returns a list of factory connectors this actor might have.") {
	OutVal(0, RArray<RTrace<UFGFactoryConnectionComponent>>, connectors, "Connectors", "The factory connectors this actor has.");
	Body()
	FINArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFGFactoryConnectionComponent::StaticClass())) continue;
		UObject* Connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(Connector))) continue;
		Output.Add(Ctx.GetTrace() / Connector);
	}
	connectors = Output;
} EndFunc()
BeginFunc(getPipeConnectors, "Get Pipe Connectors", "Returns a list of pipe (fluid & hyper) connectors this actor might have.") {
	OutVal(0, RArray<RTrace<UFGPipeConnectionComponentBase>>, connectors, "Connectors", "The pipe connectors this actor has.");
	Body()
	FINArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	TSet<UObject*> Outputted;
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFGPipeConnectionComponentBase::StaticClass())) continue;
		UObject* Connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(Connector))) continue;
		if (Outputted.Contains(Connector)) continue;
		Outputted.Add(Connector);
		Output.Add(Ctx.GetTrace() / Connector);
	}
	connectors = Output;
} EndFunc()
BeginFunc(getInventories, "Get Inventories", "Returns a list of inventories this actor might have.") {
	OutVal(0, RArray<RTrace<UFGInventoryComponent>>, inventories, "Inventories", "The inventories this actor has.");
	Body()
	FINArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFGInventoryComponent::StaticClass())) continue;
		UObject* inventory = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(inventory))) continue;
		Output.Add(Ctx.GetTrace() / inventory);
	}
	inventories = Output;
} EndFunc()
BeginFunc(getNetworkConnectors, "Get Network Connectors", "Returns the name of network connectors this actor might have.") {
	OutVal(0, RArray<RTrace<UFINNetworkConnectionComponent>>, connectors, "Connectors", "The factory connectors this actor has.")
	Body()
	FINArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFINNetworkConnectionComponent::StaticClass())) continue;
		UObject* connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(connector))) continue;
		Output.Add(Ctx.GetTrace() / connector);
	}
	connectors = Output;
} EndFunc()
BeginFunc(getComponents, "Get Components", "Returns the components that make-up this actor.") {
	InVal(0, RClass<UActorComponent>, componentType, "Component Type", "The class will be used as filter.")
	OutVal(1, RArray<RTrace<UActorComponent>>, components, "Components", "The components of this actor.")
	Body()
	FINArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(componentType)) continue;
		UObject* component = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(component))) continue;
		Output.Add(Ctx.GetTrace() / component);
	}
	components = Output;
} EndFunc()
EndClass()

BeginClass(UActorComponent, "ActorComponent", "Actor Component", "A component/part of an actor in the world.")
BeginProp(RTrace<AActor>, owner, "Owner", "The parent actor of which this component is part of") {
	return Ctx.GetTrace() / self->GetOwner();
} EndProp()
EndClass()

BeginClass(UFGInventoryComponent, "Inventory", "Inventory", "A actor component that can hold multiple item stacks.")
BeginFuncVA(getStack, "Get Stack", "Returns the item stack at the given index.\nTakes integers as input and returns the corresponding stacks.") {
	Body()
	if (self->GetOwner()->Implements<UFGReplicationDetailActorOwnerInterface>()) {
		AFGReplicationDetailActor* RepDetailActor = Cast<IFGReplicationDetailActorOwnerInterface>(self->GetOwner())->GetReplicationDetailActor();
		if (RepDetailActor) {
			RepDetailActor->FlushReplicationActorStateToOwner();
		}
	}
	int ArgNum = Params.Num();
	for (int i = 0; i < ArgNum; ++i) {
		const FINAny& Any = Params[i];
		FInventoryStack Stack;
		if (Any.GetType() == FIN_INT && self->GetStackFromIndex(Any.GetInt(), Stack)) { // GetInt realy correct?
			Params.Add(FINAny(Stack));
		} else {
			Params.Add(FINAny());
		}
	}
} EndFunc()
BeginProp(RInt, itemCount, "Item Count", "The absolute amount of items in the whole inventory.") {
	if (self->GetOwner()->Implements<UFGReplicationDetailActorOwnerInterface>()) {
		AFGReplicationDetailActor* RepDetailActor = Cast<IFGReplicationDetailActorOwnerInterface>(self->GetOwner())->GetReplicationDetailActor();
		if (RepDetailActor) {
			RepDetailActor->FlushReplicationActorStateToOwner();
		}
	}
	Return (int64)self->GetNumItems(nullptr);
} EndProp()
BeginProp(RInt, size, "Size", "The count of available item stack slots this inventory has.") {
	if (self->GetOwner()->Implements<UFGReplicationDetailActorOwnerInterface>()) {
		AFGReplicationDetailActor* RepDetailActor = Cast<IFGReplicationDetailActorOwnerInterface>(self->GetOwner())->GetReplicationDetailActor();
		if (RepDetailActor) {
			RepDetailActor->FlushReplicationActorStateToOwner();
		}
	}
	Return (int64)self->GetSizeLinear();
} EndProp()
BeginFunc(sort, "Sort", "Sorts the whole inventory. (like the middle mouse click into a inventory)") {
	Body()
	if (self->GetOwner()->Implements<UFGReplicationDetailActorOwnerInterface>()) {
		AFGReplicationDetailActor* RepDetailActor = Cast<IFGReplicationDetailActorOwnerInterface>(self->GetOwner())->GetReplicationDetailActor();
		if (RepDetailActor) {
			RepDetailActor->FlushReplicationActorStateToOwner();
		}
	}
	if (!self->IsLocked() && self->GetCanBeRearranged()) self->SortInventory();
} EndFunc()
BeginFunc(flush, "Flush", "Removes all discardable items from the inventory completely. They will be gone! No way to get them back!", 0) {
	Body()
	if (self->GetOwner()->Implements<UFGReplicationDetailActorOwnerInterface>()) {
		AFGReplicationDetailActor* RepDetailActor = Cast<IFGReplicationDetailActorOwnerInterface>(self->GetOwner())->GetReplicationDetailActor();
		if (RepDetailActor) {
			RepDetailActor->FlushReplicationActorStateToOwner();
		}
	}
	TArray<FInventoryStack> stacks;
	self->GetInventoryStacks(stacks);
	self->Empty();
	for (const FInventoryStack& stack : stacks) {
		if (stack.HasItems() && stack.Item.IsValid() && !UFGItemDescriptor::CanBeDiscarded(stack.Item.GetItemClass())) {
			self->AddStack(stack);
		}
	}
} EndFunc()
EndClass()

BeginClass(UFGPowerConnectionComponent, "PowerConnection", "Power Connection", "A actor component that allows for a connection point to the power network. Basically a point were a power cable can get attached to.")
BeginProp(RInt, connections, "Connections", "The amount of connections this power connection has.") {
	Return (int64)self->GetNumConnections();
} EndProp()
BeginProp(RInt, maxConnections, "Max Connections", "The maximum amount of connections this power connection can handle.") {
	Return (int64)self->GetMaxNumConnections();
} EndProp()
BeginFunc(getPower, "Get Power", "Returns the power info component of this power connection.") {
	OutVal(0, RTrace<UFGPowerInfoComponent>, power, "Power", "The power info compoent this power connection uses.")
	Body()
	power = Ctx.GetTrace() / self->GetPowerInfo();
} EndFunc();
BeginFunc(getCircuit, "Get Circuit", "Returns the power circuit to which this connection component is attached to.") {
	OutVal(0, RTrace<UFGPowerCircuit>, circuit, "Circuit", "The Power Circuit this connection component is attached to.")
	Body()
	circuit = Ctx.GetTrace() / self->GetPowerCircuit();
} EndFunc()
EndClass()

BeginClass(UFGPowerInfoComponent, "PowerInfo", "Power Info", "A actor component that provides information and mainly statistics about the power connection it is attached to.")
BeginProp(RFloat, dynProduction, "Dynamic Production", "The production cpacity this connection provided last tick.") {
	Return self->GetRegulatedDynamicProduction();
} EndProp()
BeginProp(RFloat, baseProduction, "Base Production", "The base production capactiy this connection always provides.") {
	Return self->GetBaseProduction();
} EndProp()
BeginProp(RFloat, maxDynProduction,	"Max Dynamic Production", "The maximum production capactiy this connection could have provided to the circuit in the last tick.") {
	Return self->GetDynamicProductionCapacity();
} EndProp()
BeginProp(RFloat, targetConsumption, "Target Consumption", "The amount of energy the connection wanted to consume from the circuit in the last tick.") {
	Return self->GetTargetConsumption();
} EndProp()
BeginProp(RFloat, consumption, "Consumption", "The amount of energy the connection actually consumed in the last tick.") {
	Return self->GetBaseProduction();
} EndProp();
BeginProp(RBool, hasPower, "Has Power", "True if the connection has satisfied power values and counts as beeing powered. (True if it has power)") {
	Return self->HasPower();
} EndProp();
BeginFunc(getCircuit, "Get Circuit", "Returns the power circuit this info component is part of.") {
	OutVal(0, RTrace<UFGPowerCircuit>, circuit, "Circuit", "The Power Circuit this info component is attached to.")
	Body()
	circuit = Ctx.GetTrace() / self->GetPowerCircuit();
}
EndFunc()
EndClass()

BeginClass(UFGPowerCircuit, "PowerCircuit", "Power Circuit", "A Object that represents a whole power circuit.")
Hook(UFINPowerCircuitHook)
BeginSignal(PowerFuseChanged, "Power Fuse Changed", "Get Triggered when the fuse state of the power circuit changes.")
EndSignal()
BeginProp(RFloat, production, "Production", "The amount of power produced by the whole circuit in the last tick.") {
	FPowerCircuitStats stats;
	self->GetStats(stats);
	Return stats.PowerProduced;
} EndProp()
BeginProp(RFloat, consumption, "Consumption", "The power consumption of the whole circuit in thge last tick.") {
	FPowerCircuitStats stats;
	self->GetStats(stats);
	Return stats.PowerConsumed;
} EndProp()
BeginProp(RFloat, capacity, "Capacity", "The power capacity of the whole network in the last tick. (The max amount of power available in the last tick)") {
	FPowerCircuitStats stats;
	self->GetStats(stats);
	Return stats.PowerProductionCapacity;
} EndProp()
BeginProp(RFloat, batteryInput, "Battery Input", "The power that gone into batteries in the last tick.") {
	FPowerCircuitStats stats;
	self->GetStats(stats);
	Return stats.BatteryPowerInput;
} EndProp()
BeginProp(RFloat, maxPowerConsumption, "Max Power Consumption", "The maximum consumption of power in the last tick.") {
	FPowerCircuitStats stats;
	self->GetStats(stats);
	Return stats.MaximumPowerConsumption;
} EndProp()
BeginProp(RBool, isFuesed, "Is Fuesed", "True if the fuse in the network triggered.") {
	Return self->IsFuseTriggered();
} EndProp()
BeginProp(RBool, hasBatteries, "Has Batteries", "True if the power circuit has batteries connected to it.") {
	Return self->HasBatteries();
} EndProp()
BeginProp(RFloat, batteryCapacity, "Battery Capacity", "The energy capacity all batteries of the network combined provide.") {
	Return self->GetBatterySumPowerStoreCapacity();
} EndProp()
BeginProp(RFloat, batteryStore, "Battery Store", "The amount of energy currently stored in all battereies of the network combined.") {
	Return self->GetBatterySumPowerStore();
} EndProp()
BeginProp(RFloat, batteryStorePercent, "Battery Store Percentage", "The fill status in percent of all battereies of the network combined.") {
	Return self->GetBatterySumPowerStorePercent();
} EndProp()
BeginProp(RFloat, batteryTimeUntilFull, "Battery Time until Full", "The time in seconds until every battery in the network is filled.") {
	Return self->GetTimeToBatteriesFull();
} EndProp()
BeginProp(RFloat, batteryTimeUntilEmpty, "Battery Time until Empty", "The time in seconds until every battery in the network is empty.") {
	Return self->GetTimeToBatteriesEmpty();
} EndProp()
BeginProp(RFloat, batteryIn, "Battery Input", "The amount of energy that currently gets stored in every battery of the whole network.") {
	Return self->GetBatterySumPowerInput();
} EndProp()
BeginProp(RFloat, batteryOut, "Battery Output", "The amount of energy that currently discharges from every battery in the whole network.") {
	Return self->GetBatterySumPowerOutput();
} EndProp()
EndClass()

BeginClass(AFGBuildablePowerStorage, "PowerStorage", "Power Storage", "A building that can store power for later usage.")
BeginProp(RFloat, powerStore, "Power Store", "The current amount of energy stored in the storage.") {
	Return self->GetPowerStore();
} EndProp()
BeginProp(RFloat, powerCapacity, "Power Capacity", "The amount of energy the storage can hold max.") {
	Return self->GetPowerStoreCapacity();
} EndProp()
BeginProp(RFloat, powerStorePercent, "Power Store Percent", "The current power store in percent.") {
	Return self->GetPowerStorePercent();
} EndProp()
BeginProp(RFloat, powerIn, "Power Input", "The amount of power coming into the storage.") {
	Return self->GetPowerInput();
} EndProp()
BeginProp(RFloat, powerOut, "Power Output", "The amount of power going out from the storage.") {
	Return self->GetPowerOutput();
} EndProp()
BeginProp(RFloat, timeUntilFull, "Time until Full", "The time in seconds until the storage is filled.") {
	Return self->GetTimeUntilFull();
} EndProp()
BeginProp(RFloat, timeUntilEmpty, "Time until Empty", "The time in seconds until the storage is empty.") {
	Return self->GetTimeUntilEmpty();
} EndProp()
BeginProp(RInt, batteryStatus, "Battery Status", "The current status of the battery.\n0 = Idle, 1 = Idle Empty, 2 = Idle Full, 3 = Power In, 4 = Power Out") {
	Return (int64) self->GetBatteryStatus();
} EndProp()
BeginProp(RInt, batteryMaxIndicatorLevel, "Max Indicator Level", "The maximum count of Level lights that are shown.") {
	Return (int64) self->GetIndicatorLevelMax();
} EndProp()
EndClass()

BeginClass(AFGBuildableCircuitBridge, "CircuitBridge", "Circuite Bridget", "A building that can connect two circuit networks together.")
BeginProp(RBool, isBridgeConnected, "Is Bridge Connected", "True if the bridge is connected to two circuits.") {
	Return self->IsBridgeConnected();
} EndProp()
BeginProp(RBool, isBridgeActive, "Is Bridge Active", "True if the two circuits are connected to each other and act as one entity.") {
	Return self->IsBridgeActive();
} EndProp()
EndClass()

BeginClass(AFGBuildableCircuitSwitch, "CircuitSwitch", "Circuit Switch", "A circuit bridge that can be activated and deactivate by the player.")
BeginProp(RBool, isSwitchOn, "Is Switch On", "True if the two circuits are connected to each other and act as one entity.", 0) {
	Return self->IsSwitchOn();
} PropSet() {
	self->SetSwitchOn(Val);
} EndProp()
EndClass()

BeginClass(AFGBuildablePriorityPowerSwitch, "CircuitSwitchPriority", "Circuit Priority Switch", "A circuit power switch that can be activated and deactivated based on a priority to prevent a full factory power shutdown.")
BeginProp(RInt, priority, "Priority", "The priority group of which this switch is part of.") {
	Return (FINInt)self->GetPriority();
} PropSet() {
	self->SetPriority(Val);
} EndProp()
EndClass()

BeginClass(UFGFactoryConnectionComponent, "FactoryConnection", "Factory Connection", "A actor component that is a connection point to which a conveyor or pipe can get attached to.")
Hook(UFINFactoryConnectorHook)
BeginSignal(ItemTransfer, "Item Transfer", "Triggers when the factory connection component transfers an item.")
	SignalParam(0, RStruct<FInventoryItem>, item, "Item", "The transfered item")
EndSignal()
BeginProp(RInt, type, "Type", "Returns the type of the connection. 0 = Conveyor, 1 = Pipe") {
	Return (int64)self->GetConnector();
} EndProp()
BeginProp(RInt, direction, "Direction", "The direction in which the items/fluids flow. 0 = Input, 1 = Output, 2 = Any, 3 = Used just as snap point") {
	Return (int64)self->GetDirection();
} EndProp()
BeginProp(RBool, isConnected, "Is Connected", "True if something is connected to this connection.") {
	Return self->IsConnected();
} EndProp()
BeginFunc(getInventory, "Get Inventory", "Returns the internal inventory of the connection component.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The internal inventory of the connection component.")
	Body()
	inventory = Ctx.GetTrace() / self->GetInventory();
} EndFunc()
BeginFunc(getConnected, "Get Connected", "Returns the connected factory connection component.") {
	OutVal(0, RTrace<UFGInventoryComponent>, connected, "Connected", "The connected factory connection component.")
	Body()
	connected = Ctx.GetTrace() / self->GetConnection();
} EndFunc()
EndClass()

BeginClass(AFGPipeHyperStart, "PipeHyperStart", "Pipe Hyper Start", "A actor that is a hypertube entrance buildable")
Hook(UFINPipeHyperStartHook)
BeginSignal(PlayerEntered, "Player Entered", "Triggers when a players enters into this hypertube entrance.")
	SignalParam(1, RBool, success, "Sucess", "True if the transfer was sucessfull")
EndSignal()
BeginSignal(PlayerExited, "Player Exited", "Triggers when a players leaves through this hypertube entrance.")
EndSignal()
EndClass()

BeginClass(AFGBuildablePipeHyper, "BuildablePipeHyper", "Buildable Pipe Hyper", "A hypertube pipe")
EndClass()

BeginClass(UFGPipeConnectionComponentBase, "PipeConnectionBase", "Pipe Connection Base", "A actor component base that is a connection point to which a pipe for fluid or hyper can get attached to.")
BeginProp(RBool, isConnected, "Is Connected", "True if something is connected to this connection.") {
	Return self->IsConnected();
} EndProp()
BeginFunc(getConnection, "Get Connection", "Returns the connected pipe connection component.") {
	OutVal(0, RTrace<UFGPipeConnectionComponentBase>, connected, "Connected", "The connected pipe connection component.")
	Body()
	connected = Ctx.GetTrace() / self->GetConnection();
} EndFunc()
EndClass()

BeginClass(UFGPipeConnectionComponent, "PipeConnection", "Pipe Connection", "A actor component that is a connection point to which a fluid pipe can get attached to.")
//Hook(UFINFactoryConnectorHook)
BeginProp(RFloat, fluidBoxContent, "Fluid Box Content", "Returns the amount of fluid this fluid container contains") {
	Return self->GetFluidIntegrant()->GetFluidBox()->Content;
} EndProp()
BeginProp(RFloat, fluidBoxHeight, "Fluid Box Height", "Returns the height of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->Height;
} EndProp()
BeginProp(RFloat, fluidBoxLaminarHeight, "Fluid Box Laminar Height", "Returns the laminar height of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->LaminarHeight;
} EndProp()
BeginProp(RFloat, fluidBoxFlowThrough, "Fluid Box Flow Through", "Returns the amount of fluid flowing through this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->FlowThrough;
} EndProp()
BeginProp(RFloat, fluidBoxFlowFill, "Fluid Box Flow Fill", "Returns the fill rate of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->FlowFill;
} EndProp()
BeginProp(RFloat, fluidBoxFlowDrain, "Fluid Box Flow Drain", "Returns the drain rate of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->FlowDrain;
} EndProp()
BeginProp(RFloat, fluidBoxFlowLimit, "Fluid Box Flow Limit", "Returns the the maximum flow limit of this fluid container") {
	Return self->GetFluidIntegrant()->GetFluidBox()->FlowLimit;
} EndProp()
BeginProp(RInt, networkID, "Get Network ID", "Returns the network ID of the pipe network this connection is associated with") {
	Return (int64)self->GetPipeNetworkID();
} EndProp();
BeginFunc(getFluidDescriptor, "Get Fluid Descriptor", "?") {  /* TODO: Write DOC when figured out exactly what it does */
	OutVal(0, RTrace<UFGItemDescriptor>, fluidDescriptor, "Fluid Descriptor", "?")   /* TODO: Write DOC */
	Body()
	fluidDescriptor = Ctx.GetTrace() / self->GetFluidDescriptor();
} EndFunc()
/*BeginFunc(getFluidIntegrant, "Get Fluid Integrant", "?") {  
	OutVal(0, RObject<IFGFluidIntegrantInterface>, fluidIntegrant, "Fluid Descriptor", "?")
    Body()
    fluidIntegrant = Ctx.GetTrace() / self->GetFluidIntegrant();
} EndFunc()*/
BeginFunc(flushPipeNetwork, "Flush Pipe Network", "Flush the associated pipe network") {  
    Body()
	auto networkID = self->GetPipeNetworkID();
    auto subsystem = AFGPipeSubsystem::GetPipeSubsystem(self->GetWorld());
	subsystem->FlushPipeNetwork(networkID);
} EndFunc()
EndClass()

BeginClass(AFGBuildableFactory, "Factory", "Factory", "The base class of most machines you can build.")
BeginProp(RFloat, progress, "Progress", "The current production progress of the current production cycle.") {
	Return self->GetProductionProgress();
} EndProp()
BeginProp(RFloat, powerConsumProducing,	"Producing Power Consumption", "The power consumption when producing.") {
	Return self->GetProducingPowerConsumption();
} EndProp()
BeginProp(RFloat, productivity,	"Productivity", "The productivity of this factory.") {
	Return self->GetProductivity();
} EndProp()
BeginProp(RFloat, cycleTime, "Cycle Time", "The time that passes till one production cycle is finsihed.") {
	Return self->GetProductionCycleTime();
} EndProp()
BeginProp(RFloat, maxPotential, "Max Potential", "The maximum potential this factory can be set to.") {
	Return self->GetMaxPossiblePotential();
} EndProp()
BeginProp(RFloat, minPotential, "Min Potential", "The minimum potential this factory needs to be set to.") {
	Return self->GetMinPotential();
} EndProp()
BeginProp(RBool, standby, "Standby", "True if the factory is in standby.") {
	Return self->IsProductionPaused();
} PropSet() {
	self->SetIsProductionPaused(Val);
} EndProp()
BeginProp(RFloat, potential, "Potential", "The potential this factory is currently set to. (the overclock value)\n 0 = 0%, 1 = 100%") {
	Return self->GetPendingPotential();
} PropSet() {
	float min = self->GetMinPotential();
	float max = self->GetMaxPossiblePotential();
	self->SetPendingPotential(FMath::Clamp((float)Val, self->GetMinPotential(), self->GetMaxPossiblePotential()));
} EndProp()
EndClass()

BeginClass(AFGBuildableManufacturer, "Manufacturer", "Manufacturer", "The base class of every machine that uses a recipe to produce something automatically.")
BeginFunc(getRecipe, "Get Recipe", "Returns the currently set recipe of the manufacturer.") {
	OutVal(0, RClass<UFGRecipe>, recipe, "Recipe", "The currently set recipe.")
	Body()
	recipe = (UClass*)self->GetCurrentRecipe();
} EndFunc()
BeginFunc(getRecipes, "Get Recipes", "Returns the list of recipes this manufacturer can get set to and process.") {
	OutVal(0, RArray<RClass<UFGRecipe>>, recipes, "Recipes", "The list of avalible recipes.")
	Body()
	TArray<FINAny> OutRecipes;
	TArray<TSubclassOf<UFGRecipe>> Recipes;
	self->GetAvailableRecipes(Recipes);
	for (TSubclassOf<UFGRecipe> Recipe : Recipes) {
		OutRecipes.Add((FINAny)(UClass*)Recipe);
	}
	recipes = OutRecipes;
} EndFunc()
BeginFunc(setRecipe, "Set Recipe", "Sets the currently producing recipe of this manufacturer.", 0) {
	InVal(0, RClass<UFGRecipe>, recipe, "Recipe", "The recipe this manufacturer should produce.")
	OutVal(1, RBool, gotSet, "Got Set", "True if the current recipe got successfully set to the new recipe.")
	Body()
	TArray<TSubclassOf<UFGRecipe>> recipes;
	self->GetAvailableRecipes(recipes);
	if (recipes.Contains(recipe)) {
		TArray<FInventoryStack> stacks;
		self->GetInputInventory()->GetInventoryStacks(stacks);
		self->GetOutputInventory()->AddStacks(stacks);
		self->SetRecipe(recipe);
		gotSet = true;
	} else {
		gotSet = false;
	}
} EndFunc()
BeginFunc(getInputInv, "Get Input Inventory", "Returns the input inventory of this manufacturer.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The input inventory of this manufacturer")
	Body()
	inventory = Ctx.GetTrace() / self->GetInputInventory();
} EndFunc()
BeginFunc(getOutputInv, "Get Output Inventory", "Returns the output inventory of this manufacturer.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The output inventory of this manufacturer.")
	Body()
	inventory = Ctx.GetTrace() / self->GetOutputInventory();
} EndFunc()
EndClass()

BeginClass(AFGBuildableDoor, "Door", "Door", "The base class of all doors.")
BeginFunc(getConfiguration, "Get Configuration", "Returns the Door Mode/Configuration.\n0 = Automatic\n1 = Always Closed\n2 = Always Open") {
	OutVal(0, RInt, configuration, "Configuration", "The current door mode/configuration.")
	Body()
	configuration = (FINInt)self->GetmDoorConfiguration();
} EndFunc()
BeginFunc(setConfiguration, "Set Configuration", "Sets the Door Mode/Configuration, only some modes are allowed, if the mod you try to set is invalid, nothing changes.\n0 = Automatic\n1 = Always Closed\n2 = Always Open", 0) {
	InVal(0, RInt, configuration, "Configuration", "The new configuration for the door.")
	Body()
	EDoorConfiguration Config = (EDoorConfiguration)FMath::Clamp(configuration, 0, 2);
	self->SetmDoorConfiguration(Config);
	FStaticReflectionSourceHelper::AFGBuildableDoor_Update(self, Config);
} EndFunc()
EndClass()

BeginClass(AFGVehicle, "Vehicle", "Vehicle", "A base class for all vehciles.")
BeginProp(RFloat, health, "Health", "The health of the vehicle.") {
	Return self->GetHealthComponent()->GetCurrentHealth();
} EndProp()
BeginProp(RFloat, maxHealth, "Max Health", "The maximum amount of health this vehicle can have.") {
	Return self->GetHealthComponent()->GetMaxHealth();
} EndProp()
BeginProp(RBool, isSelfDriving, "Is Self Driving", "True if the vehicle is currently self driving.") {
	Return self->IsSelfDriving();
} PropSet() {
	FReflectionHelper::SetPropertyValue<FBoolProperty>(self, TEXT("mIsSelfDriving"), Val);
} EndProp() 
EndClass()

BeginClass(AFGWheeledVehicle, "WheeledVehicle", "Wheeled Vehicle", "The base class for all vehicles that used wheels for movement.")
BeginFunc(getFuelInv, "Get Fuel Inventory", "Returns the inventory that contains the fuel of the vehicle.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The fuel inventory of the vehicle.")
	Body()
	inventory = Ctx.GetTrace() / self->GetFuelInventory();
} EndFunc()
BeginFunc(getStorageInv, "Get Storage Inventory", "Returns the inventory that contains the storage of the vehicle.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The storage inventory of the vehicle.")
	Body()
	inventory = Ctx.GetTrace() / self->GetStorageInventory();
} EndFunc()
BeginFunc(isValidFuel, "Is Valid Fuel", "Allows to check if the given item type is a valid fuel for this vehicle.") {
	InVal(0, RClass<UFGItemDescriptor>, item, "Item", "The item type you want to check.")
	OutVal(1, RBool, isValid, "Is Valid", "True if the given item type is a valid fuel for this vehicle.")
	Body()
	isValid = self->IsValidFuel(item);
} EndFunc()

BeginFunc(getCurrentTarget, "Get Current Target", "Returns the index of the target that the vehicle tries to move to right now.") {
	OutVal(0, RInt, index, "Index", "The index of the current target.")
	Body()
	AFGDrivingTargetList* List = self->GetTargetList();
	index = (int64)List->FindTargetIndex(List->mCurrentTarget);
} EndFunc()
BeginFunc(nextTarget, "Next Target", "Sets the current target to the next target in the list.") {
	Body()
	self->PickNextTarget();
} EndFunc()
BeginFunc(setCurrentTarget, "Set Current Target", "Sets the target with the given index as the target this vehicle tries to move to right now.") {
	InVal(0, RInt, index, "Index", "The index of the target this vehicle should move to now.")
	Body()
	AFGDrivingTargetList* List = self->GetTargetList();
	AFGTargetPoint* Target = List->FindTargetByIndex(index);
	if (!Target) throw FFINException("index out of range");
	List->mCurrentTarget = Target;
} EndFunc()
BeginFunc(getTargetList, "Get Target List", "Returns the list of targets/path waypoints.") {
	OutVal(0, RTrace<AFGDrivingTargetList>, targetList, "Target List", "The list of targets/path-waypoints.")
	Body()
	targetList = Ctx.GetTrace() / self->GetTargetList();
} EndFunc()
BeginProp(RFloat, speed, "Speed", "The current forward speed of this vehicle.") {
	Return self->GetForwardSpeed();
} EndProp()
BeginProp(RFloat, burnRatio, "Burn Ratio", "The amount of fuel this vehicle burns.") {
	Return self->GetFuelBurnRatio();
} EndProp()
BeginProp(RBool, hasFuel, "Has Fuel", "True if the vehicle has currently fuel to drive.") {
	Return self->HasFuel();
} EndProp()
EndClass()

BeginClass(AFGDrivingTargetList, "TargetList", "Target List", "The list of targets/path-waypoints a autonomous vehicle can drive")
BeginFunc(getTarget, "Get Target", "Returns the target struct at with the given index in the target list.") {
	InVal(0, RInt, index, "Index", "The index of the target you want to get the struct from.")
	OutVal(0, RStruct<FFINTargetPoint>, target, "Target", "The TargetPoint-Struct with the given index in the target list.")
	Body()
	AFGTargetPoint* Target = self->FindTargetByIndex(index);
	if (!Target) throw FFINException("index out of range");
	target = (FINAny)FFINTargetPoint(Target);
} EndFunc()
BeginFunc(removeTarget, "Remove Target", "Removes the target with the given index from the target list.") {
	InVal(0, RInt, index, "Index", "The index of the target point you want to remove from the target list.")
	Body()
	AFGTargetPoint* Target = self->FindTargetByIndex(index);
	if (!Target) throw FFINException( "index out of range");
	self->RemoveItem(Target);
	Target->Destroy();
} EndFunc()
BeginFunc(addTarget, "Add Target", "Adds the given target point struct at the end of the target list.") {
	InVal(0, RStruct<FFINTargetPoint>, target, "Target", "The target point you want to add.")
	Body()
	AFGTargetPoint* Target = target.ToWheeledTargetPoint(self);
	if (!Target) throw FFINException("failed to create target");
	self->InsertItem(Target, self->mLast);
} EndFunc()
BeginFunc(setTarget, "Set Target", "Allows to set the target at the given index to the given target point struct.") {
	InVal(0, RInt, index, "Index", "The index of the target point you want to update with the given target point struct.")
	InVal(1, RStruct<FFINTargetPoint>, target, "Target", "The new target point struct for the given index.")
	Body()
	AFGTargetPoint* Target = self->FindTargetByIndex(index);
	if (!Target) throw FFINException("index out of range");
	Target->SetActorLocation(target.Pos);
	Target->SetActorRotation(target.Rot);
	Target->SetTargetSpeed(target.Speed);
	Target->SetWaitTime(target.Wait);
} EndFunc()
BeginFunc(getTargets, "Get Targets", "Returns a list of target point structs of all the targets in the target point list.") {
	OutVal(0, RArray<RStruct<FFINTargetPoint>>, targets, "Targets", "A list of target point structs containing all the targets of the target point list.")
	Body()
	TArray<FINAny> Targets;
	AFGTargetPoint* CurrentTarget = nullptr;
	int i = 0;
	do {
		if (i++) CurrentTarget = CurrentTarget->GetNext();
		else CurrentTarget = self->GetFirstTarget();
		Targets.Add((FINAny)FFINTargetPoint(CurrentTarget));
	} while (CurrentTarget && CurrentTarget != self->GetLastTarget());
	targets = Targets;
} EndFunc()
BeginFunc(setTargets, "Set Targets", "Removes all targets from the target point list and adds the given array of target point structs to the empty target point list.", 0) {
	InVal(0, RArray<RStruct<FFINTargetPoint>>, targets, "Targets", "A list of target point structs you want to place into the empty target point list.")
	Body()
	int Count = self->GetTargetCount();
	for (const FINAny& Target : targets) {
		self->InsertItem(Target.GetStruct().Get<FFINTargetPoint>().ToWheeledTargetPoint(self), self->mLast);
	}
	for (int i = 0; i < Count; ++i) {
		self->RemoveItem(self->mFirst);
	}
} EndFunc()
EndClass()

BeginClass(AFGBuildableTrainPlatform, "TrainPlatform", "Train Platform", "The base class for all train station parts.")
BeginFunc(getTrackGraph, "Get Track Graph", "Returns the track graph of which this platform is part of.") {
	OutVal(0, RStruct<FFINTrackGraph>, graph, "Graph", "The track graph of which this platform is part of.")
	Body()
	graph = (FINAny)FFINTrackGraph{Ctx.GetTrace(), self->GetTrackGraphID()};
} EndFunc()
BeginFunc(getTrackPos, "Get Track Pos", "Returns the track pos at which this train platform is placed.") {
	OutVal(0, RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
	OutVal(1, RFloat, offset, "Offset", "The offset of the track pos.")
	OutVal(2, RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
	Body()
	FRailroadTrackPosition pos = self->GetTrackPosition();
	if (!pos.IsValid()) throw FFINException("Railroad track position of self is invalid");
	track = Ctx.GetTrace()(pos.Track.Get());
	offset = pos.Offset;
	forward = pos.Forward;
} EndFunc()
BeginFunc(getConnectedPlatform, "Get Connected Platform", "Returns the connected platform in the given direction.") {
	InVal(0, RInt, direction, "Direction", "The direction in which you want to get the connected platform.")
	OutVal(1, RTrace<AFGBuildableTrainPlatform>, platform, "Platform", "The platform connected to this platform in the given direction.")
	Body()
	platform = Ctx.GetTrace() / self->GetConnectedPlatformInDirectionOf(direction);
} EndFunc()
BeginFunc(getDockedVehicle, "Get Docked Vehicle", "Returns the currently docked vehicle.") {
	OutVal(0, RTrace<AFGVehicle>, vehicle, "Vehicle", "The currently docked vehicle")
	Body()
	vehicle = Ctx.GetTrace() / FReflectionHelper::GetPropertyValue<FObjectProperty>(self, TEXT("mDockedRailroadVehicle"));
} EndFunc()
BeginFunc(getMaster, "Get Master", "Returns the master platform of this train station.") {
	OutVal(0, RTrace<AFGRailroadVehicle>, master, "Master", "The master platform of this train station.")
	Body()
	master = Ctx.GetTrace() / FReflectionHelper::GetPropertyValue<FObjectProperty>(self, TEXT("mStationDockingMaster"));
} EndFunc()
BeginFunc(getDockedLocomotive, "Get Docked Locomotive", "Returns the currently docked locomotive at the train station.") {
	OutVal(0, RTrace<AFGLocomotive>, locomotive, "Locomotive", "The currently docked locomotive at the train station.")
	Body()
	locomotive = Ctx.GetTrace() / FReflectionHelper::GetPropertyValue<FObjectProperty>(self, TEXT("mDockingLocomotive"));
} EndFunc()
BeginProp(RInt, status, "Status", "The current docking status of the platform.") {
	Return (int64)self->GetDockingStatus();
} EndProp()
BeginProp(RBool, isReversed, "Is Reversed", "True if the orientation of the platform is reversed relative to the track/station.") {
	Return self->IsOrientationReversed();
} EndProp()
EndClass()

BeginClass(AFGBuildableRailroadStation, "RailroadStation", "Railroad Station", "The train station master platform. This platform holds the name and manages docking of trains.")
BeginProp(RString, name, "Name", "The name of the railroad station.") {
	Return self->GetStationIdentifier()->GetStationName().ToString();
} PropSet() {
	self->GetStationIdentifier()->SetStationName(FText::FromString(Val));
} EndProp()
BeginProp(RInt, dockedOffset, "Docked Offset", "The Offset to the beginning of the station at which trains dock.") {
	Return self->GetDockedVehicleOffset();
} EndProp()
EndClass()

BeginClass(AFGBuildableTrainPlatformCargo, "TrainPlatformCargo", "Train Platform Cargo", "A train platform that allows for loading and unloading cargo cars.")
BeginProp(RBool, isLoading, "Is Loading", "True if the cargo platform is currently loading the docked cargo vehicle.") {
	Return self->GetIsInLoadMode();
} PropSet() {
	self->SetIsInLoadMode(Val);
} EndProp()
BeginProp(RBool, isUnloading, "Is Unloading", "True if the cargo platform is currently loading or unloading the docked cargo vehicle.") {
	Return self->IsLoadUnloading();
} EndProp()
BeginProp(RFloat, dockedOffset, "Docked Offset", "The offset to the track start of the platform at were the vehicle docked.") {
	Return self->GetDockedVehicleOffset();
} EndProp()
BeginProp(RFloat, outputFlow, "Output Flow", "The current output flow rate.") {
	Return self->GetOutflowRate();
} EndProp()
BeginProp(RFloat, inputFlow, "Input Flow", "The current input flow rate.") {
	Return self->GetInflowRate();
} EndProp()
BeginProp(RBool, fullLoad, "Full Load", "True if the docked cargo vehicle is fully loaded.") {
	Return (bool)self->IsFullLoad();
} EndProp()
BeginProp(RBool, fullUnload, "Full Unload", "Ture if the docked cargo vehicle is fully unloaded.") {
	Return (bool)self->IsFullUnload();
} EndProp()
EndClass()

BeginClass(AFGRailroadVehicle, "RailroadVehicle", "Railroad Vehicle", "The base class for any vehicle that drives on train tracks.")
BeginFunc(getTrain, "Get Train", "Returns the train of which this vehicle is part of.") {
	OutVal(0, RTrace<AFGTrain>, train, "Train", "The train of which this vehicle is part of")
	Body()
	train = Ctx.GetTrace() / Cast<UObject>(self->GetTrain());
} EndFunc()
BeginFunc(isCoupled, "Is Coupled", "Allows to check if the given coupler is coupled to another car.") {
	InVal(0, RInt, coupler, "Coupler", "The Coupler you want to check. 0 = Front, 1 = Back")
	OutVal(1, RBool, coupled, "Coupled", "True of the give coupler is coupled to another car.")
	Body()
	coupled = self->IsCoupledAt(static_cast<ERailroadVehicleCoupler>(coupler));
} EndFunc()
BeginFunc(getCoupled, "Get Coupled", "Allows to get the coupled vehicle at the given coupler.") {
	InVal(0, RInt, coupler, "Coupler", "The Coupler you want to get the car from. 0 = Front, 1 = Back")
	OutVal(1, RTrace<AFGRailroadVehicle>, coupled, "Coupled", "The coupled car of the given coupler is coupled to another car.")
	Body()
	coupled = Ctx.GetTrace() / self->GetCoupledVehicleAt(static_cast<ERailroadVehicleCoupler>(coupler));
} EndFunc()
BeginFunc(getTrackGraph, "Get Track Graph", "Returns the track graph of which this vehicle is part of.") {
	OutVal(0, RStruct<FFINTrackGraph>, track, "Track", "The track graph of which this vehicle is part of.")
	Body()
	track = (FINAny)FFINTrackGraph{Ctx.GetTrace(), self->GetTrackGraphID()};
} EndFunc()
BeginFunc(getTrackPos, "Get Track Pos", "Returns the track pos at which this vehicle is.") {
	OutVal(0, RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
    OutVal(1, RFloat, offset, "Offset", "The offset of the track pos.")
    OutVal(2, RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
    Body()
    FRailroadTrackPosition pos = self->GetTrackPosition();
	if (!pos.IsValid()) throw FFINException("Railroad Track Position of self is invalid");
	track = Ctx.GetTrace()(pos.Track.Get());
	offset = pos.Offset;
	forward = pos.Forward;
} EndFunc()
BeginFunc(getMovement, "Get Movement", "Returns the vehicle movement of this vehicle.") {
	OutVal(0, RTrace<UFGRailroadVehicleMovementComponent>, movement, "Movement", "The movement of this vehicle.")
	Body()
	movement = Ctx.GetTrace() / self->GetRailroadVehicleMovementComponent();
} EndFunc()
BeginProp(RFloat, length, "Length", "The length of this vehicle on the track.") {
	Return self->GetLength();
} EndProp()
BeginProp(RBool, isDocked, "Is Docked", "True if this vehicle is currently docked to a platform.") {
	Return self->IsDocked();
} EndProp()
BeginProp(RBool, isReversed, "Is Reversed", "True if the vheicle is placed reversed on the track.") {
	Return self->IsOrientationReversed();
} EndProp()
EndClass()

BeginClass(UFGRailroadVehicleMovementComponent, "RailroadVehicleMovement", "Railroad Vehicle Movement", "This actor component contains all the infomation about the movement of a railroad vehicle.")
BeginFunc(getVehicle, "Get Vehicle", "Returns the vehicle this movement component holds the movement information of.") {
	OutVal(0, RTrace<AFGRailroadVehicle>, vehicle, "Vehicle", "The vehicle this movement component holds the movement information of.")
	Body()
	vehicle = Ctx.GetTrace() / self->GetOwningRailroadVehicle();
} EndFunc()
BeginFunc(getWheelsetRotation, "Get Wheelset Rotation", "Returns the current rotation of the given wheelset.") {
	InVal(0, RInt, wheelset, "Wheelset", "The index of the wheelset you want to get the rotation of.")
	OutVal(1, RFloat, x, "X", "The wheelset's rotation X component.")
	OutVal(2, RFloat, y, "Y", "The wheelset's rotation Y component.")
	OutVal(3, RFloat, z, "Z", "The wheelset's rotation Z component.")
	Body()
	FVector rot = self->GetWheelsetRotation(wheelset);
	x = rot.X;
	y = rot.Y;
	z = rot.Z;
} EndFunc()
BeginFunc(getWheelsetOffset, "Get Wheelset Offset", "Returns the offset of the wheelset with the given index from the start of the vehicle.") {
	InVal(0, RInt, wheelset, "Wheelset", "The index of the wheelset you want to get the offset of.")
	OutVal(1, RFloat, offset, "Offset", "The offset of the wheelset.")
	Body()
	offset = self->GetWheelsetOffset(wheelset);
} EndFunc()
BeginFunc(getCouplerRotationAndExtention, "Get Coupler Rotation And Extention", "Returns the normal vector and the extention of the coupler with the given index.") {
	InVal(0, RInt, coupler, "Coupler", "The index of which you want to get the normal and extention of.")
	OutVal(1, RFloat, x, "X", "The X component of the coupler normal.")
	OutVal(2, RFloat, y, "Y", "The Y component of the coupler normal.")
	OutVal(3, RFloat, z, "Z", "The Z component of the coupler normal.")
	OutVal(4, RFloat, extention, "Extention", "The extention of the coupler.")
	Body()
	float extension;
	FVector rotation = self->GetCouplerRotationAndExtention(coupler, extension);
	x =rotation.X;
	y = rotation.Y;
	z = rotation.Z;
	extention = extension;
} EndFunc()

BeginProp(RFloat, orientation, "Orientation", "The orientation of the vehicle") {
	Return self->GetOrientation();
} EndProp()
BeginProp(RFloat, mass, "Mass", "The current mass of the vehicle.") {
	Return self->GetMass();
} EndProp()
BeginProp(RFloat, tareMass, "Tare Mass", "The tare mass of the vehicle.") {
	Return self->GetTareMass();
} EndProp()
BeginProp(RFloat, payloadMass, "Payload Mass", "The mass of the payload of the vehicle.") {
	Return self->GetPayloadMass();
} EndProp()
BeginProp(RFloat, speed, "Speed", "The current forward speed of the vehicle.") {
	Return self->GetForwardSpeed();
} EndProp()
BeginProp(RFloat, relativeSpeed, "Relative Speed", "The current relative forward speed to the ground.") {
	Return self->GetRelativeForwardSpeed();
} EndProp()
BeginProp(RFloat, maxSpeed, "Max Speed", "The maximum forward speed the vehicle can reach.") {
	Return self->GetMaxForwardSpeed();
} EndProp()
BeginProp(RFloat, gravitationalForce, "Gravitationl Force", "The current gravitational force acting on the vehicle.") {
	Return self->GetGravitationalForce();
} EndProp()
BeginProp(RFloat, tractiveForce, "Tractive Force", "The current tractive force acting on the vehicle.") {
	Return self->GetTractiveForce();
} EndProp()
BeginProp(RFloat, resistiveForce, "Resistive Froce", "The resistive force currently acting on the vehicle.") {
	Return self->GetResistiveForce();
} EndProp()
BeginProp(RFloat, gradientForce, "Gradient Force", "The gradient force currently acting on the vehicle.") {
	Return self->GetGradientForce();
} EndProp()
BeginProp(RFloat, brakingForce, "Braking Force", "The braking force currently acting on the vehicle.") {
	Return self->GetBrakingForce();
} EndProp()
BeginProp(RFloat, airBrakingForce, "Air Braking Force", "The air braking force currently acting on the vehicle.") {
	Return self->GetAirBrakingForce();
} EndProp()
BeginProp(RFloat, dynamicBrakingForce, "Dynamic Braking Force", "The dynamic braking force currently acting on the vehicle.") {
	Return self->GetDynamicBrakingForce();
} EndProp()
BeginProp(RFloat, maxTractiveEffort, "Max Tractive Effort", "The maximum tractive effort of this vehicle.") {
	Return self->GetMaxTractiveEffort();
} EndProp()
BeginProp(RFloat, maxDynamicBrakingEffort, "Max Dynamic Braking Effort", "The maximum dynamic braking effort of this vehicle.") {
	Return self->GetMaxDynamicBrakingEffort();
} EndProp()
BeginProp(RFloat, maxAirBrakingEffort, "Max Air Braking Effort", "The maximum air braking effort of this vehcile.") {
	Return self->GetMaxAirBrakingEffort();
} EndProp()
BeginProp(RFloat, trackGrade, "Track Grade", "The current track grade of this vehicle.") {
	Return self->GetTrackGrade();
} EndProp()
BeginProp(RFloat, trackCurvature, "Track Curvature", "The current track curvature of this vehicle.") {
	Return self->GetTrackCurvature();
} EndProp()
BeginProp(RFloat, wheelsetAngle, "Wheelset Angle", "The wheelset angle of this vehicle.") {
	Return self->GetWheelsetAngle();
} EndProp()
BeginProp(RFloat, rollingResistance, "Rolling Resistance", "The current rolling resistance of this vehicle.") {
	Return self->GetRollingResistance();
} EndProp()
BeginProp(RFloat, curvatureResistance, "Curvature Resistance", "The current curvature resistance of this vehicle.") {
	Return self->GetCurvatureResistance();
} EndProp()
BeginProp(RFloat, airResistance, "Air Resistance", "The current air resistance of this vehicle.") {
	Return self->GetAirResistance();
} EndProp()
BeginProp(RFloat, gradientResistance, "Gradient Resistance", "The current gardient resistance of this vehicle.") {
	Return self->GetGradientResistance();
} EndProp()
BeginProp(RFloat, wheelRotation, "Wheel Rotation", "The current wheel rotation of this vehicle.") {
	Return self->GetWheelRotation();
} EndProp()
BeginProp(RInt, numWheelsets, "Num Wheelsets", "The number of wheelsets this vehicle has.") {
	Return (int64)self->GetNumWheelsets();
} EndProp()
BeginProp(RBool, isMoving, "Is Moving", "True if this vehicle is currently moving.") {
	Return self->IsMoving();
} EndProp()
EndClass()

BeginClass(AFGTrain, "Train", "Train", "This class holds information and references about a trains (a collection of multiple railroad vehicles) and its timetable f.e.")
Hook(UFINTrainHook)
BeginSignal(SelfDrvingUpdate, "Self Drving Update", "Triggers when the self driving mode of the train changes")
	SignalParam(0, RBool, enabled, "Enabled", "True if the train is now self driving.")
EndSignal()
BeginFunc(getName, "Get Name", "Returns the name of this train.") {
	OutVal(0, RString, name, "Name", "The name of this train.")
	Body()
	name = self->GetTrainName().ToString();
} EndFunc()
BeginFunc(setName, "Set Name", "Allows to set the name of this train.") {
	InVal(0, RString, name, "Name", "The new name of this trian.")
	Body()
	self->SetTrainName(FText::FromString(name));
} EndFunc()
BeginFunc(getTrackGraph, "Get Track Graph", "Returns the track graph of which this train is part of.") {
	OutVal(0, RStruct<FFINTrackGraph>, track, "Track", "The track graph of which this train is part of.")
	Body()
	track = (FINAny) FFINTrackGraph{Ctx.GetTrace(), self->GetTrackGraphID()};
} EndFunc()
BeginFunc(setSelfDriving, "Set Self Driving", "Allows to set if the train should be self driving or not.", 0) {
	InVal(0, RBool, selfDriving, "Self Driving", "True if the train should be self driving.")
	Body()
	self->SetSelfDrivingEnabled(selfDriving);
} EndFunc()
BeginFunc(getMaster, "Get Master", "Returns the master locomotive that is part of this train.") {
	OutVal(0, RTrace<AFGLocomotive>, master, "Master", "The master locomotive of this train.")
	Body()
	master = Ctx.GetTrace() / self->GetMultipleUnitMaster();
} EndFunc()
BeginFunc(getTimeTable, "Get Time Table", "Returns the timetable of this train.") {
	OutVal(0, RTrace<AFGRailroadTimeTable>, timeTable, "Time Table", "The timetable of this train.")
	Body()
	timeTable = Ctx.GetTrace() / self->GetTimeTable();
} EndFunc()
BeginFunc(newTimeTable, "New Time Table", "Creates and returns a new timetable for this train.") {
	OutVal(0, RTrace<AFGRailroadTimeTable>, timeTable, "Time Table", "The new timetable for this train.")
	Body()
	timeTable = Ctx.GetTrace() / self->NewTimeTable();
} EndFunc()
BeginFunc(getFirst, "Get First", "Returns the first railroad vehicle that is part of this train.") {
	OutVal(0, RTrace<AFGRailroadVehicle>, first, "First", "The first railroad vehicle that is part of this train.")
	Body()
	first = Ctx.GetTrace() / self->GetFirstVehicle();
} EndFunc()
BeginFunc(getLast, "Get Last", "Returns the last railroad vehicle that is part of this train.") {
	OutVal(0, RTrace<AFGRailroadVehicle>, last, "Last", "The last railroad vehicle that is part of this train.")
	Body()
	last = Ctx.GetTrace() / self->GetLastVehicle();
} EndFunc()
BeginFunc(dock, "Dock", "Trys to dock the train to the station it is currently at.") {
	Body()
	self->Dock();
} EndFunc()
BeginFunc(getVehicles, "Get Vehicles", "Returns a list of all the vehicles this train has.") {
	OutVal(0, RArray<RTrace<AFGRailroadVehicle>>, vehicles, "Vehicles", "A list of all the vehicles this train has.")
	Body()
	TArray<FINAny> Vehicles;
	for (AFGRailroadVehicle* vehicle : self->mSimulationData.SimulatedVehicles) {
		Vehicles.Add(Ctx.GetTrace() / vehicle);
	}
	vehicles = Vehicles;
} EndFunc()
BeginProp(RBool, isPlayerDriven, "Is Player Driven", "True if the train is currently player driven.") {
	Return self->IsPlayerDriven();
} EndProp()
BeginProp(RBool, isSelfDriving, "Is Self Driving", "True if the train is currently self driving.") {
	Return self->IsSelfDrivingEnabled();
} EndProp()
BeginProp(RInt, selfDrivingError, "Self Driving Error", "The last self driving error.\n0 = No Error\n1 = No Power\n2 = No Time Table\n3 = Invalid Next Stop\n4 = Invalid Locomotive Placement\n5 = No Path") {
	Return (int64)self->GetSelfDrivingError();
} EndProp()
BeginProp(RBool, hasTimeTable, "Has Time Table", "True if the train has currently a time table.") {
	Return self->HasTimeTable();
} EndProp()
BeginProp(RInt, dockState, "Dock State", "The current docking state of the train.") {
	Return (int64)self->GetDockingState();
} EndProp()
BeginProp(RBool, isDocked, "Is Docked", "True if the train is currently docked.") {
	Return self->IsDocked();
} EndProp()
EndClass()

BeginClass(AFGRailroadTimeTable, "TimeTable", "Time Table", "Contains the time table information of train.")
BeginFunc(addStop, "Add Stop", "Adds a stop to the time table.") {
	InVal(0, RInt, index, "Index", "The index at which the stop should get added.")
	InVal(1, RTrace<AFGBuildableRailroadStation>, station, "Station", "The railroad station at which the stop should happen.")
	InVal(2, RStruct<FTrainDockingRuleSet>, ruleSet, "Rule Set", "The docking rule set that descibes when the train will depart from the station.")
	OutVal(3, RBool, added, "Added", "True if the stop got sucessfully added to the time table.")
	Body()
	FTimeTableStop stop;
	stop.Station = Cast<AFGBuildableRailroadStation>(station.Get())->GetStationIdentifier();
	stop.DockingRuleSet = ruleSet;
	added = self->AddStop(index, stop);
} EndFunc()
BeginFunc(removeStop, "Remove Stop", "Removes the stop with the given index from the time table.") {
	InVal(0, RInt, index, "Index", "The index at which the stop should get added.")
	Body()
	self->RemoveStop(index);
} EndFunc()
BeginFunc(getStops, "Get Stops", "Returns a list of all the stops this time table has") {
	OutVal(0, RArray<RStruct<FFINTimeTableStop>>, stops, "Stops", "A list of time table stops this time table has.")
	Body()
	TArray<FINAny> Output;
	TArray<FTimeTableStop> Stops;
	self->GetStops(Stops);
	for (const FTimeTableStop& Stop : Stops) {
		Output.Add((FINAny)FFINTimeTableStop{Ctx.GetTrace() / Stop.Station->GetStation(), Stop.DockingRuleSet});
	}
	stops = Output;
} EndFunc()
BeginFunc(setStops, "Set Stops", "Allows to empty and fill the stops of this time table with the given list of new stops.") {
	InVal(0, RArray<RStruct<FFINTimeTableStop>>, stops, "Stops", "The new time table stops.")
	OutVal(0, RBool, gotSet, "Got Set", "True if the stops got sucessfully set.")
	Body()
	TArray<FTimeTableStop> Stops;
	for (const FINAny& Any : stops) {
		Stops.Add(Any.GetStruct().Get<FFINTimeTableStop>());
	}
	gotSet = self->SetStops(Stops);
} EndFunc()
BeginFunc(isValidStop, "Is Valid Stop", "Allows to check if the given stop index is valid.") {
	InVal(0, RInt, index, "Index", "The stop index you want to check its validity.")
	OutVal(1, RBool, valid, "Valid", "True if the stop index is valid.")
	Body()
	valid = self->IsValidStop(index);
} EndFunc()
BeginFunc(getStop, "Get Stop", "Returns the stop at the given index.") {
	InVal(0, RInt, index, "Index", "The index of the stop you want to get.")
	OutVal(1, RStruct<FFINTimeTableStop>, stop, "Stop", "The time table stop at the given index.")
	Body()
	FTimeTableStop Stop = self->GetStop(index);
	if (IsValid(Stop.Station)) {
		stop = (FINAny)FFINTimeTableStop{Ctx.GetTrace() / Stop.Station->GetStation(), Stop.DockingRuleSet};
	} else {
		stop = FINAny();
	}
} EndFunc()
BeginFunc(setCurrentStop, "Set Current Stop", "Sets the stop, to which the train trys to drive to right now.") {
	InVal(0, RInt, index, "Index", "The index of the stop the train should drive to right now.")
	Body()
	self->SetCurrentStop(index);
} EndFunc()
BeginFunc(incrementCurrentStop, "Increment Current Stop", "Sets the current stop to the next stop in the time table.") {
	Body()
	self->IncrementCurrentStop();
} EndFunc()
BeginFunc(getCurrentStop, "Get Current Stop", "Returns the index of the stop the train drives to right now.") {
	OutVal(0, RInt, index, "Index", "The index of the stop the train tries to drive to right now.")
    Body()
    index = (int64) self->GetCurrentStop();
} EndFunc()
BeginProp(RInt, numStops, "Num Stops", "The current number of stops in the time table.") {
	Return (int64)self->GetNumStops();
} EndProp()
EndClass()

BeginClass(AFGBuildableRailroadTrack, "RailroadTrack", "Railroad Track", "A peice of railroad track over which trains can drive.")
BeginFunc(getClosestTrackPosition, "Get Closeset Track Position", "Returns the closes track position from the given world position") {
	InVal(0, RStruct<FVector>, worldPos, "World Pos", "The world position form which you want to get the closest track position.")
	OutVal(1, RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
    OutVal(2, RFloat, offset, "Offset", "The offset of the track pos.")
    OutVal(3, RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
    Body()
	FRailroadTrackPosition pos = self->FindTrackPositionClosestToWorldLocation(worldPos);
	if (!pos.IsValid()) throw FFINException("Railroad Track Position of self is invalid");
	track = Ctx.GetTrace()(pos.Track.Get());
	offset = pos.Offset;
	forward = pos.Forward;
} EndFunc()
BeginFunc(getWorldLocAndRotAtPos, "Get World Location And Rotation At Position", "Returns the world location and world rotation of the track position from the given track position.") {
	InVal(0, RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
    InVal(1, RFloat, offset, "Offset", "The offset of the track pos.")
    InVal(2, RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
    OutVal(3, RStruct<FVector>, location, "Location", "The location at the given track position")
	OutVal(4, RStruct<FVector>, rotation, "Rotation", "The rotation at the given track position (forward vector)")
	Body()
	FRailroadTrackPosition pos(Cast<AFGBuildableRailroadTrack>(track.Get()), offset, forward);
	FVector loc;
	FVector rot;
	self->GetWorldLocationAndDirectionAtPosition(pos, loc, rot);
	location = (FINAny)loc;
	rotation = (FINAny)rot;
} EndFunc()
BeginFunc(getConnection, "Get Connection", "Returns the railroad track connection at the given direction.") {
	InVal(0, RInt, direction, "Direction", "The direction of which you want to get the connector from. 0 = front, 1 = back")
	OutVal(1, RTrace<UFGRailroadTrackConnectionComponent>, connection, "Connection", "The connection component in the given direction.")
	Body()
	connection = Ctx.GetTrace() / self->GetConnection(FMath::Clamp<int>(direction, 0, 1));
} EndFunc()
BeginFunc(getTrackGraph, "Get Track Graph", "Returns the track graph of which this track is part of.") {
	OutVal(0, RStruct<FFINTrackGraph>, track, "Track", "The track graph of which this track is part of.")
    Body()
    track = (FINAny)FFINTrackGraph{Ctx.GetTrace(), self->GetTrackGraphID()};
} EndFunc()
BeginProp(RFloat, length, "Length", "The length of the track.") {
	Return self->GetLength();
} EndProp()
BeginProp(RBool, isOwnedByPlatform, "Is Owned By Platform", "True if the track is part of/owned by a railroad platform.") {
	Return self->GetIsOwnedByPlatform();
} EndProp()
EndClass()

BeginClass(UFGRailroadTrackConnectionComponent, "RailroadTrackConnection", "Railroad Track Connection", "This is a actor component for railroad tracks that allows to connecto to other track connections and so to connection multiple tracks with each eather so you can build a train network.")
BeginProp(RStruct<FVector>, connectorLocation, "Connector Location", "The world location of the the connection.") {
	Return self->GetConnectorLocation();
} EndProp()
BeginProp(RStruct<FVector>, connectorNormal, "Connector Normal", "The normal vecotr of the connector.") {
	Return self->GetConnectorNormal();
} EndProp()
BeginFunc(getConnection, "Get Connection", "Returns the connected connection with the given index.") {
	InVal(1, RInt, index, "Index", "The index of the connected connection you want to get.")
	OutVal(0, RTrace<UFGRailroadTrackConnectionComponent>, connection, "Connection", "The connected connection at the given index.")
	Body()
	connection = Ctx.GetTrace() / self->GetConnection(index);
} EndFunc()
BeginFunc(getConnections, "Get Connections", "Returns a list of all connected connections.") {
	OutVal(0, RArray<RTrace<UFGRailroadTrackConnectionComponent>>, connections, "Connections", "A list of all connected connections.")
	Body()
	TArray<FINAny> Connections;
	for (UFGRailroadTrackConnectionComponent* conn : self->GetConnections()) {
		Connections.Add(Ctx.GetTrace() / conn);
	}
	connections = Connections;
} EndFunc()
BeginFunc(getTrackPos, "Get Track Pos", "Returns the track pos at which this connection is.") {
	OutVal(0, RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track the track pos points to.")
    OutVal(1, RFloat, offset, "Offset", "The offset of the track pos.")
    OutVal(2, RFloat, forward, "Forward", "The forward direction of the track pos. 1 = with the track direction, -1 = against the track direction")
    Body()
    FRailroadTrackPosition pos = self->GetTrackPosition();
	if (!pos.IsValid()) throw FFINException("Railroad Track Position of self is invalid");
	track = Ctx.GetTrace()(pos.Track.Get());
	offset = pos.Offset;
	forward = pos.Forward;
} EndFunc()
BeginFunc(getTrack, "Get Track", "Returns the track of which this connection is part of.") {
	OutVal(0, RTrace<AFGBuildableRailroadTrack>, track, "Track", "The track of which this connection is part of.")
	Body()
	track = Ctx.GetTrace() / self->GetTrack();
} EndFunc()
BeginFunc(getSwitchControl, "Get Switch Control", "Returns the switch control of this connection.") {
	OutVal(0, RTrace<AFGBuildableRailroadSwitchControl>, switchControl, "Switch", "The switch control of this connection.")
	Body()
	switchControl = Ctx.GetTrace() / self->GetSwitchControl();
} EndFunc()
BeginFunc(getStation, "Get Station", "Returns the station of which this connection is part of.") {
	OutVal(0, RTrace<AFGBuildableRailroadStation>, station, "Station", "The station of which this connection is part of.")
	Body()
	station = Ctx.GetTrace() / self->GetStation();
} EndFunc()
BeginFunc(getFacingSignal, "Get Facing Signal", "Returns the signal this connection is facing to.") {
	OutVal(0, RTrace<AFGBuildableRailroadSignal>, signal, "Signal", "The signal this connection is facing.")
	Body()
	signal = Ctx.GetTrace() / self->GetFacingSignal();
} EndFunc()
BeginFunc(getTrailingSignal, "Get Trailing Signal", "Returns the signal this connection is trailing from.") {
	OutVal(0, RTrace<AFGBuildableRailroadSignal>, signal, "Signal", "The signal this connection is trailing.")
	Body()
	signal = Ctx.GetTrace() / self->GetTrailingSignal();
} EndFunc()
BeginFunc(getOpposite, "Get Opposite", "Returns the opposite connection of the track this connection is part of.") {
	OutVal(0, RTrace<UFGRailroadTrackConnectionComponent>, opposite, "Opposite", "The opposite connection of the track this connection is part of.")
	Body()
	opposite = Ctx.GetTrace() / self->GetOpposite();
} EndFunc()
BeginFunc(getNext, "Get Next", "Returns the next connection in the direction of the track. (used the correct path switched point to)") {
	OutVal(0, RTrace<UFGRailroadTrackConnectionComponent>, next, "Next", "The next connection in the direction of the track.")
	Body()
	next = Ctx.GetTrace() / self->GetNext();
} EndFunc()
BeginFunc(setSwitchPosition, "Set Switch Position", "Sets the position (connection index) to which the track switch points to.") {
	InVal(0, RInt, index, "Index", "The connection index to which the switch should point to.")
	Body()
	self->SetSwitchPosition(index);
} EndFunc()
BeginFunc(getSwitchPosition, "Get Switch Position", "Returns the current switch position.") {
	OutVal(0, RInt, index, "Index", "The index of the connection connection the switch currently points to.")
    Body()
    index = (int64)self->GetSwitchPosition();
} EndFunc()
BeginProp(RBool, isConnected, "Is Connected", "True if the connection has any connection to other connections.") {
	Return self->IsConnected();
} EndProp()
BeginProp(RBool, isFacingSwitch, "Is Facing Switch", "True if this connection is pointing to the merge/spread point of the switch.") {
	Return self->IsFacingSwitch();
} EndProp()
BeginProp(RBool, isTrailingSwitch, "Is Trailing Switch", "True if this connection is pointing away from the merge/spread point of a switch.") {
	Return self->IsTrailingSwitch();
} EndProp()
BeginProp(RInt, numSwitchPositions, "Num Switch Positions", "Returns the number of different switch poisitions this switch can have.") {
	Return (int64)self->GetNumSwitchPositions();
} EndProp()
EndClass()

BeginClass(AFGBuildableRailroadSwitchControl, "RailroadSwitchControl", "Railroad Switch Control", "The controler object for a railroad switch.")
BeginFunc(toggleSwitch, "Toggle Switch", "Toggles the railroad switch like if you interact with it.") {
	Body()
	self->ToggleSwitchPosition();
} EndFunc()
BeginFunc(switchPosition, "Switch Position", "Returns the current switch position of this switch.") {
	OutVal(0, RInt, position, "Position", "The current switch position of this switch.")
    Body()
    position = (int64)self->GetSwitchPosition();
} EndFunc()
BeginFunc(getControlledConnection, "Get Controlled Connection", "Returns the Railroad Connection this switch is controlling.") {
	OutVal(0, RTrace<UFGRailroadTrackConnectionComponent>, connection, "Connection", "The controlled connectino.")
	Body()
	connection = Ctx.GetTrace() / self->GetmControlledConnection();
} EndFunc()
EndClass()

BeginClass(AFGBuildableRailroadSignal, "RailroadSignal", "Railroad Signal", "A train signal to control trains on a track.")
Hook(UFINRailroadSignalHook)
BeginSignal(AspectChanged, "Aspect Changed", "Triggers when the aspect of this signal changes.")
	SignalParam(0, RInt, aspect, "Aspect", "The new aspect of the signal (see 'Get Aspect' for more information)")
EndSignal()
BeginProp(RBool, isPathSignal, "Is Path Signal", "True if this signal is a path-signal.") {
	Return self->IsPathSignal();
} EndProp()
BeginProp(RBool, isBiDirectional, "Is Bi-Directional", "True if this signal is bi-directional. (trains can pass into both directions)") {
	Return self->IsBiDirectional();
} EndProp()
BeginProp(RBool, hasObservedBlock, "Has Observed Block", "True if this signal is currently observing at least one block.") {
	Return self->HasObservedBlock();
} EndProp()
BeginProp(RInt, blockValidation, "Block Validation", "Any error states of the block.\n0 = Unknown\n1 = No Error\n2 = No Exit Signal\n3 = Contains Loop\n4 = Contains Mixed Entry Signals") {
	Return (int64)self->GetBlockValidation();
} EndProp()
BeginProp(RInt, aspect, "Aspect", "The aspect of the signal. The aspect shows if a train is allowed to pass (clear) or not and if it should dock.\n0 = Unknown\n1 = The track is clear and the train is allowed to pass.\n2 = The next track is Occupied and the train should stop\n3 = The train should dock.") {
	Return (int64)self->GetAspect();
} EndProp()
BeginFunc(getObservedBlock, "Get Observed Block", "Returns the track block this signals observes.") {
	OutVal(0, RStruct<FFINRailroadSignalBlock>, block, "Block", "The railroad signal block this signal is observing.")
	Body()
	block = FINStruct(FFINRailroadSignalBlock(self->GetObservedBlock()));
} EndFunc()
EndClass()

BeginClass(AFGBuildableResourceSink, "ResourceSink", "Resource Sink", "The resource sink, also known a A.W.E.S.O.M.E Sink")
BeginProp(RInt, numPoints, "Num Points", "The number of available points.") {
	Return (int64)AFGResourceSinkSubsystem::Get(self)->GetNumTotalPoints(EResourceSinkTrack::RST_Default);
} EndProp()
BeginProp(RInt, numCoupons, "Num Coupons", "The number of available coupons to print.") {
	Return (int64)AFGResourceSinkSubsystem::Get(self)->GetNumCoupons();
} EndProp()
BeginProp(RInt, numPointsToNextCoupon, "Num Points To Next Coupon", "The number of needed points for the next coupon.") {
	Return (int64)AFGResourceSinkSubsystem::Get(self)->GetNumPointsToNextCoupon(EResourceSinkTrack::RST_Default);
} EndProp()
BeginProp(RFloat, couponProgress, "Coupon Progress", "The percentage of the progress for the next coupon.") {
	Return AFGResourceSinkSubsystem::Get(self)->GetProgressionTowardsNextCoupon(EResourceSinkTrack::RST_Default);
} EndProp()
EndClass()

BeginClass(AFGBuildableDockingStation, "DockingStation", "Docking Station", "A docking station for wheeled vehicles to transfer cargo.")
BeginFunc(getFuelInv, "Get Fueld Inventory", "Returns the fuel inventory of the docking station.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The fuel inventory of the docking station.")
	Body()
	inventory = Ctx.GetTrace() / self->GetFuelInventory();
} EndFunc()
BeginFunc(getInv, "Get Inventory", "Returns the cargo inventory of the docking staiton.") {
	OutVal(0, RTrace<UFGInventoryComponent>, inventory, "Inventory", "The cargo inventory of this docking station.")
	Body()
	inventory = Ctx.GetTrace() / self->GetInventory();
} EndFunc()
BeginFunc(getDocked, "Get Docked", "Returns the currently docked actor.") {
	OutVal(0, RTrace<AActor>, docked, "Docked", "The currently docked actor.")
	Body()
	docked = Ctx.GetTrace() / self->GetDockedActor();
} EndFunc()
BeginFunc(undock, "Undock", "Undocked the currently docked vehicle from this docking station.") {
	Body()
	self->Undock(true);
} EndFunc()
BeginProp(RBool, isLoadMode, "Is Load Mode", "True if the docking station loads docked vehicles, flase if it unloads them.") {
	Return self->GetIsInLoadMode();
} PropSet() {
	self->SetIsInLoadMode(Val);
} EndProp()
BeginProp(RBool, isLoadUnloading, "Is Load Unloading", "True if the docking station is currently loading or unloading a docked vehicle.") {
	Return self->IsLoadUnloading();
} EndProp()
EndClass()

BeginClass(AFGBuildablePipeReservoir, "PipeReservoir", "Pipe Reservoir", "The base class for all fluid tanks.")
BeginFunc(flush, "Flush", "Emptys the whole fluid container.") {
	Body()
	AFGPipeSubsystem::Get(self->GetWorld())->FlushIntegrant(self);
} EndFunc()
BeginFunc(getFluidType, "Get Fluid Type", "Returns the type of the fluid.") {
	OutVal(0, RClass<UFGItemDescriptor>, type, "Type", "The type of the fluid the tank contains.")
	Body()
	type = (UClass*)self->GetFluidDescriptor();
} EndFunc()
BeginProp(RFloat, fluidContent, "Fluid Content", "The amount of fluid in the tank.") {
	Return self->GetFluidBox()->Content;
} EndProp()
BeginProp(RFloat, maxFluidContent, "Max Fluid Content", "The maximum amount of fluid this tank can hold.") {
	Return self->GetFluidBox()->MaxContent;
} EndProp()
BeginProp(RFloat, flowFill, "Flow Fill", "The currentl inflow rate of fluid.") {
	Return self->GetFluidBox()->FlowFill;
} EndProp()
BeginProp(RFloat, flowDrain, "Float Drain", "The current outflow rate of fluid.") {
	Return self->GetFluidBox()->FlowDrain;
} EndProp()
BeginProp(RFloat, flowLimit, "Flow Limit", "The maximum flow rate of fluid this tank can handle.") {
	Return self->GetFluidBox()->FlowLimit;
} EndProp()
EndClass()

BeginClass(AFGBuildablePipelinePump, "PipelinePump", "PipelinePump", "A building that can pump fluids to a higher level within a pipeline.")
BeginProp(RFloat, maxHeadlift, "Max Headlift", "The maximum amount of headlift this pump can provide.") {
	Return self->GetMaxHeadLift();
} EndProp()
BeginProp(RFloat, designedHeadlift, "Designed Headlift", "The amomunt of headlift this pump is designed for.") {
	Return self->GetDesignHeadLift();
} EndProp()
BeginProp(RFloat, indicatorHeadlift, "Indicator Headlift", "The amount of headlift the indicator shows.") {
	Return self->GetIndicatorHeadLift();
} EndProp()
BeginProp(RFloat, indicatorHeadliftPct, "Indicator Headlift Percent", "The amount of headlift the indicator shows as percantage from max.") {
	Return self->GetIndicatorHeadLiftPct();
} EndProp()
BeginProp(RFloat, userFlowLimit, "User Flow Limit", "The flow limit of this pump the user can specifiy. Use -1 for now user set limit. (in m^3/s)") {
	Return self->GetUserFlowLimit();
} PropSet() {
	self->SetUserFlowLimit(Val);
} EndProp()
BeginProp(RFloat, flowLimit, "Flow Limit", "The overal flow limit of this pump. (in m^3/s)") {
	Return self->GetFlowLimit();
} EndProp()
BeginProp(RFloat, flowLimitPct, "Flow Limit Pct", "The overal flow limit of this pump. (in percent)") {
	Return self->GetFlowLimitPct();
} EndProp()
BeginProp(RFloat, flow, "Flow", "The current flow amount. (in m^3/s)") {
	Return self->GetIndicatorFlow();
} EndProp()
BeginProp(RFloat, flowPct, "Float Pct", "The current flow amount. (in percent)") {
	Return self->GetIndicatorFlowPct();
} EndProp()
EndClass()

BeginClass(AFGBuildableLightSource, "LightSource", "Light Source", "The base class for all light you can build.")
BeginProp(RBool, isLightEnabled, "Is Light Enabled", "True if the light is enabled") {
	return self->IsLightEnabled();
} PropSet() {
	self->SetLightEnabled(Val);
} EndProp()
BeginProp(RBool, isTimeOfDayAware, "Is Time of Day Aware", "True if the light should automatically turn on and off depending on the time of the day.") {
	return self->GetLightControlData().IsTimeOfDayAware;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.IsTimeOfDayAware = Val;
	self->SetLightControlData(data);
} EndProp()
BeginProp(RFloat, intensity, "Intensity", "The intensity of the light.") {
	return self->GetLightControlData().Intensity;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.Intensity = Val;
	self->SetLightControlData(data);
} EndProp()
BeginProp(RInt, colorSlot, "Color Slot", "The color slot the light uses.") {
	return (int64) self->GetLightControlData().ColorSlotIndex;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.ColorSlotIndex = Val;
	self->SetLightControlData(data);
} EndProp()
BeginFunc(getColorFromSlot, "Get Color from Slot", "Returns the light color that is referenced by the given slot.") {
	InVal(0, RInt, slot, "Slot", "The slot you want to get the referencing color from.")
	OutVal(1, RStruct<FLinearColor>, color, "Color", "The color this slot references.")
	Body()
	AFGBuildableSubsystem* SubSys = AFGBuildableSubsystem::Get(self);
	color = (FINStruct) SubSys->GetBuildableLightColorSlot(slot);
} EndFunc()
BeginFunc(setColorFromSlot, "Set Color from Slot", "Allows to update the light color that is referenced by the given slot.", 0) {
	InVal(0, RInt, slot, "Slot", "The slot you want to update the referencing color for.")
	InVal(1, RStruct<FLinearColor>, color, "Color", "The color this slot should now reference.")
	Body()
	AFGBuildableSubsystem* SubSys = AFGBuildableSubsystem::Get(self);
	Cast<AFGGameState>(self->GetWorld()->GetGameState())->Server_SetBuildableLightColorSlot(slot, color);
} EndFunc()
EndClass()

BeginClass(AFGBuildableLightsControlPanel, "LightsControlPanel", "Light Source", "A control panel to configure multiple lights at once.")
BeginProp(RBool, isLightEnabled, "Is Light Enabled", "True if the lights should be enabled") {
	return self->IsLightEnabled();
} PropSet() {
	self->SetLightEnabled(Val);
	for (AFGBuildable* Light : self->GetControlledBuildables(AFGBuildableLightSource::StaticClass())) {
		Cast<AFGBuildableLightSource>(Light)->SetLightEnabled(Val);
	}
} EndProp()
BeginProp(RBool, isTimeOfDayAware, "Is Time of Day Aware", "True if the lights should automatically turn on and off depending on the time of the day.") {
	return self->GetLightControlData().IsTimeOfDayAware;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.IsTimeOfDayAware = Val;
	self->SetLightControlData(data);
	for (AFGBuildable* Light : self->GetControlledBuildables(AFGBuildableLightSource::StaticClass())) {
		Cast<AFGBuildableLightSource>(Light)->SetLightControlData(data);
	}
} EndProp()
BeginProp(RFloat, intensity, "Intensity", "The intensity of the lights.") {
	return self->GetLightControlData().Intensity;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.Intensity = Val;
	self->SetLightControlData(data);
	for (AFGBuildable* Light : self->GetControlledBuildables(AFGBuildableLightSource::StaticClass())) {
		Cast<AFGBuildableLightSource>(Light)->SetLightControlData(data);
	}
} EndProp()
BeginProp(RInt, colorSlot, "Color Slot", "The color slot the lights should use.") {
	return (int64) self->GetLightControlData().ColorSlotIndex;
} PropSet() {
	FLightSourceControlData data = self->GetLightControlData();
	data.ColorSlotIndex = Val;
	self->SetLightControlData(data);
	for (AFGBuildable* Light : self->GetControlledBuildables(AFGBuildableLightSource::StaticClass())) {
		Cast<AFGBuildableLightSource>(Light)->SetLightControlData(data);
	}
} EndProp()
BeginFunc(setColorFromSlot, "Set Color from Slot", "Allows to update the light color that is referenced by the given slot.", 0) {
	InVal(0, RInt, slot, "Slot", "The slot you want to update the referencing color for.")
	InVal(1, RStruct<FLinearColor>, color, "Color", "The color this slot should now reference.")
	Body()
	AFGBuildableSubsystem* SubSys = AFGBuildableSubsystem::Get(self);
	Cast<AFGGameState>(self->GetWorld()->GetGameState())->Server_SetBuildableLightColorSlot(slot, color);
} EndFunc()
EndClass()

BeginClass(AFGBuildableSignBase, "SignBase", "Sign Base", "The base class for all signs in the game.")
BeginFunc(getSignType, "Get Sign Type", "Returns the sign type descriptor") {
	OutVal(0, RClass<UFGSignTypeDescriptor>, descriptor, "Descriptor", "The sign type descriptor")
	Body()
	descriptor = (FINClass)IFGSignInterface::Execute_GetSignTypeDescriptor(self);
} EndFunc()
EndClass()

BeginClass(AFGBuildableWidgetSign, "WidgetSign", "Widget Sign", "The type of sign that allows you to define layouts, images, texts and colors manually.")
BeginFunc(setPrefabSignData, "Set Prefab Sign Data", "Sets the prefabg sign data e.g. the user settings like colo and more to define the signs content.", 0) {
	InVal(0, RStruct<FPrefabSignData>, prefabSignData, "Prefab Sign Data", "The new prefab sign data for this sign.")
	Body()
	self->SetPrefabSignData(prefabSignData);
} EndFunc()
BeginFunc(getPrefabSignData, "Get Prefab Sign Data", "Returns the prefabg sign data e.g. the user settings like colo and more to define the signs content.") {
	OutVal(0, RStruct<FPrefabSignData>, prefabSignData, "Prefab Sign Data", "The new prefab sign data for this sign.")
	Body()
	FPrefabSignData SignData;
	self->GetSignPrefabData(SignData);
	prefabSignData = (FINStruct)SignData;
} EndFunc()
EndClass()

BeginClass(UFGSignTypeDescriptor, "SignType", "Sign Type", "Describes the type of a sign.")
BeginClassProp(RStruct<FVector2D>, dimensions, "Dimensions", "The canvas dimensions of this sign.") {
	FVector2D dimensions;
	UFGSignLibrary::GetCanvasDimensionsFromSignDescriptor(self, dimensions);
	Return dimensions;
} EndProp()
BeginClassFunc(getColors, "Get Colors", "Returns the default foreground/background/auxiliary colors of this sign type.", false) {
	OutVal(0, RStruct<FLinearColor>, foreground, "Foreground", "The foreground color")
	OutVal(1, RStruct<FLinearColor>, background, "Background", "The background color")
	OutVal(2, RStruct<FLinearColor>, auxiliary, "Auxiliary", "The auxiliary color")
	Body()
	FLinearColor fg, bg, au;
	UFGSignLibrary::GetDefaultColorsFromSignDescriptor(self, fg, bg, au);
	foreground = (FINStruct)fg;
	background = (FINStruct)bg;
	auxiliary = (FINStruct)au;
} EndFunc()
BeginClassFunc(getPrefabs, "Get Prefabs", "Returns a list of all sign prefabs this sign can use.", false) {
	OutVal(0, RArray<RClass<UFGSignPrefabWidget>>, prefabs, "Prefabs", "The sign prefabs this sign can use")
	Body()
	TArray<FINAny> PrefabsArray;
	TArray<TSubclassOf<UFGSignPrefabWidget>> PrefabList;
	UFGSignLibrary::GetPrefabLayoutsFromSignDescriptor(self, PrefabList);
	for (TSubclassOf<UFGSignPrefabWidget> Prefab : PrefabList) {
		PrefabsArray.Add((FINClass)Prefab);
	}
	prefabs = PrefabsArray;
} EndFunc()
BeginClassFunc(getTextElements, "Get Text Elements", "Returns a list of element names and their default text values.", false) {
	OutVal(0, RArray<RString>, textElements, "Text Elements", "A list of text element names of this type.")
	OutVal(0, RArray<RString>, textElementsDefaultValues, "Text Elements Default Values", "A list of default values for the text elements of this type.")
	Body()
	TArray<FINAny> TextElements, TextElementsDefaultValues;
	TMap<FString, FString> Elements;
	UFGSignLibrary::GetTextElementNameMapFromSignDescriptor(self, Elements);
	for (const TPair<FString, FString>& Element : Elements) {
		TextElements.Add(Element.Key);
		TextElementsDefaultValues.Add(Element.Value);
	}
	textElements = TextElements;
	textElementsDefaultValues = TextElementsDefaultValues;
} EndFunc()
BeginClassFunc(getIconElements, "Get Icon Elements", "Returns a list of element names and their default icon values.", false) {
	OutVal(0, RArray<RString>, iconElements, "Icon Elements", "A list of icon element names of this type.")
	OutVal(0, RArray<RObject<UTexture2D>>, iconElementsDefaultValues, "Icon Elements Default Values", "A list of default values for the icon elements of this type.")
	Body()
	TArray<FINAny> IconElements, IconElementsDefaultValues;
	TMap<FString, UObject*> Elements;
	UFGSignLibrary::GetIconElementNameMapFromSignDescriptor(self, Elements);
	for (const TPair<FString, UObject*>& Element : Elements) {
		IconElements.Add(Element.Key);
		IconElementsDefaultValues.Add((FINObj)Element.Value);
	}
	iconElements = IconElements;
	iconElementsDefaultValues = IconElementsDefaultValues;
} EndFunc()
EndClass()

BeginClass(UFGSignPrefabWidget, "SignPrefab", "Sign Prefab", "Descibes a layout of a sign.")
EndClass()

BeginStruct(FPrefabSignData, "PrefabSignData", "Prefab Sign Data", "This structure stores all data that defines what a sign displays.")
BeginProp(RClass<UObject>, layout, "Layout", "The object that actually displayes the layout") {
	Return (FINClass)self->PrefabLayout;
} PropSet() {
	self->PrefabLayout = Val;
} EndProp()
BeginProp(RStruct<FLinearColor>, foreground, "Foreground", "The foreground Color.") {
	Return (FINStruct)self->ForegroundColor;
} PropSet() {
	self->ForegroundColor = Val;
} EndProp()
BeginProp(RStruct<FLinearColor>, background, "bBckground", "The background Color.") {
	Return (FINStruct)self->BackgroundColor;
} PropSet() {
	self->BackgroundColor = Val;
} EndProp()
BeginProp(RStruct<FLinearColor>, auxiliary, "Auxiliary", "The auxiliary Color.") {
	Return (FINStruct)self->AuxiliaryColor;
} PropSet() {
	self->AuxiliaryColor = Val;
} EndProp()
BeginProp(RClass<UFGSignTypeDescriptor>, signType, "Sign Type", "The type of sign this prefab fits to.") {
	Return (FINClass)self->SignTypeDesc;
} PropSet() {
	self->SignTypeDesc = Val;
} EndProp()
BeginFunc(getTextElements, "Get Text Elements", "Returns all text elements and their values.") {
	OutVal(0, RArray<RString>, textElements, "Text Elements", "The element names for all text elements.")
	OutVal(1, RArray<RString>, textElementValues, "Text Element Values", "The values for all text elements.")
	Body()
	TArray<FINAny> TextElements, TextElementValues;
	for (const TPair<FString, FString>& Element : self->TextElementData) {
		TextElements.Add(Element.Key);
		TextElementValues.Add(Element.Value);
	}
	textElements = (FINArray)TextElements;
	textElementValues = (FINArray)TextElementValues;
} EndFunc()
BeginFunc(getIconElements, "Get Icon Elements", "Returns all icon elements and their values.") {
	OutVal(0, RArray<RString>, iconElements, "Icon Elements", "The element names for all icon elements.")
	OutVal(1, RArray<RInt>, iconElementValues, "Icon Element Values", "The values for all icon elements.")
	Body()
	TArray<FINAny> IconElements, IconElementValues;
	for (const TPair<FString, int32>& Element : self->IconElementData) {
		IconElements.Add(Element.Key);
		IconElementValues.Add((FINInt)Element.Value);
	}
	iconElements = IconElements;
	iconElementValues = IconElementValues;
} EndFunc()
BeginFunc(setTextElements, "Set Text Elements", "Sets all text elements and their values.") {
	InVal(0, RArray<RString>, textElements, "Text Elements", "The element names for all text elements.")
	InVal(1, RArray<RString>, textElementValues, "Text Element Values", "The values for all text elements.")
	Body()
	if (textElements.Num() != textElementValues.Num()) throw FFINException(TEXT("Count of element names and element values are not the same."));
	self->TextElementData.Empty();
	for (int i = 0; i < textElements.Num(); ++i) {
		self->TextElementData.Add(textElements[i].GetString(), textElementValues[i].GetString());
	}
} EndFunc()
BeginFunc(setIconElements, "Set Icon Elements", "Sets all icon elements and their values.") {
	InVal(0, RArray<RString>, iconElements, "Icon Elements", "The element names for all icon elements.")
	InVal(1, RArray<RInt>, iconElementValues, "Icon Element Values", "The values for all icon elements.")
	Body()
	if (iconElements.Num() != iconElementValues.Num()) throw FFINException(TEXT("Count of element names and element values are not the same."));
	self->IconElementData.Empty();
	for (int i = 0; i < iconElements.Num(); ++i) {
		self->IconElementData.Add(iconElements[i].GetString(), iconElementValues[i].GetInt());
	}
} EndFunc()
BeginFunc(setTextElement, "Set Text Element", "Sets a text element with the given element name.") {
	InVal(0, RString, elementName, "Element Name", "The name of the text element")
	InVal(1, RString, value, "Value", "The value of the text element")
	Body()
	self->TextElementData.Add(elementName, value);
} EndFunc()
BeginFunc(setIconElement, "Set Icon Element", "Sets a icon element with the given element name.") {
	InVal(0, RString, elementName, "Element Name", "The name of the icon element")
	InVal(1, RInt, value, "Value", "The value of the icon element")
	Body()
	self->IconElementData.Add(elementName, value);
} EndFunc()
BeginFunc(getTextElement, "Get Text Element", "Gets a text element with the given element name.") {
	InVal(0, RString, elementName, "Element Name", "The name of the text element")
	OutVal(1, RInt, value, "Value", "The value of the text element")
	Body()
	FString* Element = self->TextElementData.Find(elementName);
	if (!Element) throw FFINException(TEXT("No element with the given name found"));
	value = *Element;
} EndFunc()
BeginFunc(getIconElement, "Get Icon Element", "Gets a icon element with the given element name.") {
	InVal(0, RString, elementName, "Element Name", "The name of the icon element")
	OutVal(1, RInt, value, "Value", "The value of the icon element")
	Body()
	int* Element = self->IconElementData.Find(elementName);
	if (!Element) throw FFINException(TEXT("No element with the given name found"));
	value = (FINInt)*Element;
} EndFunc()
EndStruct()

BeginClass(UFGRecipe, "Recipe", "Recipe", "A struct that holds information about a recipe in its class. Means don't use it as object, use it as class type!")
BeginClassProp(RString, name, "Name", "The name of this recipe.") {
	Return (FINStr)UFGRecipe::GetRecipeName(self).ToString();
} EndProp()
BeginClassProp(RFloat, duration, "Duration", "The duration how much time it takes to cycle the recipe once.") {
	Return UFGRecipe::GetManufacturingDuration(self);
} EndProp()
BeginClassFunc(getProducts, "Get Products", "Returns a array of item amounts, this recipe returns (outputs) when the recipe is processed once.", false) {
	OutVal(0, RArray<RStruct<FItemAmount>>, products, "Products", "The products of this recipe.")
	Body()
	TArray<FINAny> Products;
	for (const FItemAmount& Product : UFGRecipe::GetProducts(self)) {
		Products.Add((FINAny)Product);
	}
	products = Products;
} EndFunc()
BeginClassFunc(getIngredients, "Get Ingredients", "Returns a array of item amounts, this recipe needs (input) so the recipe can be processed.", false) {
	OutVal(0, RArray<RStruct<FItemAmount>>, ingredients, "Ingredients", "The ingredients of this recipe.")
	Body()
	TArray<FINAny> Ingredients;
	for (const FItemAmount& Ingredient : UFGRecipe::GetIngredients(self)) {
		Ingredients.Add((FINAny)Ingredient);
	}
	ingredients = Ingredients;
} EndFunc()
EndClass()

BeginClass(UFGItemDescriptor, "ItemType", "Item Type", "The type of an item (iron plate, iron rod, leaves)")
BeginClassProp(RInt, form, "Form", "The matter state of this resource.\n1: Solid\n2: Liquid\n3: Gas\n4: Heat") {
	Return (FINInt)UFGItemDescriptor::GetForm(self);
} EndProp()
BeginClassProp(RFloat, energy, "Energy", "How much energy this resource provides if used as fuel.") {
	Return (FINFloat)UFGItemDescriptor::GetForm(self);
} EndProp()
BeginClassProp(RFloat, radioactiveDecay, "Radioactive Decay", "The amount of radiation this item radiates.") {
	Return (FINFloat)UFGItemDescriptor::GetForm(self);
} EndProp()
BeginClassProp(RString, name, "Name", "The name of the item.") {
	Return (FINStr)UFGItemDescriptor::GetItemName(self).ToString();
} EndProp()
BeginClassProp(RString, description, "Description", "The description of this item.") {
	Return (FINStr)UFGItemDescriptor::GetItemDescription(self).ToString();
} EndProp()
BeginClassProp(RInt, max, "Max", "The maximum stack size of this item.") {
	Return (FINInt)UFGItemDescriptor::GetStackSize(self);
} EndProp()
BeginClassProp(RBool, canBeDiscarded, "Can be Discarded", "True if this item can be discarded.") {
	Return (FINBool)UFGItemDescriptor::CanBeDiscarded(self);
} EndProp()
BeginClassProp(RClass<UFGItemCategory>, category, "Category", "The category in which this item is in.") {
	Return (FINClass)UFGItemDescriptor::GetCategory(self);
} EndProp()
BeginClassProp(RStruct<FLinearColor>, fluidColor, "Fluid Color", "The color of this fluid.") {
	Return (FINStruct)(FLinearColor)UFGItemDescriptor::GetFluidColor(self);
} EndProp()
EndClass()

BeginClass(UFGItemCategory, "ItemCategory", "Item Category", "The category of some items.")
BeginClassProp(RString, name, "Name", "The name of the category.") {
	Return (FINStr)UFGItemCategory::GetCategoryName(self).ToString();
} EndProp()
EndClass()

BeginStruct(FFINFuture, "Future", "Future", "A Future struct MAY BE HANDLED BY CPU IMPLEMENTATION differently, generaly, this is used to make resources available on a later point in time. Like if data won't be avaialble right away and you have to wait for it to process first. Like when you do a HTTP Request, then it takes some time to get the data from the web server. And since we don't want to halt the game and wait for the data, you can use a future to check if the data is available, or let just the Lua Code wait, till the data becomes available.")
EndStruct()

BeginStruct(FVector2D, "Vector2D", "Vector 2D", "Contains two cordinates (X, Y) to describe a position or movement vector in 2D Space")
BeginProp(RFloat, x, "X", "The X coordinate component") {
	Return self->X;
} PropSet() {
	self->X = Val;
} EndProp()
BeginProp(RFloat, y, "Y", "The Y coordinate component") {
	Return self->Y;
} PropSet() {
	self->Y = Val;
} EndProp()
EndStruct()

BeginStruct(FVector, "Vector", "Vector", "Contains three cordinates (X, Y, Z) to describe a position or movement vector in 3D Space")
BeginProp(RFloat, x, "X", "The X coordinate component") {
	Return self->X;
} PropSet() {
	self->X = Val;
} EndProp()
BeginProp(RFloat, y, "Y", "The Y coordinate component") {
	Return self->Y;
} PropSet() {
	self->Y = Val;
} EndProp()
BeginProp(RFloat, z, "Z", "The Z coordinate component") {
	Return self->Z;
} PropSet() {
	self->Z = Val;
} EndProp()
EndStruct()

BeginStruct(FRotator, "Rotator", "Rotator", "Contains rotation information about a object in 3D spaces using 3 rotation axis in a gimble.")
BeginProp(RFloat, pitch, "Pitch", "The pitch component") {
	Return self->Pitch;
} PropSet() {
	self->Pitch = Val;
} EndProp()
BeginProp(RFloat, yaw, "Yaw", "The yaw component") {
	Return self->Yaw;
} PropSet() {
	self->Yaw = Val;
} EndProp()
BeginProp(RFloat, roll, "Roll", "The roll component") {
	Return self->Roll;
} PropSet() {
	self->Roll = Val;
} EndProp()
EndStruct()

BeginStruct(FFINTimeTableStop, "TimeTableStop", "Time Table Stop", "Information about a train stop in a time table.")
BeginProp(RTrace<AFGBuildableRailroadStation>, station, "Station", "The station at which the train should stop") {
	Return self->Station;
} PropSet() {
	self->Station = Val;
} EndProp()
BeginFunc(getRuleSet, "Get Rule Set", "Returns The rule set wich describe when the train will depart from the train station.") {
	OutVal(0, RStruct<FTrainDockingRuleSet>, ruleset, "Rule Set", "The rule set of this time table stop.")
	Body()
	ruleset = FINStruct(self->RuleSet);
} EndFunc()
BeginFunc(setRuleSet, "Set Rule Set", "Allows you to change the Rule Set of this time table stop.") {
	InVal(1, RStruct<FTrainDockingRuleSet>, ruleset, "Rule Set", "The rule set you want to use instead.")
	Body()
	self->RuleSet = ruleset;
} EndFunc()
EndStruct()

BeginStruct(FTrainDockingRuleSet, "TrainDockingRuleSet", "Train Docking Rule Set", "Contains infromation about the rules that descibe when a trian should depart from a station")
BeginProp(RInt, definition, "Defintion", "0 = Load/Unload Once, 1 = Fully Load/Unload") {
	Return (FINInt)self->DockingDefinition;
} PropSet() {
	self->DockingDefinition = (ETrainDockingDefinition)Val;
} EndProp()
BeginProp(RFloat, duration, "Duration", "The amount of time the train will dock at least.") {
	Return self->DockForDuration;
} PropSet() {
	self->DockForDuration = Val;
} EndProp()
BeginProp(RBool, isDurationAndRule, "Is Duration and Rule", "True if the duration of the train stop and the other rules have to be applied.") {
	Return self->IsDurationAndRule;
} PropSet() {
	self->IsDurationAndRule = Val;
} EndProp()
BeginProp(RArray<RClass<UFGItemDescriptor>>, loadFilters, "Load Filters", "The types of items that will be loaded.") {
	TArray<FINAny> Filters;
	for (TSubclassOf<UFGItemDescriptor> Filter : self->LoadFilterDescriptors) {
		Filters.Add((FINClass)Filter);
	}
	Return Filters;
} PropSet() {
	TArray<TSubclassOf<UFGItemDescriptor>> Filters;
	for (const FINAny& Filter : Val) {
		Filters.Add(Filter.GetClass());
	}
	self->LoadFilterDescriptors = Filters;
} EndProp()
BeginProp(RArray<RClass<UFGItemDescriptor>>, unloadFilters, "Unload Filters", "The types of items that will be unloaded.") {
	TArray<FINAny> Filters;
	for (TSubclassOf<UFGItemDescriptor> Filter : self->UnloadFilterDescriptors) {
		Filters.Add((FINClass)Filter);
	}
	Return Filters;
} PropSet() {
	TArray<TSubclassOf<UFGItemDescriptor>> Filters;
	for (const FINAny& Filter : Val) {
		Filters.Add(Filter.GetClass());
	}
	self->UnloadFilterDescriptors = Filters;
} EndProp()
EndStruct()

BeginStruct(FFINTrackGraph, "TrackGraph", "Track Graph", "Struct that holds a cache of a whole train/rail network.")
BeginFunc(getTrains, "Get Trains", "Returns a list of all trains in the network.") {
	OutVal(0, RArray<RTrace<AFGTrain>>, trains, "Trains", "The list of trains in the network.")
	Body()
	TArray<FINAny> Trains;
	TArray<AFGTrain*> TrainList;
	AFGRailroadSubsystem::Get(*self->Trace)->GetTrains(self->TrackID, TrainList);
	for (AFGTrain* Train : TrainList) {
		Trains.Add(self->Trace / Train);
	}
	trains = Trains;
} EndFunc()
BeginFunc(getStations, "Get Stations", "Returns a list of all trainstations in the network.") {
	OutVal(0, RArray<RTrace<AFGBuildableRailroadStation>>, stations, "Stations", "The list of trainstations in the network.")
    Body()
    TArray<FINAny> Stations;
	TArray<AFGTrainStationIdentifier*> StationList;
	AFGRailroadSubsystem::Get(*self->Trace)->GetTrainStations(self->TrackID, StationList);
	for (const auto& Station : StationList) {
		Stations.Add(self->Trace / Station->mStation);
	}
	stations = Stations;
} EndFunc()
EndStruct()

BeginStruct(FFINRailroadSignalBlock, "RailroadSignalBlock", "Railroad Signal Block", "A track section that combines the area between multiple signals.")
BeginProp(RBool, isValid, "Is Valid", "Is true if this signal block reference is valid.") {
	Return self->Block.IsValid();
} EndProp()
BeginProp(RBool, isBlockOccupied, "Is Block Occupied", "True if the block this signal is observing is currently occupied by a vehicle.") {
	if (!self->Block.IsValid()) throw FFINException(TEXT("The block reference is invalid"));
	Return self->Block.Pin()->IsOccupied();
} EndProp()
BeginProp(RBool, hasBlockReservation, "Has Block Reservation", "True if the block this signal is observing has a reservation of a train e.g. will be passed by a train soon.") {
	if (!self->Block.IsValid()) throw FFINException(TEXT("The signal is not observing any block"));
	Return self->Block.Pin()->HaveReservations();
} EndProp()
BeginProp(RBool, isPathBlock, "Is Path Block", "True if the block this signal is observing is a path-block.") {
	if (!self->Block.IsValid()) throw FFINException(TEXT("The signal is not observing any block"));
	Return self->Block.Pin()->IsPathBlock();
} PropSet() {
	if (!self->Block.IsValid()) throw FFINException(TEXT("The signal is not observing any block"));
	self->Block.Pin()->SetIsPathBlock(Val);
} EndProp()
BeginProp(RInt, blockValidation, "Block Validation", "Returns the blocks validation status.") {
	if (!self->Block.IsValid()) throw FFINException(TEXT("The signal is not observing any block"));
	Return (int64)self->Block.Pin()->GetBlockValidation();
} EndProp()
BeginFunc(isOccupiedBy, "Is Occupied By", "Allows you to check if this block is occupied by a given train.") {
	InVal(0, RObject<AFGTrain>, train, "Train", "The train you want to check if it occupies this block")
	OutVal(1, RBool, isOccupied, "Is Occupied", "True if the given train occupies this block.")
	Body()
	if (!self->Block.IsValid()) throw FFINException(TEXT("The signal is not observing any block"));
	isOccupied = self->Block.Pin()->IsOccupiedBy(train.Get());
} EndFunc()
EndStruct()

BeginStruct(FFINTargetPoint, "TargetPoint", "Target Point", "Target Point in the waypoint list of a wheeled vehicle.")
BeginProp(RStruct<FVector>, pos, "Pos", "The position of the target point in the world.") {
	Return self->Pos;
} PropSet() {
	self->Pos = Val;
} EndProp()
BeginProp(RStruct<FRotator>, rot, "Rot", "The rotation of the target point in the world.") {
	Return self->Rot;
} PropSet() {
	self->Rot = Val;
} EndProp()
BeginProp(RFloat, speed, "Speed", "The speed at which the vehicle should pass the target point.") {
	Return self->Speed;
} PropSet() {
	self->Speed = Val;
} EndProp()
BeginProp(RFloat, wait, "Wait", "The amount of time which needs to pass till the vehicle will continue to the next target point.") {
	Return self->Wait;
} PropSet() {
	self->Wait = Val;
} EndProp()
EndStruct()

BeginStruct(FItemAmount, "ItemAmount", "Item Amount", "A struct that holds a pair of amount and item type.")
BeginProp(RInt, amount, "Amount", "The amount of items.") {
	Return (int64) self->Amount;
} PropSet() {
	self->Amount = Val;
} EndProp()
BeginProp(RClass<UFGItemDescriptor>, type, "Type", "The type of the items.") {
	Return (UClass*)self->ItemClass;
} PropSet() {
	self->ItemClass = Val;
} EndProp()
EndStruct()

BeginStruct(FInventoryStack, "ItemStack", "Item Stack", "A structure that holds item information and item amount to represent an item stack.")
BeginProp(RInt, count, "Count", "The count of items.") {
	Return (int64) self->NumItems;
} PropSet() {
	self->NumItems = Val;
} EndProp()
BeginProp(RStruct<FInventoryItem>, item, "Item", "The item information of this stack.") {
	Return self->Item;
} PropSet() {
	self->Item = Val;
} EndProp()
EndStruct()

BeginStruct(FInventoryItem, "Item", "Item", "A structure that holds item information.")
BeginProp(RClass<UFGItemDescriptor>, type, "Type", "The type of the item.") {
	Return (UClass*)self->GetItemClass();
} PropSet() {
	self->SetItemClass(Val);
} EndProp()
EndStruct()

BeginStruct(FLinearColor, "Color", "Color", "A structure that holds a rgba color value")
BeginProp(RFloat, r, "Red", "The red portion of the color.") {
	Return (FINFloat) self->R;
} PropSet() {
	self->R = Val;
} EndProp()
BeginProp(RFloat, g, "Green", "The green portion of the color.") {
	Return (FINFloat) self->G;
} PropSet() {
	self->G = Val;
} EndProp()
BeginProp(RFloat, b, "Blue", "The blue portion of the color.") {
	Return (FINFloat) self->B;
} PropSet() {
	self->B = Val;
} EndProp()
BeginProp(RFloat, a, "Alpha", "The alpha (opacity) portion of the color.") {
	Return (FINFloat) self->A;
} PropSet() {
	self->A = Val;
} EndProp()
EndStruct()

BeginStruct(FFINGPUT1Buffer, "GPUT1Buffer", "GPU T1 Buffer", "A structure that can hold a buffer of characters and colors that can be displayed with a gpu")
BeginFunc(getSize, "Get Size", "Allows to get the dimensions of the buffer.", 2) {
	OutVal(0, RFloat, width, "Width", "The width of this buffer")
	OutVal(1, RFloat, height, "Height", "The height of this buffer")
	Body()
	int Width, Height;
	self->GetSize(Width, Height);
	width = (FINInt)Width;
	height = (FINInt)Height;
} EndFunc()
BeginFunc(setSize, "Set Size", "Allows to set the dimensions of the buffer.", 2) {
	InVal(0, RFloat, width, "Width", "The width this buffer should now have")
	InVal(1, RFloat, height, "Height", "The height this buffer now have")
	Body()
	self->SetSize(width, height);
} EndFunc()
BeginFunc(get, "Get", "Allows to get a single pixel from the buffer at the given position", 2) {
	InVal(0, RInt, x, "X", "The x position of the character you want to get")
	InVal(1, RInt, y, "Y", "The y position of the character you want to get")
	OutVal(2, RString, c, "Char", "The character at the given position")
	OutVal(3, RStruct<FLinearColor>, foreground, "Foreground Color", "The foreground color of the pixel at the given position")
	OutVal(4, RStruct<FLinearColor>, background, "Background Color", "The background color of the pixel at the given position")
	Body()
	const FFINGPUT1BufferPixel& Pixel = self->Get(x, y);
	c = FString::Chr(Pixel.Character);
	foreground = (FINStruct) Pixel.ForegroundColor;
	background = (FINStruct) Pixel.BackgroundColor;
} EndFunc()
BeginFunc(set, "Set", "Allows to set a single pixel of the buffer at the given position", 2) {
	InVal(0, RInt, x, "X", "The x position of the character you want to set")
	InVal(1, RInt, y, "Y", "The y position of the character you want to set")
	InVal(2, RString, c, "Char", "The character the pixel should have")
	InVal(3, RStruct<FLinearColor>, foreground, "Foreground Color", "The foreground color the pixel at the given position should have")
	InVal(4, RStruct<FLinearColor>, background, "Background Color", "The background color the pixel at the given position should have")
	OutVal(5, RBool, done, "Done", "True if the pixel got set successfully")
	Body()
	if (c.Len() < 1) return;
	done = self->Set(x, y, FFINGPUT1BufferPixel(c[0], foreground, background));
} EndFunc()
BeginFunc(copy, "Copy", "Copies the given buffer at the given offset of the upper left corner into this buffer.", 2) {
	InVal(0, RInt, x, "X", "The x offset of the upper left corner of the buffer relative to this buffer")
	InVal(1, RInt, y, "Y", "The y offset of the upper left corener of the buffer relative to this buffer")
	InVal(2, RStruct<FFINGPUT1Buffer>, buffer, "Buffer", "The buffer from wich you want to copy from")
	InVal(3, RInt, textBlendMode, "Text Blend Mode", "The blend mode that is used for the text.\n0 = Overwrite this with the content of the given buffer\n1 = Overwrite with only characters that are not ' '\n2 = Overwrite only were this characters are ' '\n3 = Keep this buffer")
	InVal(4, RInt, foregroundBlendMode, "Foreground Color Blend Mode", "The blend mode that is used for the foreground color.\n0 = Overwrite with the given color\n1 = Normal alpha composition\n2 = Multiply\n3 = Divide\n4 = Addition\n5 = Subtraction\n6 = Difference\n7 = Darken Only\n8 = Lighten Only\n9 = None")
	InVal(5, RInt, backgroundBlendMode, "Background Color Blend Mode", "The blend mode that is used for the background color.\n0 = Overwrite with the given color\n1 = Normal alpha composition\n2 = Multiply\n3 = Divide\n4 = Addition\n5 = Subtraction\n6 = Difference\n7 = Darken Only\n8 = Lighten Only\n9 = None")
	Body()
	self->Copy(x, y, buffer, (EFINGPUT1TextBlendingMethod)textBlendMode, (EFINGPUT1ColorBlendingMethod)foregroundBlendMode, (EFINGPUT1ColorBlendingMethod)backgroundBlendMode);
} EndFunc()
BeginFunc(setText, "Set Text", "Allows to write the given text onto the buffer and with the given offset.", 2) {
	InVal(0, RInt, x, "X", "The X Position at which the text should begin to get written.")
	InVal(1, RInt, y, "Y", "The Y Position at which the text should begin to get written.")
	InVal(2, RString, text, "Text", "The text that should get written.")
	InVal(3, RStruct<FLinearColor>, foreground, "Foreground", "The foreground color which will be used to write the text.")
	InVal(4, RStruct<FLinearColor>, background, "Background", "The background color which will be used to write the text.")
	Body()
	self->SetText(x, y, text, foreground, background);
} EndFunc()
BeginFunc(fill, "Fill", "Draws the given character at all given positions in the given rectangle on-to the hidden screen buffer.", 2) {
	InVal(0, RInt, x, "X", "The x coordinate at which the rectangle should get drawn. (upper-left corner)")
	InVal(1, RInt, y, "Y", "The y coordinate at which the rectangle should get drawn. (upper-left corner)")
	InVal(2, RInt, width, "Width", "The width of the rectangle.")
	InVal(3, RInt, height, "Height", "The height of the rectangle.")
	InVal(4, RString, character, "Character", "A string with a single character that will be used for each pixel in the range you want to fill.")
	InVal(5, RStruct<FLinearColor>, foreground, "Foreground", "The foreground color which will be used to fill the rectangle.")
	InVal(6, RStruct<FLinearColor>, background, "Background", "The background color which will be used to fill the rectangle.")
	Body()
	if (character.Len() < 1) character = " ";
	self->Fill(x, y, width, height, FFINGPUT1BufferPixel(character[0], foreground, background));
} EndFunc()
BeginFunc(setRaw, "Set Raw", "Allows to set the internal data of the buffer more directly.", 2) {
	InVal(0, RString, characters, "Characters", "The characters you want to draw with a length of exactly width*height.")
	InVal(1, RArray<RFloat>, foreground, "Foreground Color", "The values of the foreground color slots for each character were a group of four values give one color. so the length has to be exactly width*height*4.")
	InVal(2, RArray<RFloat>, background, "Background Color", "The values of the background color slots for each character were a group of four values give one color. so the length has to be exactly width*height*4.")
	OutVal(3, RBool, success, "Success", "True if the raw data was successfully written")
	Body()
	TArray<float> Foreground, Background;
	if (foreground.Num() != background.Num()) success = false;
	else {
		//Foreground.AddUninitialized(foreground.Num());
		//Background.AddUninitialized(background.Num());
		//ParallelFor(foreground.Num(), [&Foreground, &foreground, &Background, &background](int i) {
		for (int i = 0; i < foreground.Num(); ++i) {
			Foreground.Add(foreground[i].GetFloat());
			Background.Add(background[i].GetFloat());
		//});
		}
		int Width, Height;
		self->GetSize(Width, Height);
		const int Length = Width * Height;
		if (characters.Len() != Length
			|| foreground.Num() != Length*4
			|| background.Num() != Length*4) {
			success = false;
		}
		ParallelFor(Length, [self, Width, &characters, &foreground, &background](int i) {
			int Offset = i * 4;
			const FLinearColor ForegroundColor(
				foreground[Offset].GetFloat(),
				foreground[Offset+1].GetFloat(),
				foreground[Offset+2].GetFloat(),
				foreground[Offset+3].GetFloat());
			const FLinearColor BackgroundColor(
				background[Offset].GetFloat(),
				background[Offset+1].GetFloat(),
				background[Offset+2].GetFloat(),
				background[Offset+3].GetFloat());
			self->Set(i % Width, i / Width, FFINGPUT1BufferPixel(characters[i], ForegroundColor, BackgroundColor));
		});
		success = true;
	}
} EndFunc()
BeginFunc(clone, "Clone", "Clones this buffer into a new struct") {
	OutVal(1, RStruct<FFINGPUT1Buffer>, buffer, "Buffer", "The clone of this buffer")
	Body()
	buffer = (FINStruct) *self;
} EndFunc()
EndStruct()