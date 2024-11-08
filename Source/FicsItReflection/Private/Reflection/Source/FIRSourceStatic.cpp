#include "Reflection/Source/FIRSourceStatic.h"

#include "FGBuildableDoor.h"
#include "Reflection/FIRArrayProperty.h"
#include "Reflection/FIRBoolProperty.h"
#include "Reflection/FIRClassProperty.h"
#include "Reflection/FIRFloatProperty.h"
#include "Reflection/FIRFuncProperty.h"
#include "Reflection/FIRIntProperty.h"
#include "Reflection/FIRObjectProperty.h"
#include "Reflection/FIRStrProperty.h"
#include "Reflection/FIRStructProperty.h"
#include "Reflection/FIRTraceProperty.h"
#include "Reflection/ReflectionHelper.h"

#include "FGFactoryConnectionComponent.h"
#include "FGPipeConnectionComponent.h"
#include "FGGameState.h"
#include "FGHealthComponent.h"
#include "FGIconLibrary.h"
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
#include "Buildables/FGBuildableTrainPlatform.h"
#include "Buildables/FGBuildableTrainPlatformCargo.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "WheeledVehicles/FGTargetPointLinkedList.h"
#include "WheeledVehicles/FGWheeledVehicle.h"

TMap<UClass*, FFIRStaticClassReg> UFIRSourceStatic::Classes;
TMap<UScriptStruct*, FFIRStaticStructReg> UFIRSourceStatic::Structs;

bool UFIRSourceStatic::ProvidesRequirements(UClass* Class) const {
	return Classes.Contains(Class);
}

bool UFIRSourceStatic::ProvidesRequirements(UScriptStruct* Struct) const {
	return Structs.Contains(Struct);
}

void UFIRSourceStatic::FillData(FFicsItReflectionModule* Ref, UFIRClass* ToFillClass, UClass* Class) const {
	const FFIRStaticClassReg* ClassReg = Classes.Find(Class);
	if (!ClassReg) return;
	ToFillClass->InternalName = ClassReg->InternalName;
	ToFillClass->DisplayName = ClassReg->DisplayName;
	ToFillClass->Description = ClassReg->Description;
	ToFillClass->Parent = Ref->FindClass(Class->GetSuperClass());
	ToFillClass->StructFlags |= FIR_Struct_StaticSource;
	if (ToFillClass->Parent == ToFillClass) ToFillClass->Parent = nullptr;

	for (const TPair<int, FFIRStaticFuncReg>& KVFunc : ClassReg->Functions) {
		const FFIRStaticFuncReg& Func = KVFunc.Value;
		UFIRFunction* FIRFunc = NewObject<UFIRFunction>(ToFillClass);
		FIRFunc->InternalName = Func.InternalName;
		FIRFunc->DisplayName = Func.DisplayName;
		FIRFunc->Description = Func.Description;
		FIRFunc->FunctionFlags |= FIR_Func_StaticSource;
		if (Func.VarArgs) FIRFunc->FunctionFlags = FIRFunc->FunctionFlags | FIR_Func_VarArgs;
		switch (Func.Runtime) {
		case 0:
			FIRFunc->FunctionFlags = (FIRFunc->FunctionFlags & ~FIR_Func_Runtime) | FIR_Func_Sync;
			break;
		case 1:
			FIRFunc->FunctionFlags = (FIRFunc->FunctionFlags & ~FIR_Func_Runtime) | FIR_Func_Parallel;
			break;
		case 2:
			FIRFunc->FunctionFlags = (FIRFunc->FunctionFlags & ~FIR_Func_Runtime) | FIR_Func_Async;
			break;
		default:
			break;
		}
		switch (Func.FuncType) {
		case 1:
			FIRFunc->FunctionFlags = FIRFunc->FunctionFlags | FIR_Func_ClassFunc;
			break;
		case 2:
			FIRFunc->FunctionFlags = FIRFunc->FunctionFlags | FIR_Func_StaticFunc;
			break;
		default:
			FIRFunc->FunctionFlags = FIRFunc->FunctionFlags | FIR_Func_MemberFunc;
			break;
		}

		TArray<int> ParamPos;
		Func.Parameters.GetKeys(ParamPos);
		ParamPos.Sort();
		for (int Pos : ParamPos) {
			const FFIRStaticFuncParamReg& Param = Func.Parameters[Pos];
			UFIRProperty* FIRProp = Param.PropConstructor(FIRFunc);
			FIRProp->InternalName = Param.InternalName;
			FIRProp->DisplayName = Param.DisplayName;
			FIRProp->Description = Param.Description;
			FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_Param | FIR_Prop_StaticSource;
			switch (Param.ParamType) {
				case 2:
					FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_RetVal;
				case 1:
					FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_OutParam;
					break;
				default: break;
			}
			FIRFunc->Parameters.Add(FIRProp);
		}

		auto NFunc = Func.Function;
		FIRFunc->NativeFunction = [Func](const FFIRExecutionContext& Ctx, const TArray<FIRAny>& InValues) -> TArray<FIRAny> {
			TArray<FIRAny> Parameters;
			TArray<int> Pos;
			Func.Parameters.GetKeys(Pos);
			Pos.Sort();
			int j = 0;
			if (Pos.Num() > 0) for (int i = 0; i <= Pos[Pos.Num()-1]; ++i) {
				const FFIRStaticFuncParamReg* Reg = Func.Parameters.Find(i);
				if (Reg && Reg->ParamType == 0) {
					Parameters.Add(InValues[j++]);
				} else {
					Parameters.Add(FIRAny());
				}
			}
			for (; j < InValues.Num(); j++) Parameters.Add(InValues[j]);
			Func.Function(Ctx, Parameters);

			TArray<FIRAny> OutValues;
			if (Pos.Num() > 0) for (int i = 0; i <= Pos[Pos.Num()-1]; ++i) {
				const FFIRStaticFuncParamReg* Reg = Func.Parameters.Find(i);
				if (Reg && Reg->ParamType > 0) {
					OutValues.Add(Parameters[i]);
					j++;
				}
			}
			for (; j < Parameters.Num();) OutValues.Add(Parameters[j++]);
			return OutValues;
		};

		ToFillClass->Functions.Add(FIRFunc);
	}

	for (const TPair<int, FFIRStaticPropReg>& KVProp : ClassReg->Properties) {
		const FFIRStaticPropReg& Prop = KVProp.Value;
		UFIRProperty* FIRProp = Prop.PropConstructor(ToFillClass);
		FIRProp->InternalName = Prop.InternalName;
		FIRProp->DisplayName = Prop.DisplayName;
		FIRProp->Description = Prop.Description;
		FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_Attrib | FIR_Prop_StaticSource;
		if (UFIRFuncProperty* FIRFuncProp = Cast<UFIRFuncProperty>(FIRProp)) {
			FIRFuncProp->GetterFunc.GetterFunc = Prop.Get;
			if ((bool)Prop.Set) FIRFuncProp->SetterFunc.SetterFunc = Prop.Set;
			else FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_ReadOnly;
		}
		switch (Prop.Runtime) {
		case 0:
			FIRProp->PropertyFlags = (FIRProp->PropertyFlags & ~FIR_Prop_Runtime) | FIR_Prop_Sync;
			break;
		case 1:
			FIRProp->PropertyFlags = (FIRProp->PropertyFlags & ~FIR_Prop_Runtime) | FIR_Prop_Parallel;
			break;
		case 2:
			FIRProp->PropertyFlags = (FIRProp->PropertyFlags & ~FIR_Prop_Runtime) | FIR_Prop_Async;
			break;
		default:
			break;
		}
		switch (Prop.PropType) {
		case 1:
			FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_ClassProp;
			break;
		case 2:
			FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_StaticProp;
			break;
		default:
			break;
		}
		ToFillClass->Properties.Add(FIRProp);
	}

	for (const TPair<int, FFIRStaticSignalReg>& KVSignal : ClassReg->Signals) {
		const FFIRStaticSignalReg& Signal = KVSignal.Value;
		UFIRSignal* FIRSignal = NewObject<UFIRSignal>(ToFillClass);
		FIRSignal->InternalName = Signal.InternalName;
		FIRSignal->DisplayName = Signal.DisplayName;
		FIRSignal->Description = Signal.Description;
		FIRSignal->bIsVarArgs = Signal.bIsVarArgs;
		FIRSignal->SignalFlags = FIR_Signal_StaticSource;

		TArray<int> ParamPos;
		Signal.Parameters.GetKeys(ParamPos);
		ParamPos.Sort();
		for (int Pos : ParamPos) {
			const FFIRStaticSignalParamReg& Param = Signal.Parameters[Pos];
			UFIRProperty* FIRProp = Param.PropConstructor(FIRSignal);
			FIRProp->InternalName = Param.InternalName;
			FIRProp->DisplayName = Param.DisplayName;
			FIRProp->Description = Param.Description;
			FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_Param;
			FIRSignal->Parameters.Add(FIRProp);
		}
		ToFillClass->Signals.Add(FIRSignal);
	}
}

void UFIRSourceStatic::FillData(FFicsItReflectionModule* Ref, UFIRStruct* ToFillStruct, UScriptStruct* Struct) const {
	const FFIRStaticStructReg* StructReg = Structs.Find(Struct);
	if (!StructReg) return;
	ToFillStruct->InternalName = StructReg->InternalName;
	ToFillStruct->DisplayName = StructReg->DisplayName;
	ToFillStruct->Description = StructReg->Description;
	ToFillStruct->Parent = Ref->FindStruct(Cast<UScriptStruct>(Struct->GetSuperStruct()));
	ToFillStruct->StructFlags |= FIR_Struct_StaticSource;
	if (StructReg->bConstructable) ToFillStruct->StructFlags |= FIR_Struct_Constructable;
	if (ToFillStruct->Parent == ToFillStruct) ToFillStruct->Parent = nullptr;

	for (const TPair<int, FFIRStaticFuncReg>& KVFunc : StructReg->Functions) {
		const FFIRStaticFuncReg& Func = KVFunc.Value;
		UFIRFunction* FIRFunc = NewObject<UFIRFunction>(ToFillStruct);
		FIRFunc->InternalName = Func.InternalName;
		FIRFunc->DisplayName = Func.DisplayName;
		FIRFunc->Description = Func.Description;
		FIRFunc->FunctionFlags |= FIR_Func_StaticSource;
		if (Func.VarArgs) FIRFunc->FunctionFlags = FIRFunc->FunctionFlags | FIR_Func_VarArgs;
		switch (Func.Runtime) {
		case 0:
			FIRFunc->FunctionFlags = (FIRFunc->FunctionFlags & ~FIR_Func_Runtime) | FIR_Func_Sync;
			break;
		case 1:
			FIRFunc->FunctionFlags = (FIRFunc->FunctionFlags & ~FIR_Func_Runtime) | FIR_Func_Parallel;
			break;
		case 2:
			FIRFunc->FunctionFlags = (FIRFunc->FunctionFlags & ~FIR_Func_Runtime) | FIR_Func_Async;
			break;
		default:
			break;
		}
		switch (Func.FuncType) {
		case 1:
			FIRFunc->FunctionFlags = FIRFunc->FunctionFlags | FIR_Func_ClassFunc;
			break;
		case 2:
			FIRFunc->FunctionFlags = FIRFunc->FunctionFlags | FIR_Func_StaticFunc;
			break;
		default:
			FIRFunc->FunctionFlags = FIRFunc->FunctionFlags | FIR_Func_MemberFunc;
			break;
		}

		TArray<int> ParamPos;
		Func.Parameters.GetKeys(ParamPos);
		ParamPos.Sort();
		for (int Pos : ParamPos) {
			const FFIRStaticFuncParamReg& Param = Func.Parameters[Pos];
			UFIRProperty* FIRProp = Param.PropConstructor(FIRFunc);
			FIRProp->InternalName = Param.InternalName;
			FIRProp->DisplayName = Param.DisplayName;
			FIRProp->Description = Param.Description;
			FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_Param | FIR_Prop_StaticSource;
			switch (Param.ParamType) {
				case 2:
					FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_RetVal;
				case 1:
					FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_OutParam;
					break;
				default: break;
			}
			FIRFunc->Parameters.Add(FIRProp);
		}

		auto NFunc = Func.Function;
		FIRFunc->NativeFunction = [Func](const FFIRExecutionContext& Ctx, const TArray<FIRAny>& Params) -> TArray<FIRAny> {
			TArray<FIRAny> Parameters;
			TArray<int> Pos;
			Func.Parameters.GetKeys(Pos);
			Pos.Sort();
			int j = 0;
			if (Pos.Num() > 0) for (int i = 0; i <= Pos[Pos.Num()-1]; ++i) {
				const FFIRStaticFuncParamReg* Reg = Func.Parameters.Find(i);
				if (Reg && Reg->ParamType == 0) {
					Parameters.Add(Params[j++]);
				} else {
					Parameters.Add(FIRAny());
				}
			}
			for (; j < Params.Num(); ++j) {
				Parameters.Add(Params[j]);
			}
			Func.Function(Ctx, Parameters);

			TArray<FIRAny> OutValues;
			if (Pos.Num() > 0) for (int i = 0; i <= Pos[Pos.Num()-1]; ++i) {
				const FFIRStaticFuncParamReg* Reg = Func.Parameters.Find(i);
				if (Reg && Reg->ParamType > 0) {
					OutValues.Add(Parameters[i]);
				}
			}
			return OutValues;
		};

		ToFillStruct->Functions.Add(FIRFunc);
	}

	for (const TPair<int, FFIRStaticPropReg>& KVProp : StructReg->Properties) {
		const FFIRStaticPropReg& Prop = KVProp.Value;
		UFIRProperty* FIRProp = Prop.PropConstructor(ToFillStruct);
		FIRProp->InternalName = Prop.InternalName;
		FIRProp->DisplayName = Prop.DisplayName;
		FIRProp->Description = Prop.Description;
		FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_Attrib | FIR_Prop_StaticSource;
		if (UFIRFuncProperty* FIRFuncProp = Cast<UFIRFuncProperty>(FIRProp)) {
			FIRFuncProp->GetterFunc.GetterFunc = Prop.Get;
			if ((bool)Prop.Set) FIRFuncProp->SetterFunc.SetterFunc = Prop.Set;
			else FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_ReadOnly;
		}
		switch (Prop.Runtime) {
		case 0:
			FIRProp->PropertyFlags = (FIRProp->PropertyFlags & ~FIR_Prop_Runtime) | FIR_Prop_Sync;
			break;
		case 1:
			FIRProp->PropertyFlags = (FIRProp->PropertyFlags & ~FIR_Prop_Runtime) | FIR_Prop_Parallel;
			break;
		case 2:
			FIRProp->PropertyFlags = (FIRProp->PropertyFlags & ~FIR_Prop_Runtime) | FIR_Prop_Async;
			break;
		default:
			break;
		}
		switch (Prop.PropType) {
		case 1:
			FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_ClassProp;
			break;
		case 2:
			FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_StaticProp;
			break;
		default:
			break;
		}
		ToFillStruct->Properties.Add(FIRProp);
	}
}
