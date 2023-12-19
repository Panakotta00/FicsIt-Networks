#include "Reflection/FINUReflectionSource.h"
#include "FicsItNetworksModule.h"
#include "Buildables/FGBuildable.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINReflectionUtils.h"
#include "Reflection/FINUFunction.h"
#include "UObject/PropertyIterator.h"
#include "Utils/FINUtils.h"

TMap<UFunction*, UFINSignal*> UFINUReflectionSource::FuncSignalMap;

template<typename T3, typename T1, typename T2, typename ProjectionType>
void FieldMapToMetaMap(TMap<FString, T1>& To, ProjectionType Projection, const TMap<FString, T2>& From) {
	for (const TPair<FString, T2>& Entry : From) {
		Invoke(Projection, To.FindOrAdd(Entry.Key)) = T3(Entry.Value);
	}
}

template<typename T1, typename T2, typename ProjectionType>
void FieldArrayToMetaArray(TArray<T1>& To, ProjectionType Projection, const TArray<T2>& From) {
	int Num = From.Num() - To.Num();
	if (Num > 0) To.AddDefaulted(Num);
	for (int i = 0; i < From.Num(); ++i) {
		Invoke(Projection, To[i]) = From[i];
	}
}

bool UFINUReflectionSource::ProvidesRequirements(UClass* Class) const {
	if (Class->IsChildOf(AFGBuildable::StaticClass())) return true;
	for (TFieldIterator<FProperty> Property(Class); Property; ++Property) {
		if (Property->GetName().StartsWith("netProp_")) return true;
		if (Property->GetName().StartsWith("netPropReadOnly_")) return true;
	}
	for (TFieldIterator<UFunction> Function(Class); Function; ++Function) {
		if (Function->GetName().StartsWith("netPropGet_")) return true;
		if (Function->GetName().StartsWith("netFunc_")) return true;
		if (Function->GetName().StartsWith("netSig_")) return true;
		if (Function->GetName() == "netDesc" && CastField<FTextProperty>(Function->GetReturnProperty()) && Function->ParmsSize == sizeof(FText)) return true;
	}
	return false;
}

bool UFINUReflectionSource::ProvidesRequirements(UScriptStruct* Struct) const {
	if (Struct->GetName().EndsWith("_NetType")) {
		return true;
	}
	return false;
}

void UFINUReflectionSource::FillData(FFINReflection* Ref, UFINClass* ToFillClass, UClass* Class) const {
	UFINClass* DirectParent = Ref->FindClass(Class->GetSuperClass(), false, false);
	if (DirectParent) {
		int childCount = 0;
		for(TObjectIterator<UClass> It; It; ++It) {
			if(It->IsChildOf(Class->GetSuperClass())) {
				childCount++;
			}
		}
		if (childCount < 2) {
			const_cast<TMap<UClass*, UFINClass*>*>(&Ref->GetClasses())->Remove(Class);
			ToFillClass = DirectParent;
		}
	}
	
	FFINReflectionClassMeta Meta = GetClassMeta(Class);
	ToFillClass->InternalName = Meta.InternalName;
	ToFillClass->DisplayName = Meta.DisplayName;
	ToFillClass->Description = Meta.Description;

	if (Class->GetOuterUPackage()->GetName().Contains("FactoryGame")) {
		ToFillClass->InternalName.ReplaceCharInline('-', '_');
	}

	for (TFieldIterator<UFunction> Function(Class); Function; ++Function) {
		if (Function->GetOwnerClass() != Class) continue; 
		if (Function->GetName().StartsWith("netProp_")) {
			ToFillClass->Properties.Add(GenerateProperty(Ref, Meta, Class, *Function));
		} else if (Function->GetName().StartsWith("netPropGet_")) {
			ToFillClass->Properties.Add(GenerateProperty(Ref, Meta, Class, *Function));
		} else if (Function->GetName().StartsWith("netFunc_")) {
			ToFillClass->Functions.Add(GenerateFunction(Ref, Meta, Class, *Function));
		} else if (Function->GetName().StartsWith("netSig_")) {
			ToFillClass->Signals.Add(GenerateSignal(Ref, Meta, Class, *Function));
		}
	}
		
	ToFillClass->Parent = Ref->FindClass(Class->GetSuperClass());
	if (ToFillClass->Parent == ToFillClass) ToFillClass->Parent = nullptr;

	if (ToFillClass->Properties.Num() > 0 && ToFillClass->Functions.Num() > 0 && ToFillClass->Signals.Num() > 0) {
		checkf(CheckName(ToFillClass->InternalName), TEXT("Invalid name '%s' for class '%s'"), *ToFillClass->InternalName, *Class->GetPathName());
	}
}

void UFINUReflectionSource::FillData(FFINReflection* Ref, UFINStruct* ToFillStruct, UScriptStruct* Struct) const {
	ToFillStruct->DisplayName = FText::FromString(Struct->GetName());
	ToFillStruct->InternalName = Struct->GetName();
	ToFillStruct->Description = FText::FromString("");
}

FFINReflectionClassMeta UFINUReflectionSource::GetClassMeta(UClass* Class) const {
	FFINReflectionClassMeta Meta;

	Meta.InternalName = Class->GetName();
	Meta.DisplayName = FText::FromString(Meta.InternalName);

	// try tp get meta from buildable
	if (Class->IsChildOf(AFGBuildable::StaticClass())) {
		AFGBuildable* DefaultObj = Cast<AFGBuildable>(Class->GetDefaultObject());
		Meta.Description = DefaultObj->mDescription;
		Meta.DisplayName = DefaultObj->mDisplayName;
	}

	// try to get meta from function
	UFunction* MetaFunc = Class->FindFunctionByName("netClass_Meta");
	
	if (MetaFunc && MetaFunc->GetOuter() == Class) {
		TMap<FString, FFINReflectionFunctionMeta> funcMetas;
		TMap<FString, FFINReflectionPropertyMeta> propMetas;
		// allocate parameter space
		uint8* Params = (uint8*)FMemory_Alloca(MetaFunc->GetStructureSize());
		if (Params) MetaFunc->InitializeStruct(Params);
		ON_SCOPE_EXIT {
			if (Params) MetaFunc->DestroyStruct(Params);
		};

		Class->GetDefaultObject()->ProcessEvent(MetaFunc, Params);

		for (TFieldIterator<FProperty> Property(MetaFunc); Property; ++Property) {
			if(!(Property->PropertyFlags & CPF_OutParm)) {
				continue;
			}
		
			FTextProperty* TextProp = CastField<FTextProperty>(*Property);
			FStrProperty* StrProp = CastField<FStrProperty>(*Property);
			FMapProperty* MapProp = CastField<FMapProperty>(*Property);
			FStructProperty* StructProp = CastField<FStructProperty>(*Property);
			if (StrProp && Property->GetName() == "InternalName") Meta.InternalName = StrProp->GetPropertyValue_InContainer(Params);
			else if (TextProp && Property->GetName() == "DisplayName") Meta.DisplayName = TextProp->GetPropertyValue_InContainer(Params);
			else if (TextProp && Property->GetName() == "Description") Meta.Description = TextProp->GetPropertyValue_InContainer(Params);
			else if (StructProp) {
				if(StructProp->Struct == FFINReflectionFunctionMeta::StaticStruct()) {
					FString FuncName = StructProp->GetName();
					check(FuncName.RemoveFromStart(TEXT("netFuncMeta_")));
					FFINReflectionFunctionMeta* FuncMeta = StructProp->ContainerPtrToValuePtr<FFINReflectionFunctionMeta>(Params);
					Meta.Functions.Add(FuncName, *FuncMeta);
				} else if(StructProp->Struct == FFINReflectionSignalMeta::StaticStruct()) {
					FString SignalName = StructProp->GetName();
					check(SignalName.RemoveFromStart(TEXT("netSignalMeta_")));
					FFINReflectionSignalMeta* SignalMeta = StructProp->ContainerPtrToValuePtr<FFINReflectionSignalMeta>(Params);
					Meta.Signals.Add(SignalName, *SignalMeta);
				} else if(StructProp->Struct == FFINReflectionPropertyMeta::StaticStruct()) {
					FString PropName = StructProp->GetName(); 
					check(PropName.StartsWith(TEXT("netPropMeta_")));
					FFINReflectionPropertyMeta* PropMeta = StructProp->ContainerPtrToValuePtr<FFINReflectionPropertyMeta>(Params);
					Meta.Properties.Add(PropName, *PropMeta);
				}
			} else if (MapProp && CastField<FStrProperty>(MapProp->KeyProp)) {
				if (CastField<FStrProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyInternalNames") {
					FieldMapToMetaMap<FString>(Meta.Properties, &FFINReflectionPropertyMeta::InternalName, *MapProp->ContainerPtrToValuePtr<TMap<FString, FString>>(Params));
				} else if (CastField<FTextProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyDisplayNames") {
					FieldMapToMetaMap<FText>(Meta.Properties, &FFINReflectionPropertyMeta::DisplayName, *MapProp->ContainerPtrToValuePtr<TMap<FString, FText>>(Params));
				} else if (CastField<FTextProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyDescriptions") {
					FieldMapToMetaMap<FText>(Meta.Properties, &FFINReflectionPropertyMeta::Description, *MapProp->ContainerPtrToValuePtr<TMap<FString, FText>>(Params));
				} else if (CastField<FIntProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyRuntimes") {
					FieldMapToMetaMap<EFINReflectionMetaRuntimeState>(Meta.Properties, &FFINReflectionPropertyMeta::RuntimeState, *MapProp->ContainerPtrToValuePtr<TMap<FString, int32>>(Params));
				}
			}
		}
	}
	
	return Meta;
}

FFINReflectionFunctionMeta UFINUReflectionSource::GetFunctionMeta(UClass* Class, UFunction* Func) const {
	FFINReflectionFunctionMeta Meta;

	// try to get meta from function
	UFunction* MetaFunc = Class->FindFunctionByName(*(FString("netFuncMeta_") + GetFunctionNameFromUFunction(Func)));
	if (MetaFunc) {
		// allocate parameter space
		uint8* Params = (uint8*)FMemory_Alloca(MetaFunc->GetStructureSize());
		if (Params) MetaFunc->InitializeStruct(Params);
		ON_SCOPE_EXIT {
			if (Params) MetaFunc->DestroyStruct(Params);
		};

		Class->GetDefaultObject()->ProcessEvent(MetaFunc, Params);

		for (TFieldIterator<FProperty> Property(MetaFunc); Property; ++Property) {
			FTextProperty* TextProp = CastField<FTextProperty>(*Property);
			FStrProperty* StrProp = CastField<FStrProperty>(*Property);
			FArrayProperty* ArrayProp = CastField<FArrayProperty>(*Property);
			FIntProperty* IntProp = CastField<FIntProperty>(*Property);
			if (StrProp && Property->GetName() == "InternalName") Meta.InternalName = StrProp->GetPropertyValue_InContainer(Params);
			else if (TextProp && Property->GetName() == "DisplayName") Meta.DisplayName = TextProp->GetPropertyValue_InContainer(Params);
			else if (TextProp && Property->GetName() == "Description") Meta.Description = TextProp->GetPropertyValue_InContainer(Params);
			else if (ArrayProp && CastField<FStrProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterInternalNames") {
				FieldArrayToMetaArray(Meta.Parameters, &FFINReflectionFunctionParameterMeta::InternalName, *ArrayProp->ContainerPtrToValuePtr<TArray<FString>>(Params));
			} else if (ArrayProp && CastField<FTextProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterDisplayNames") {
				FieldArrayToMetaArray(Meta.Parameters, &FFINReflectionFunctionParameterMeta::DisplayName, *ArrayProp->ContainerPtrToValuePtr<TArray<FText>>(Params));
			} else if (ArrayProp && CastField<FTextProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterDescriptions") {
				FieldArrayToMetaArray(Meta.Parameters, &FFINReflectionFunctionParameterMeta::Description, *ArrayProp->ContainerPtrToValuePtr<TArray<FText>>(Params));
			} else if (IntProp && Property->GetName() == "Runtime") Meta.RuntimeState = (EFINReflectionMetaRuntimeState)IntProp->GetPropertyValue_InContainer(Params);
		}
	}

	return Meta;
}

FFINReflectionSignalMeta UFINUReflectionSource::GetSignalMeta(UClass* Class, UFunction* Func) const {
	FFINReflectionSignalMeta Meta;

	// try to get meta from function
	UFunction* MetaFunc = Class->FindFunctionByName(*(FString("netSigMeta_") + GetSignalNameFromUFunction(Func)));
	if (MetaFunc) {
		// allocate parameter space
		uint8* Params = (uint8*)FMemory_Alloca(MetaFunc->GetStructureSize());
		if (Params) MetaFunc->InitializeStruct(Params);
		ON_SCOPE_EXIT {
			if (Params) MetaFunc->DestroyStruct(Params);
		};
		
		Class->GetDefaultObject()->ProcessEvent(MetaFunc, Params);

		for (TFieldIterator<FProperty> Property(MetaFunc); Property; ++Property) {
			FTextProperty* TextProp = CastField<FTextProperty>(*Property);
			FStrProperty* StrProp = CastField<FStrProperty>(*Property);
			FArrayProperty* ArrayProp = CastField<FArrayProperty>(*Property);
			if (StrProp && Property->GetName() == "InternalName") Meta.InternalName = StrProp->GetPropertyValue_InContainer(Params);
			else if (TextProp && Property->GetName() == "DisplayName") Meta.DisplayName = TextProp->GetPropertyValue_InContainer(Params);
			else if (TextProp && Property->GetName() == "Description") Meta.Description = TextProp->GetPropertyValue_InContainer(Params);
			else if (ArrayProp && CastField<FStrProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterInternalNames") {
				FieldArrayToMetaArray(Meta.Parameters, &FFINReflectionSignalMeta::InternalName, *ArrayProp->ContainerPtrToValuePtr<TArray<FString>>(Params));
			} else if (ArrayProp && CastField<FTextProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterDisplayNames") {
				FieldArrayToMetaArray(Meta.Parameters, &FFINReflectionSignalMeta::DisplayName, *ArrayProp->ContainerPtrToValuePtr<TArray<FText>>(Params));
			} else if (ArrayProp && CastField<FTextProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterDescriptions") {
				FieldArrayToMetaArray(Meta.Parameters, &FFINReflectionSignalMeta::Description, *ArrayProp->ContainerPtrToValuePtr<TArray<FText>>(Params));
			}
		}
	}

	return Meta;
}

FString UFINUReflectionSource::GetFunctionNameFromUFunction(UFunction* Func) const {
	FString Name = Func->GetName();
	Name.RemoveFromStart("netFunc_");
	return Name;
}

FString UFINUReflectionSource::GetPropertyNameFromUFunction(UFunction* Func) const {
	FString Name = Func->GetName();
	if (!Name.RemoveFromStart("netPropGet_")) Name.RemoveFromStart("netPropSet_");
	return Name;
}

FString UFINUReflectionSource::GetPropertyNameFromFProperty(FProperty* Prop, bool& bReadOnly) const {
	FString Name = Prop->GetName();
	if (!Name.RemoveFromStart("netProp_")) bReadOnly = Name.RemoveFromStart("netPropReadOnly_");
	return Name;
}

FString UFINUReflectionSource::GetSignalNameFromUFunction(UFunction* Func) const {
	FString Name = Func->GetName();
	Name.RemoveFromStart("netSig_");
	return Name;
}

UFINFunction* UFINUReflectionSource::GenerateFunction(FFINReflection* Ref, const FFINReflectionTypeMeta& TypeMeta, UClass* Class, UFunction* Func) const {
	FFINReflectionFunctionMeta Meta;
	const FString FuncName = GetFunctionNameFromUFunction(Func);
	if (const FFINReflectionFunctionMeta* MetaPtr = TypeMeta.Functions.Find(FuncName)) {
		Meta = *MetaPtr;
	} else {
		Meta = GetFunctionMeta(Class, Func);
	}

	UFINUFunction* FINFunc = NewObject<UFINUFunction>(Ref->FindClass(Class, false, false));
	FINFunc->RefFunction = Func;
	FINFunc->InternalName = FuncName;
	FINFunc->DisplayName = FText::FromString(FINFunc->InternalName);
	FINFunc->FunctionFlags = FIN_Func_MemberFunc | FIN_Func_Parallel;
	
	if (Meta.InternalName.Len()) FINFunc->InternalName = Meta.InternalName;
	if (!Meta.DisplayName.IsEmpty()) FINFunc->DisplayName = Meta.DisplayName;
	if (!Meta.Description.IsEmpty()) FINFunc->Description = Meta.Description;
	switch (Meta.RuntimeState) {
	case EFINReflectionMetaRuntimeState::Synchronous:
		FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Sync;
		break;
	case EFINReflectionMetaRuntimeState::Parallel:
		FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Parallel;
		break;
	case EFINReflectionMetaRuntimeState::Asynchronous:
		FINFunc->FunctionFlags = (FINFunc->FunctionFlags & ~FIN_Func_Runtime) | FIN_Func_Async;
		break;
	default:
		break;
	}
	
	for (TFieldIterator<FProperty> Param(Func); Param; ++Param) {
		if (!(Param->PropertyFlags & CPF_Parm)) continue;
		int ParameterIndex = FINFunc->Parameters.Num();
		UFINProperty* FINProp = FINCreateFINPropertyFromFProperty(*Param, FINFunc);
		
		FINProp->InternalName = Param->GetName();
		FINProp->DisplayName = FText::FromString(FINProp->InternalName);
		FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Param;

		if (Meta.Parameters.Num() > ParameterIndex) {
			const FFINReflectionFunctionParameterMeta& ParamMeta = Meta.Parameters[ParameterIndex];
			if (!ParamMeta.InternalName.IsEmpty()) FINProp->InternalName = ParamMeta.InternalName;
			if (!ParamMeta.DisplayName.IsEmpty()) FINProp->DisplayName = ParamMeta.DisplayName;
			if (!ParamMeta.Description.IsEmpty()) FINProp->Description = ParamMeta.Description;
		}

		FINFunc->Parameters.Add(FINProp);

		checkf(CheckName(FINFunc->GetInternalName()), TEXT("Invalid parameter name '%s' for function '%s'"), *FINProp->GetInternalName(), *Func->GetFullName());
	}

	// Handle Var Args property
	if (FINFunc->Parameters.Num() > 0) {
		UFINArrayProperty* Prop = nullptr;
		for (int i = FINFunc->Parameters.Num()-1; i >= 0; --i) {
			UFINArrayProperty* ArrProp = Cast<UFINArrayProperty>(FINFunc->Parameters[i]);
			if (ArrProp && !(ArrProp->GetPropertyFlags() & FIN_Prop_OutParam || ArrProp->GetPropertyFlags() & FIN_Prop_RetVal)) {
				Prop = ArrProp;
				break;
			}
		}
		if (Prop && UFINReflectionUtils::CheckIfVarargs(Prop)) {
			FINFunc->FunctionFlags = FINFunc->FunctionFlags | FIN_Func_VarArgs;
			FINFunc->VarArgsProperty = Prop;
			FINFunc->Parameters.Remove(Prop);
		}
	}

	checkf(CheckName(FINFunc->GetInternalName()), TEXT("Invalid function name '%s' for class '%s'"), *FINFunc->GetInternalName(), *Class->GetFullName());
	
	return FINFunc;
}

UFINProperty* UFINUReflectionSource::GenerateProperty(FFINReflection* Ref, const FFINReflectionTypeMeta& TypeMeta, UClass* Class, FProperty* Prop) const {
	FFINReflectionPropertyMeta Meta;
	bool bReadOnly = true;
	const FString PropName = GetPropertyNameFromFProperty(Prop, bReadOnly);
	if (const FFINReflectionPropertyMeta* MetaPtr = TypeMeta.Properties.Find(PropName)) {
		Meta = *MetaPtr;
	}
	
	UFINProperty* FINProp = FINCreateFINPropertyFromFProperty(Prop, Ref->FindClass(Class, false, false));
	FINProp->InternalName = PropName;
	FINProp->DisplayName = FText::FromString(PropName);
	FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Attrib | FIN_Prop_Parallel;;
	if (bReadOnly) FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_ReadOnly;
	
	if (!Meta.InternalName.IsEmpty()) FINProp->InternalName = Meta.InternalName;
	if (!Meta.DisplayName.IsEmpty()) FINProp->DisplayName = Meta.DisplayName;
	if (!Meta.Description.IsEmpty()) FINProp->Description = Meta.Description;
	switch (Meta.RuntimeState) {
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

	checkf(CheckName(FINProp->GetInternalName()), TEXT("Invalid property name '%s' for class '%s'"), *FINProp->GetInternalName(), *Class->GetFullName());
	
	return FINProp;
}

UFINProperty* UFINUReflectionSource::GenerateProperty(FFINReflection* Ref, const FFINReflectionTypeMeta& TypeMeta, UClass* Class, UFunction* Get) const {
	FFINReflectionPropertyMeta Meta;
	
	FProperty* GetProp = nullptr;
	for (TFieldIterator<FProperty> Param(Get); Param; ++Param) {
		if (Param->PropertyFlags & CPF_Parm) {
			check(Param->PropertyFlags & CPF_OutParm);
			check(GetProp == nullptr);
			GetProp = *Param;
		}
	}

	const FString PropName = GetPropertyNameFromUFunction(Get);
	if (const FFINReflectionPropertyMeta* MetaPtr = TypeMeta.Properties.Find(PropName)) {
		Meta = *MetaPtr;
	}
	
	UFINProperty* FINProp = FINCreateFINPropertyFromFProperty(GetProp, nullptr, Ref->FindClass(Class, false, false));
	FINProp->InternalName = PropName;
	FINProp->DisplayName = FText::FromString(PropName);
	FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Attrib | FIN_Prop_RT_Parallel;
	if (UFINFuncProperty* FINSProp = Cast<UFINFuncProperty>(FINProp)) {
		FINSProp->GetterFunc.Function = Get;
		FINSProp->GetterFunc.Property = FINCreateFINPropertyFromFProperty(GetProp, FINProp);
	}
	UFunction* Set = Class->FindFunctionByName(*(FString("netPropSet_") + FINProp->InternalName));
	if (Set) {
		FProperty* SetProp = nullptr;
		for (TFieldIterator<FProperty> Param(Set); Param; ++Param) {
			if (Param->PropertyFlags & CPF_Parm) {
				check(!(Param->PropertyFlags & CPF_OutParm));
				check(SetProp == nullptr);
				check(Param->GetClass() == GetProp->GetClass());
				SetProp = *Param;
			}
		}
		if (UFINFuncProperty* FINSProp = Cast<UFINFuncProperty>(FINProp)) {
			FINSProp->SetterFunc.Function = Set;
			FINSProp->SetterFunc.Property = FINCreateFINPropertyFromFProperty(SetProp, FINProp);
		}
	} else {
		FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_ReadOnly;
	}
	if (!Meta.InternalName.IsEmpty()) FINProp->InternalName = Meta.InternalName;
	if (!Meta.DisplayName.IsEmpty()) FINProp->DisplayName = Meta.DisplayName;
	if (!Meta.Description.IsEmpty()) FINProp->Description = Meta.Description;
	switch (Meta.RuntimeState) {
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
	
	checkf(CheckName(FINProp->GetInternalName()), TEXT("Invalid property name '%s' for class '%s'"), *FINProp->GetInternalName(), *Class->GetFullName());
	
	return FINProp;
}

UFINSignal* UFINUReflectionSource::GenerateSignal(FFINReflection* Ref, const FFINReflectionClassMeta& ClassMeta, UClass* Class, UFunction* Func) const {
	FFINReflectionSignalMeta Meta;
	const FString SignalName = GetSignalNameFromUFunction(Func);
	if (const FFINReflectionSignalMeta* MetaPtr = ClassMeta.Signals.Find(SignalName)) {
		Meta = *MetaPtr;
	}else {
		Meta = GetSignalMeta(Class, Func);
	}
	
	UFINSignal* FINSignal = NewObject<UFINSignal>(Ref->FindClass(Class, false, false));
	FINSignal->InternalName = GetSignalNameFromUFunction(Func);
	FINSignal->DisplayName = FText::FromString(FINSignal->InternalName);
	
	if (!Meta.InternalName.IsEmpty()) FINSignal->InternalName = Meta.InternalName;
	if (!Meta.DisplayName.IsEmpty()) FINSignal->DisplayName = Meta.DisplayName;
	if (!Meta.Description.IsEmpty()) FINSignal->Description = Meta.Description;
	for (TFieldIterator<FProperty> Param(Func); Param; ++Param) {
		if (!(Param->PropertyFlags & CPF_Parm)) continue;
		int ParameterIndex = FINSignal->Parameters.Num();
		UFINProperty* FINProp = FINCreateFINPropertyFromFProperty(*Param, FINSignal);

		FINProp->InternalName = Param->GetName();
		FINProp->DisplayName = FText::FromString(FINProp->InternalName);
		FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Param;

		if (Meta.Parameters.Num() > ParameterIndex) {
			const FFINReflectionFunctionParameterMeta& ParamMeta = Meta.Parameters[ParameterIndex];
			if (!ParamMeta.InternalName.IsEmpty()) FINProp->InternalName = Meta.InternalName;
			if (!ParamMeta.DisplayName.IsEmpty()) FINProp->DisplayName = Meta.DisplayName;
			if (!ParamMeta.Description.IsEmpty()) FINProp->Description = Meta.Description;
		}
		
		FINSignal->Parameters.Add(FINProp);
	}
	if (FINSignal->Parameters.Num() > 0) {
		UFINArrayProperty* Prop = Cast<UFINArrayProperty>(FINSignal->Parameters[FINSignal->Parameters.Num()-1]);
		if (Prop && Prop->GetInternalName() == "varargs") {
			UFINStructProperty* Inner = Cast<UFINStructProperty>(Prop->InnerType);
			if (FINSignal && Inner->Property && Inner->Property->Struct == FFINAnyNetworkValue::StaticStruct()) {
				FINSignal->bIsVarArgs = true;
				FINSignal->Parameters.Pop();
			}
		}
	}
	FuncSignalMap.Add(Func, FINSignal);
	SetupFunctionAsSignal(Ref, Func);

	checkf(CheckName(FINSignal->GetInternalName()), TEXT("Invalid signal name '%s' for class '%s'"), *FINSignal->GetInternalName(), *Class->GetFullName());
	
	return FINSignal;
}

UFINSignal* UFINUReflectionSource::GetSignalFromFunction(UFunction* Func) {
	UFINSignal** Signal = FuncSignalMap.Find(Func);
	if (Signal) return *Signal;
	return nullptr;
}

void FINUFunctionBasedSignalExecute(UObject* Context, FFrame& Stack, RESULT_DECL) {
	// get signal name
	UFINSignal* FINSignal = UFINUReflectionSource::GetSignalFromFunction(Stack.CurrentNativeFunction);
	if (!FINSignal || !Context) {
		UE_LOG(LogFicsItNetworks, Display, TEXT("Invalid Unreal Reflection Signal Execution '%s'"), *Stack.CurrentNativeFunction->GetName());

		P_FINISH;
		
		return;
	}

	// allocate signal data storage and copy data
	void* ParamStruct = FMemory_Alloca(Stack.CurrentNativeFunction->GetStructureSize());
	if (ParamStruct) Stack.CurrentNativeFunction->InitializeStruct(ParamStruct);
	ON_SCOPE_EXIT{
		if (ParamStruct) Stack.CurrentNativeFunction->DestroyStruct(ParamStruct);
	};

	for (auto p = TFieldIterator<FProperty>(Stack.CurrentNativeFunction); p; ++p) {
		auto dp = p->ContainerPtrToValuePtr<void>(ParamStruct);
		if (Stack.Code) {
			std::invoke(&FFrame::Step, Stack, Context, dp);
		} else {
			Stack.StepExplicitProperty(dp, *p);
		}
	}

	// copy data into parameter list
	TArray<FFINAnyNetworkValue> Parameters;
	TArray<UFINProperty*> ParameterList = FINSignal->GetParameters();
	for (UFINProperty* Parameter : ParameterList) {
		Parameters.Add(Parameter->GetValue(ParamStruct));
	}
	if (FINSignal->bIsVarArgs && Parameters.Num() > 0 && Parameters.Last().GetType() == FIN_ARRAY) {
		FFINAnyNetworkValue Array = Parameters.Last();
		Parameters.Pop();
		Parameters.Append(Array.GetArray());
	}

	FINSignal->Trigger(Context, Parameters);

	P_FINISH;
}

void UFINUReflectionSource::SetupFunctionAsSignal(FFINReflection* Ref, UFunction* Func) const {
	Func->SetNativeFunc(&FINUFunctionBasedSignalExecute);
	Func->FunctionFlags |= FUNC_Native;
}

bool UFINUReflectionSource::CheckName(const FString& Name) {
	FRegexPattern Pattern(TEXT("^[\\w]+$"));
	return FRegexMatcher(Pattern, Name).FindNext();
}
