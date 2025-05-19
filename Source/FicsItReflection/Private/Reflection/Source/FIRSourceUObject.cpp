#include "Reflection/Source/FIRSourceUObject.h"

#include "FicsItReflection.h"
#include "Buildables/FGBuildable.h"
#include "FIRUtils.h"
#include "Regex.h"
#include "Reflection/FIRArrayProperty.h"
#include "Reflection/FIRStructProperty.h"
#include "Reflection/FIRUFunction.h"

TMap<UFunction*, UFIRSignal*> UFIRSourceUObject::FuncSignalMap;

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

bool UFIRSourceUObject::ProvidesRequirements(UClass* Class) const {
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

bool UFIRSourceUObject::ProvidesRequirements(UScriptStruct* Struct) const {
	if (Struct->GetName().EndsWith("_NetType")) {
		return true;
	}
	return false;
}

UFIRClass* UFIRSourceUObject::FillData(FFicsItReflectionModule* Ref, UFIRClass* ToFillClass, UClass* Class) {
	UFIRClass* DirectParent = Ref->FindClass(Class->GetSuperClass(), false, false);
	if (DirectParent) {
		int childCount = 0;
		for(TObjectIterator<UClass> It; It; ++It) {
			if(It->IsChildOf(Class->GetSuperClass())) {
				childCount++;
			}
		}
		if (childCount < 2) {
			const_cast<TMap<UClass*, UFIRClass*>*>(&Ref->GetClasses())->Remove(Class);
			ToFillClass = DirectParent;
		}
	}
	
	FFIRClassMeta Meta = GetClassMeta(Class);

	if (!IsValid(ToFillClass)) {
		ToFillClass = NewObject<UFIRClass>(this, FName(Meta.InternalName));
	}

	ToFillClass->InternalName = Meta.InternalName;
	ToFillClass->DisplayName = Meta.DisplayName;
	ToFillClass->Description = Meta.Description;

	if (Class->GetOuterUPackage()->GetName().Contains("FactoryGame")) {
		ToFillClass->InternalName.ReplaceCharInline('-', '_');
	}

	for (TFieldIterator<UFunction> Function(Class); Function; ++Function) {
		if (Function->GetOwnerClass() != Class) continue; 
		if (Function->GetName().StartsWith("netProp_")) {
			ToFillClass->Properties.Add(GenerateProperty(Ref, Meta, Class, ToFillClass, *Function));
		} else if (Function->GetName().StartsWith("netPropGet_")) {
			ToFillClass->Properties.Add(GenerateProperty(Ref, Meta, Class, ToFillClass, *Function));
		} else if (Function->GetName().StartsWith("netFunc_")) {
			ToFillClass->Functions.Add(GenerateFunction(Ref, Meta, Class, ToFillClass, *Function));
		} else if (Function->GetName().StartsWith("netSig_")) {
			ToFillClass->Signals.Add(GenerateSignal(Ref, Meta, Class, ToFillClass, *Function));
		}
	}
		
	ToFillClass->Parent = Ref->FindClass(Class->GetSuperClass());
	if (ToFillClass->Parent == ToFillClass) ToFillClass->Parent = nullptr;

	if (ToFillClass->Properties.Num() > 0 && ToFillClass->Functions.Num() > 0 && ToFillClass->Signals.Num() > 0) {
		checkf(CheckName(ToFillClass->InternalName), TEXT("Invalid name '%s' for class '%s'"), *ToFillClass->InternalName, *Class->GetPathName());
	}

	return ToFillClass;
}

UFIRStruct* UFIRSourceUObject::FillData(FFicsItReflectionModule* Ref, UFIRStruct* ToFillStruct, UScriptStruct* Struct) {
	if (!IsValid(ToFillStruct)) {
		ToFillStruct = NewObject<UFIRStruct>(this, FName(Struct->GetName()));
	}

	ToFillStruct->DisplayName = FText::FromString(Struct->GetName());
	ToFillStruct->InternalName = Struct->GetName();
	ToFillStruct->Description = FText::FromString("");

	return ToFillStruct;
}

FFIRClassMeta UFIRSourceUObject::GetClassMeta(UClass* Class) const {
	FFIRClassMeta Meta;

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
		TMap<FString, FFIRFunctionMeta> funcMetas;
		TMap<FString, FFIRPropertyMeta> propMetas;
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
				if(StructProp->Struct == FFIRFunctionMeta::StaticStruct()) {
					FString FuncName = StructProp->GetName();
					check(FuncName.RemoveFromStart(TEXT("netFuncMeta_")));
					FFIRFunctionMeta* FuncMeta = StructProp->ContainerPtrToValuePtr<FFIRFunctionMeta>(Params);
					Meta.Functions.Add(FuncName, *FuncMeta);
				} else if(StructProp->Struct == FFIRSignalMeta::StaticStruct()) {
					FString SignalName = StructProp->GetName();
					check(SignalName.RemoveFromStart(TEXT("netSignalMeta_")));
					FFIRSignalMeta* SignalMeta = StructProp->ContainerPtrToValuePtr<FFIRSignalMeta>(Params);
					Meta.Signals.Add(SignalName, *SignalMeta);
				} else if(StructProp->Struct == FFIRPropertyMeta::StaticStruct()) {
					FString PropName = StructProp->GetName(); 
					check(PropName.RemoveFromStart(TEXT("netPropMeta_")));
					FFIRPropertyMeta* PropMeta = StructProp->ContainerPtrToValuePtr<FFIRPropertyMeta>(Params);
					Meta.Properties.Add(PropName, *PropMeta);
				}
			} else if (MapProp && CastField<FStrProperty>(MapProp->KeyProp)) {
				if (CastField<FStrProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyInternalNames") {
					FieldMapToMetaMap<FString>(Meta.Properties, &FFIRPropertyMeta::InternalName, *MapProp->ContainerPtrToValuePtr<TMap<FString, FString>>(Params));
				} else if (CastField<FTextProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyDisplayNames") {
					FieldMapToMetaMap<FText>(Meta.Properties, &FFIRPropertyMeta::DisplayName, *MapProp->ContainerPtrToValuePtr<TMap<FString, FText>>(Params));
				} else if (CastField<FTextProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyDescriptions") {
					FieldMapToMetaMap<FText>(Meta.Properties, &FFIRPropertyMeta::Description, *MapProp->ContainerPtrToValuePtr<TMap<FString, FText>>(Params));
				} else if (CastField<FIntProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyRuntimes") {
					FieldMapToMetaMap<EFIRMetaRuntimeState>(Meta.Properties, &FFIRPropertyMeta::RuntimeState, *MapProp->ContainerPtrToValuePtr<TMap<FString, int32>>(Params));
				}
			}
		}
	}
	
	return Meta;
}

FFIRFunctionMeta UFIRSourceUObject::GetFunctionMeta(UClass* Class, UFunction* Func) const {
	FFIRFunctionMeta Meta;

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
				FieldArrayToMetaArray(Meta.Parameters, &FFIRFunctionParameterMeta::InternalName, *ArrayProp->ContainerPtrToValuePtr<TArray<FString>>(Params));
			} else if (ArrayProp && CastField<FTextProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterDisplayNames") {
				FieldArrayToMetaArray(Meta.Parameters, &FFIRFunctionParameterMeta::DisplayName, *ArrayProp->ContainerPtrToValuePtr<TArray<FText>>(Params));
			} else if (ArrayProp && CastField<FTextProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterDescriptions") {
				FieldArrayToMetaArray(Meta.Parameters, &FFIRFunctionParameterMeta::Description, *ArrayProp->ContainerPtrToValuePtr<TArray<FText>>(Params));
			} else if (IntProp && Property->GetName() == "Runtime") Meta.RuntimeState = (EFIRMetaRuntimeState)IntProp->GetPropertyValue_InContainer(Params);
		}
	}

	return Meta;
}

FFIRSignalMeta UFIRSourceUObject::GetSignalMeta(UClass* Class, UFunction* Func) const {
	FFIRSignalMeta Meta;

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
				FieldArrayToMetaArray(Meta.Parameters, &FFIRSignalMeta::InternalName, *ArrayProp->ContainerPtrToValuePtr<TArray<FString>>(Params));
			} else if (ArrayProp && CastField<FTextProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterDisplayNames") {
				FieldArrayToMetaArray(Meta.Parameters, &FFIRSignalMeta::DisplayName, *ArrayProp->ContainerPtrToValuePtr<TArray<FText>>(Params));
			} else if (ArrayProp && CastField<FTextProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterDescriptions") {
				FieldArrayToMetaArray(Meta.Parameters, &FFIRSignalMeta::Description, *ArrayProp->ContainerPtrToValuePtr<TArray<FText>>(Params));
			}
		}
	}

	return Meta;
}

FString UFIRSourceUObject::GetFunctionNameFromUFunction(UFunction* Func) const {
	FString Name = Func->GetName();
	Name.RemoveFromStart("netFunc_");
	return Name;
}

FString UFIRSourceUObject::GetPropertyNameFromUFunction(UFunction* Func) const {
	FString Name = Func->GetName();
	if (!Name.RemoveFromStart("netPropGet_")) Name.RemoveFromStart("netPropSet_");
	return Name;
}

FString UFIRSourceUObject::GetPropertyNameFromFProperty(FProperty* Prop, bool& bReadOnly) const {
	FString Name = Prop->GetName();
	if (!Name.RemoveFromStart("netProp_")) bReadOnly = Name.RemoveFromStart("netPropReadOnly_");
	return Name;
}

FString UFIRSourceUObject::GetSignalNameFromUFunction(UFunction* Func) const {
	FString Name = Func->GetName();
	Name.RemoveFromStart("netSig_");
	return Name;
}

UFIRFunction* UFIRSourceUObject::GenerateFunction(FFicsItReflectionModule* Ref, const FFIRTypeMeta& TypeMeta, UClass* Class, UFIRClass* FIRClass, UFunction* Func) const {
	FFIRFunctionMeta Meta;
	const FString FuncName = GetFunctionNameFromUFunction(Func);
	if (const FFIRFunctionMeta* MetaPtr = TypeMeta.Functions.Find(FuncName)) {
		Meta = *MetaPtr;
	} else {
		Meta = GetFunctionMeta(Class, Func);
	}

	EFIRFunctionFlags flags = FIR_Func_MemberFunc | FIR_Func_Parallel;
	switch (Meta.RuntimeState) {
		case EFIRMetaRuntimeState::Synchronous:
			flags |= FIR_Func_Sync;
			break;
		case EFIRMetaRuntimeState::Parallel:
			flags |= FIR_Func_Parallel;
			break;
		case EFIRMetaRuntimeState::Asynchronous:
			flags |= FIR_Func_Async;
			break;
		default:
			break;
	}

	UFIRUFunction* FIRFunc = NewObject<UFIRUFunction>(FIRClass, FIRFunctionObjectName(flags, FuncName));
	FIRFunc->RefFunction = Func;
	FIRFunc->InternalName = FuncName;
	FIRFunc->DisplayName = FText::FromString(FIRFunc->InternalName);
	FIRFunc->FunctionFlags = (flags & ~FIR_Func_Runtime) | flags;
	
	if (Meta.InternalName.Len()) FIRFunc->InternalName = Meta.InternalName;
	if (!Meta.DisplayName.IsEmpty()) FIRFunc->DisplayName = Meta.DisplayName;
	if (!Meta.Description.IsEmpty()) FIRFunc->Description = Meta.Description;
	
	for (TFieldIterator<FProperty> Param(Func); Param; ++Param) {
		if (!(Param->PropertyFlags & CPF_Parm)) continue;
		int ParameterIndex = FIRFunc->Parameters.Num();
		UFIRProperty* FIRProp = FIRCreateFIRPropertyFromFProperty(*Param, FIRFunc, FName(Param->GetName()));
		
		FIRProp->InternalName = Param->GetName();
		FIRProp->DisplayName = FText::FromString(FIRProp->InternalName);
		FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_Param;

		if (Meta.Parameters.Num() > ParameterIndex) {
			const FFIRFunctionParameterMeta& ParamMeta = Meta.Parameters[ParameterIndex];
			if (!ParamMeta.InternalName.IsEmpty()) FIRProp->InternalName = ParamMeta.InternalName;
			if (!ParamMeta.DisplayName.IsEmpty()) FIRProp->DisplayName = ParamMeta.DisplayName;
			if (!ParamMeta.Description.IsEmpty()) FIRProp->Description = ParamMeta.Description;
		}

		FIRFunc->Parameters.Add(FIRProp);

		checkf(CheckName(FIRFunc->GetInternalName()), TEXT("Invalid parameter name '%s' for function '%s'"), *FIRProp->GetInternalName(), *Func->GetFullName());
	}

	// Handle Var Args property
	if (FIRFunc->Parameters.Num() > 0) {
		UFIRArrayProperty* Prop = nullptr;
		for (int i = FIRFunc->Parameters.Num()-1; i >= 0; --i) {
			UFIRArrayProperty* ArrProp = Cast<UFIRArrayProperty>(FIRFunc->Parameters[i]);
			if (ArrProp && !(ArrProp->GetPropertyFlags() & FIR_Prop_OutParam || ArrProp->GetPropertyFlags() & FIR_Prop_RetVal)) {
				Prop = ArrProp;
				break;
			}
		}
		if (Prop && UFIRUtils::CheckIfVarargs(Prop)) {
			FIRFunc->FunctionFlags = FIRFunc->FunctionFlags | FIR_Func_VarArgs;
			FIRFunc->VarArgsProperty = Prop;
			FIRFunc->Parameters.Remove(Prop);
		}
	}

	checkf(CheckName(FIRFunc->GetInternalName()), TEXT("Invalid function name '%s' for class '%s'"), *FIRFunc->GetInternalName(), *Class->GetFullName());
	
	return FIRFunc;
}

UFIRProperty* UFIRSourceUObject::GenerateProperty(FFicsItReflectionModule* Ref, const FFIRTypeMeta& TypeMeta, UClass* Class, UFIRClass* FIRClass, FProperty* Prop) const {
	FFIRPropertyMeta Meta;
	bool bReadOnly = true;
	FString PropName = GetPropertyNameFromFProperty(Prop, bReadOnly);
	if (const FFIRPropertyMeta* MetaPtr = TypeMeta.Properties.Find(PropName)) {
		Meta = *MetaPtr;
	}

	EFIRPropertyFlags flags = FIR_Prop_Attrib | FIR_Prop_Parallel;
	if (bReadOnly) flags |= FIR_Prop_ReadOnly;
	switch (Meta.RuntimeState) {
		case 0:
			flags |= FIR_Prop_Sync;
			break;
		case 1:
			flags |= FIR_Prop_Parallel;
			break;
		case 2:
			flags |= FIR_Prop_Async;
			break;
		default:
			break;
	}

	if (!Meta.InternalName.IsEmpty()) PropName = Meta.InternalName;

	UFIRProperty* FIRProp = FIRCreateFIRPropertyFromFProperty(Prop, FIRClass, FIRPropertyObjectName(flags, PropName));
	FIRProp->InternalName = PropName;
	FIRProp->DisplayName = FText::FromString(PropName);
	FIRProp->PropertyFlags |= (FIRProp->PropertyFlags & ~FIR_Prop_Runtime) | flags;

	if (!Meta.DisplayName.IsEmpty()) FIRProp->DisplayName = Meta.DisplayName;
	if (!Meta.Description.IsEmpty()) FIRProp->Description = Meta.Description;

	checkf(CheckName(FIRProp->GetInternalName()), TEXT("Invalid property name '%s' for class '%s'"), *FIRProp->GetInternalName(), *Class->GetFullName());
	
	return FIRProp;
}

UFIRProperty* UFIRSourceUObject::GenerateProperty(FFicsItReflectionModule* Ref, const FFIRTypeMeta& TypeMeta, UClass* Class, UFIRClass* FIRClass, UFunction* Get) const {
	FFIRPropertyMeta Meta;
	
	FProperty* GetProp = nullptr;
	for (TFieldIterator<FProperty> Param(Get); Param; ++Param) {
		if (Param->PropertyFlags & CPF_Parm) {
			check(Param->PropertyFlags & CPF_OutParm);
			check(GetProp == nullptr);
			GetProp = *Param;
		}
	}

	FString PropName = GetPropertyNameFromUFunction(Get);
	if (const FFIRPropertyMeta* MetaPtr = TypeMeta.Properties.Find(PropName)) {
		Meta = *MetaPtr;
	}
	if (!Meta.InternalName.IsEmpty()) PropName = Meta.InternalName;

	EFIRPropertyFlags flags = FIR_Prop_Attrib | FIR_Prop_RT_Parallel;
	switch (Meta.RuntimeState) {
		case 0:
			flags |= FIR_Prop_Sync;
			break;
		case 1:
			flags |= FIR_Prop_Parallel;
			break;
		case 2:
			flags |= FIR_Prop_Async;
			break;
		default:
			break;
	}

	UFIRProperty* FIRProp = FIRCreateFIRPropertyFromFProperty(GetProp, nullptr, FIRClass, FIRPropertyObjectName(flags, PropName));
	FIRProp->InternalName = PropName;
	FIRProp->DisplayName = FText::FromString(PropName);
	FIRProp->PropertyFlags = (flags & ~FIR_Prop_Runtime) | flags;
	if (UFIRFuncProperty* FIRSProp = Cast<UFIRFuncProperty>(FIRProp)) {
		FIRSProp->GetterFunc.Function = Get;
		FIRSProp->GetterFunc.Property = FIRCreateFIRPropertyFromFProperty(GetProp, FIRProp, FName(TEXT("GetterFunc")));
	}
	UFunction* Set = Class->FindFunctionByName(*(FString("netPropSet_") + FIRProp->InternalName));
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
		if (UFIRFuncProperty* FIRSProp = Cast<UFIRFuncProperty>(FIRProp)) {
			FIRSProp->SetterFunc.Function = Set;
			FIRSProp->SetterFunc.Property = FIRCreateFIRPropertyFromFProperty(SetProp, FIRProp, FName(TEXT("SetterFunc")));
		}
	} else {
		FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_ReadOnly;
	}
	if (!Meta.InternalName.IsEmpty()) FIRProp->InternalName = Meta.InternalName;
	if (!Meta.DisplayName.IsEmpty()) FIRProp->DisplayName = Meta.DisplayName;
	if (!Meta.Description.IsEmpty()) FIRProp->Description = Meta.Description;
	
	checkf(CheckName(FIRProp->GetInternalName()), TEXT("Invalid property name '%s' for class '%s'"), *FIRProp->GetInternalName(), *Class->GetFullName());
	
	return FIRProp;
}

UFIRSignal* UFIRSourceUObject::GenerateSignal(FFicsItReflectionModule* Ref, const FFIRClassMeta& ClassMeta, UClass* Class, UFIRClass* FIRClass, UFunction* Func) const {
	FFIRSignalMeta Meta;
	const FString SignalName = GetSignalNameFromUFunction(Func);
	if (const FFIRSignalMeta* MetaPtr = ClassMeta.Signals.Find(SignalName)) {
		Meta = *MetaPtr;
	}else {
		Meta = GetSignalMeta(Class, Func);
	}

	UFIRSignal* FIRSignal = NewObject<UFIRSignal>(FIRClass, FIRSignalObjectName(FIR_Signal_None, SignalName));
	FIRSignal->InternalName = GetSignalNameFromUFunction(Func);
	FIRSignal->DisplayName = FText::FromString(FIRSignal->InternalName);
	
	if (!Meta.InternalName.IsEmpty()) FIRSignal->InternalName = Meta.InternalName;
	if (!Meta.DisplayName.IsEmpty()) FIRSignal->DisplayName = Meta.DisplayName;
	if (!Meta.Description.IsEmpty()) FIRSignal->Description = Meta.Description;
	for (TFieldIterator<FProperty> Param(Func); Param; ++Param) {
		if (!(Param->PropertyFlags & CPF_Parm)) continue;
		int ParameterIndex = FIRSignal->Parameters.Num();
		UFIRProperty* FIRProp = FIRCreateFIRPropertyFromFProperty(*Param, FIRSignal, FName(Param->GetName()));

		FIRProp->InternalName = Param->GetName();
		FIRProp->DisplayName = FText::FromString(FIRProp->InternalName);
		FIRProp->PropertyFlags = FIRProp->PropertyFlags | FIR_Prop_Param;

		if (Meta.Parameters.Num() > ParameterIndex) {
			const FFIRFunctionParameterMeta& ParamMeta = Meta.Parameters[ParameterIndex];
			if (!ParamMeta.InternalName.IsEmpty()) FIRProp->InternalName = ParamMeta.InternalName;
			if (!ParamMeta.DisplayName.IsEmpty()) FIRProp->DisplayName = ParamMeta.DisplayName;
			if (!ParamMeta.Description.IsEmpty()) FIRProp->Description = ParamMeta.Description;
		}
		
		FIRSignal->Parameters.Add(FIRProp);
	}
	if (FIRSignal->Parameters.Num() > 0) {
		UFIRArrayProperty* Prop = Cast<UFIRArrayProperty>(FIRSignal->Parameters[FIRSignal->Parameters.Num()-1]);
		if (Prop && Prop->GetInternalName() == "varargs") {
			UFIRStructProperty* Inner = Cast<UFIRStructProperty>(Prop->InnerType);
			if (FIRSignal && Inner->Property && Inner->Property->Struct == FFIRAnyValue::StaticStruct()) {
				FIRSignal->bIsVarArgs = true;
				FIRSignal->Parameters.Pop();
			}
		}
	}
	FuncSignalMap.Add(Func, FIRSignal);
	SetupFunctionAsSignal(Ref, Func);

	checkf(CheckName(FIRSignal->GetInternalName()), TEXT("Invalid signal name '%s' for class '%s'"), *FIRSignal->GetInternalName(), *Class->GetFullName());
	
	return FIRSignal;
}

UFIRSignal* UFIRSourceUObject::GetSignalFromFunction(UFunction* Func) {
	UFIRSignal** Signal = FuncSignalMap.Find(Func);
	if (Signal) return *Signal;
	return nullptr;
}

void FIRUFunctionBasedSignalExecute(UObject* Context, FFrame& Stack, RESULT_DECL) {
	// get signal name
	UFIRSignal* FIRSignal = UFIRSourceUObject::GetSignalFromFunction(Stack.CurrentNativeFunction);
	if (!FIRSignal || !Context) {
		UE_LOG(LogFicsItReflection, Display, TEXT("Invalid Unreal Reflection Signal Execution '%s'"), *Stack.CurrentNativeFunction->GetName());

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
	TArray<FFIRAnyValue> Parameters;
	TArray<UFIRProperty*> ParameterList = FIRSignal->GetParameters();
	for (UFIRProperty* Parameter : ParameterList) {
		Parameters.Add(Parameter->GetValue(ParamStruct));
	}
	if (FIRSignal->bIsVarArgs && Parameters.Num() > 0 && Parameters.Last().GetType() == FIR_ARRAY) {
		FFIRAnyValue Array = Parameters.Last();
		Parameters.Pop();
		Parameters.Append(Array.GetArray());
	}

	FIRSignal->Trigger(Context, Parameters);

	P_FINISH;
}

void UFIRSourceUObject::SetupFunctionAsSignal(FFicsItReflectionModule* Ref, UFunction* Func) const {
	Func->SetNativeFunc(&FIRUFunctionBasedSignalExecute);
	Func->FunctionFlags |= FUNC_Native;
}

bool UFIRSourceUObject::CheckName(const FString& Name) {
	FRegexPattern Pattern(TEXT("^[\\w]+$"));
	return FRegexMatcher(Pattern, Name).FindNext();
}
