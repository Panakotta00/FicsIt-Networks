#include "Reflection/FINUReflectionSource.h"
#include "FicsItNetworksModule.h"
#include "Buildables/FGBuildable.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINReflectionUtils.h"
#include "Reflection/FINUFunction.h"
#include "UObject/PropertyIterator.h"

TMap<UFunction*, UFINSignal*> UFINUReflectionSource::FuncSignalMap;

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
	
	ToFillClass->InternalName = Class->GetName();
	ToFillClass->DisplayName = FText::FromString(ToFillClass->InternalName);

	FFINTypeMeta Meta = GetClassMeta(Class);

	if (Meta.InternalName.Len()) ToFillClass->InternalName = Meta.InternalName;
	if (!Meta.DisplayName.IsEmpty()) ToFillClass->DisplayName = Meta.DisplayName;
	if (!Meta.Description.IsEmpty()) ToFillClass->Description = Meta.Description;

	if (Class->GetOuterUPackage()->GetName().Contains("FactoryGame")) {
		ToFillClass->InternalName.ReplaceCharInline('-', '_');
	}

	for (TFieldIterator<UFunction> Function(Class); Function; ++Function) {
		if (Function->GetOwnerClass() != Class) continue; 
		if (Function->GetName().StartsWith("netProp_")) {
			ToFillClass->Properties.Add(GenerateProperty(Ref, Meta, Class, *Function));
		}
	}
	for (TFieldIterator<UFunction> Func(Class); Func; ++Func) {
		if (Func->GetOwnerClass() != Class) continue; 
		if (Func->GetName().StartsWith("netPropGet_")) {
			ToFillClass->Properties.Add(GenerateProperty(Ref, Meta, Class, *Func));
		} else if (Func->GetName().StartsWith("netFunc_")) {
			ToFillClass->Functions.Add(GenerateFunction(Ref, Class, *Func));
		} else if (Func->GetName().StartsWith("netSig_")) {
			ToFillClass->Signals.Add(GenerateSignal(Ref, Class, *Func));
		} else if (Func->GetName() == "netDesc" && CastField<FTextProperty>(Func->GetReturnProperty()) && Func->ParmsSize == sizeof(FText)) {
			FText Desc;
			Class->GetDefaultObject()->ProcessEvent(*Func, &Desc);
			ToFillClass->Description = Desc;
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

UFINUReflectionSource::FFINTypeMeta UFINUReflectionSource::GetClassMeta(UClass* Class) const {
	FFINTypeMeta Meta;

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
		// allocate parameter space
		uint8* Params = (uint8*)FMemory::Malloc(MetaFunc->PropertiesSize);
		FMemory::Memzero(Params + MetaFunc->ParmsSize, MetaFunc->PropertiesSize - MetaFunc->ParmsSize);
		MetaFunc->InitializeStruct(Params);
		bool bInvalidDeclaration = false;
		for (FProperty* LocalProp = MetaFunc->FirstPropertyToInit; LocalProp != NULL; LocalProp = (FProperty*)LocalProp->Next) {
			LocalProp->InitializeValue_InContainer(Params);
			FMapProperty* MapProp = CastField<FMapProperty>(LocalProp);
			if (!(LocalProp->PropertyFlags & CPF_OutParm) && !MapProp) {
				bInvalidDeclaration = true;
			}
		}

		if (!bInvalidDeclaration) {
			Class->GetDefaultObject()->ProcessEvent(MetaFunc, Params);

			for (TFieldIterator<FProperty> Property(MetaFunc); Property; ++Property) {
				FTextProperty* TextProp = CastField<FTextProperty>(*Property);
				FStrProperty* StrProp = CastField<FStrProperty>(*Property);
				FMapProperty* MapProp = CastField<FMapProperty>(*Property);
				if (StrProp && Property->GetName() == "InternalName") Meta.InternalName = StrProp->GetPropertyValue_InContainer(Params);
				else if (TextProp && Property->GetName() == "DisplayName") Meta.DisplayName = TextProp->GetPropertyValue_InContainer(Params);
				else if (TextProp && Property->GetName() == "Description") Meta.Description = TextProp->GetPropertyValue_InContainer(Params);
				else if (MapProp && CastField<FStrProperty>(MapProp->KeyProp)) {
					if (CastField<FStrProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyInternalNames") {
						Meta.PropertyInternalNames = *MapProp->ContainerPtrToValuePtr<TMap<FString, FString>>(Params);
						Meta.PropertyDisplayNames.Empty();
						for (const TPair<FString, FString>& InternalName : Meta.PropertyInternalNames) {
							Meta.PropertyDisplayNames.Add(InternalName.Key, FText::FromString(InternalName.Value));
						}
					} else if (CastField<FTextProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyDisplayNames") {
						for (const TPair<FString, FText>& DisplayName : *MapProp->ContainerPtrToValuePtr<TMap<FString, FText>>(Params)) {
							Meta.PropertyDisplayNames.FindOrAdd(DisplayName.Key) = DisplayName.Value;
						}
					} else if (CastField<FTextProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyDescriptions") {
						for (const TPair<FString, FText>& Description : *MapProp->ContainerPtrToValuePtr<TMap<FString, FText>>(Params)) {
							Meta.PropertyDescriptions.FindOrAdd(Description.Key) = Description.Value;
						}
					} else if (CastField<FIntProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyRuntimes") {
						for (const TPair<FString, int32>& Runtime : *MapProp->ContainerPtrToValuePtr<TMap<FString, int32>>(Params)) {
							Meta.PropertyRuntimes.FindOrAdd(Runtime.Key) = Runtime.Value;
						}
					}
				}
			}
		}
		
		for (FProperty* P = MetaFunc->DestructorLink; P; P = P->DestructorLinkNext) {
			if (!P->IsInContainer(MetaFunc->ParmsSize)) {
				P->DestroyValue_InContainer(Params);
			}
		}
		FMemory::Free(Params);
	}
	
	return Meta;
}

UFINUReflectionSource::FFINFunctionMeta UFINUReflectionSource::GetFunctionMeta(UClass* Class, UFunction* Func) const {
	FFINFunctionMeta Meta;

	// try to get meta from function
	UFunction* MetaFunc = Class->FindFunctionByName(*(FString("netFuncMeta_") + GetFunctionNameFromUFunction(Func)));
	if (MetaFunc) {
		// allocate parameter space
		uint8* Params = (uint8*)FMemory::Malloc(MetaFunc->PropertiesSize);
		FMemory::Memzero(Params + MetaFunc->ParmsSize, MetaFunc->PropertiesSize - MetaFunc->ParmsSize);
		MetaFunc->InitializeStruct(Params);
		bool bInvalidDeclaration = false;
		for (FProperty* LocalProp = MetaFunc->FirstPropertyToInit; LocalProp != NULL; LocalProp = (FProperty*)LocalProp->Next) {
			LocalProp->InitializeValue_InContainer(Params);
			if (!(LocalProp->PropertyFlags & CPF_OutParm)) bInvalidDeclaration = true;
		}

		if (!bInvalidDeclaration) {
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
					Meta.ParameterInternalNames = *ArrayProp->ContainerPtrToValuePtr<TArray<FString>>(Params);
					Meta.ParameterDisplayNames.Empty();
					for (const FString& InternalName : Meta.ParameterInternalNames) {
						Meta.ParameterDisplayNames.Add(FText::FromString(InternalName));
					}
				} else if (ArrayProp && CastField<FTextProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterDisplayNames") {
					int i = 0;
					for (const FText& DisplayName : *ArrayProp->ContainerPtrToValuePtr<TArray<FText>>(Params)) {
						if (Meta.ParameterDisplayNames.Num() > i) Meta.ParameterDisplayNames[i] = DisplayName;
						else Meta.ParameterDisplayNames.Add(DisplayName);
						++i;
					}
				} else if (ArrayProp && CastField<FTextProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterDescriptions") {
					int i = 0;
					for (const FText& Description : *ArrayProp->ContainerPtrToValuePtr<TArray<FText>>(Params)) {
						if (Meta.ParameterDescriptions.Num() > i) Meta.ParameterDescriptions[i] = Description;
						else Meta.ParameterDescriptions.Add(Description);
						++i;
					}
				} else if (IntProp && Property->GetName() == "Runtime") Meta.Runtime = IntProp->GetPropertyValue_InContainer(Params);
			}
		}
	
		for (FProperty* P = MetaFunc->DestructorLink; P; P = P->DestructorLinkNext) {
			if (!P->IsInContainer(MetaFunc->ParmsSize)) {
				P->DestroyValue_InContainer(Params);
			}
		}
		FMemory::Free(Params);
	}

	return Meta;
}

UFINUReflectionSource::FFINSignalMeta UFINUReflectionSource::GetSignalMeta(UClass* Class, UFunction* Func) const {
	FFINSignalMeta Meta;

	// try to get meta from function
	UFunction* MetaFunc = Class->FindFunctionByName(*(FString("netSigMeta_") + GetSignalNameFromUFunction(Func)));
	if (MetaFunc) {
		// allocate parameter space
		uint8* Params = (uint8*)FMemory::Malloc(MetaFunc->PropertiesSize);
		FMemory::Memzero(Params + MetaFunc->ParmsSize, MetaFunc->PropertiesSize - MetaFunc->ParmsSize);
		MetaFunc->InitializeStruct(Params);
		bool bInvalidDeclaration = false;
		for (FProperty* LocalProp = MetaFunc->FirstPropertyToInit; LocalProp != NULL; LocalProp = (FProperty*)LocalProp->Next) {
			LocalProp->InitializeValue_InContainer(Params);
			if (!(LocalProp->PropertyFlags & CPF_OutParm)) bInvalidDeclaration = true;
		}

		if (!bInvalidDeclaration) {
			Class->GetDefaultObject()->ProcessEvent(MetaFunc, Params);

			for (TFieldIterator<FProperty> Property(MetaFunc); Property; ++Property) {
				FTextProperty* TextProp = CastField<FTextProperty>(*Property);
				FStrProperty* StrProp = CastField<FStrProperty>(*Property);
				FArrayProperty* ArrayProp = CastField<FArrayProperty>(*Property);
				if (StrProp && Property->GetName() == "InternalName") Meta.InternalName = StrProp->GetPropertyValue_InContainer(Params);
				else if (TextProp && Property->GetName() == "DisplayName") Meta.DisplayName = TextProp->GetPropertyValue_InContainer(Params);
				else if (TextProp && Property->GetName() == "Description") Meta.Description = TextProp->GetPropertyValue_InContainer(Params);
				else if (ArrayProp && CastField<FStrProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterInternalNames") {
					Meta.ParameterInternalNames = *ArrayProp->ContainerPtrToValuePtr<TArray<FString>>(Params);
					Meta.ParameterDisplayNames.Empty();
					for (const FString& InternalName : Meta.ParameterInternalNames) {
						Meta.ParameterDisplayNames.Add(FText::FromString(InternalName));
					}
				} else if (ArrayProp && CastField<FTextProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterDisplayNames") {
					int i = 0;
					for (const FText& DisplayName : *ArrayProp->ContainerPtrToValuePtr<TArray<FText>>(Params)) {
						if (Meta.ParameterDisplayNames.Num() > i) Meta.ParameterDisplayNames[i] = DisplayName;
						else Meta.ParameterDisplayNames.Add(DisplayName);
						++i;
					}
				} else if (ArrayProp && CastField<FTextProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterDescriptions") {
					int i = 0;
					for (const FText& Description : *ArrayProp->ContainerPtrToValuePtr<TArray<FText>>(Params)) {
						if (Meta.ParameterDescriptions.Num() > i) Meta.ParameterDescriptions[i] = Description;
						else Meta.ParameterDescriptions.Add(Description);
						++i;
					}
				}
			}
		}
	
		for (FProperty* P = MetaFunc->DestructorLink; P; P = P->DestructorLinkNext) {
			if (!P->IsInContainer(MetaFunc->ParmsSize)) {
				P->DestroyValue_InContainer(Params);
			}
		}
		FMemory::Free(Params);
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

UFINFunction* UFINUReflectionSource::GenerateFunction(FFINReflection* Ref, UClass* Class, UFunction* Func) const {
	FFINFunctionMeta Meta = GetFunctionMeta(Class, Func);
	
	UFINUFunction* FINFunc = NewObject<UFINUFunction>(Ref->FindClass(Class, false, false));
	FINFunc->RefFunction = Func;
	FINFunc->InternalName = GetFunctionNameFromUFunction(Func);
	FINFunc->DisplayName = FText::FromString(FINFunc->InternalName);
	FINFunc->FunctionFlags = FIN_Func_MemberFunc;
	
	if (Meta.InternalName.Len()) FINFunc->InternalName = Meta.InternalName;
	if (!Meta.DisplayName.IsEmpty()) FINFunc->DisplayName = Meta.DisplayName;
	if (!Meta.Description.IsEmpty()) FINFunc->Description = Meta.Description;
	switch (Meta.Runtime) {
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
	for (TFieldIterator<FProperty> Param(Func); Param; ++Param) {
		if (!(Param->PropertyFlags & CPF_Parm)) continue;
		int i = FINFunc->Parameters.Num();
		UFINProperty* FINProp = FINCreateFINPropertyFromFProperty(*Param, FINFunc);
		FINProp->InternalName = Param->GetName();
		FINProp->DisplayName = FText::FromString(FINProp->InternalName);
		if (Meta.ParameterInternalNames.Num() > i) FINProp->InternalName = Meta.ParameterInternalNames[i];
		if (Meta.ParameterDisplayNames.Num() > i) FINProp->DisplayName = Meta.ParameterDisplayNames[i];
		if (Meta.ParameterDescriptions.Num() > i) FINProp->Description = Meta.ParameterDescriptions[i];
		FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Param;
		FINFunc->Parameters.Add(FINProp);

		checkf(CheckName(FINFunc->GetInternalName()), TEXT("Invalid parameter name '%s' for function '%s'"), *FINProp->GetInternalName(), *Func->GetFullName());
	}
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

UFINProperty* UFINUReflectionSource::GenerateProperty(FFINReflection* Ref, const FFINTypeMeta& Meta, UClass* Class, FProperty* Prop) const {
	UFINProperty* FINProp = FINCreateFINPropertyFromFProperty(Prop, Ref->FindClass(Class, false, false));
	FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Attrib;
	bool bReadOnly = false;
	FINProp->InternalName = GetPropertyNameFromFProperty(Prop, bReadOnly);
	if (bReadOnly) FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_ReadOnly;
	if (Meta.PropertyInternalNames.Contains(FINProp->GetInternalName())) FINProp->InternalName = Meta.PropertyInternalNames[FINProp->GetInternalName()];
	if (Meta.PropertyDisplayNames.Contains(FINProp->GetInternalName())) FINProp->DisplayName = Meta.PropertyDisplayNames[FINProp->GetInternalName()];
	if (Meta.PropertyDescriptions.Contains(FINProp->GetInternalName())) FINProp->Description = Meta.PropertyDescriptions[FINProp->GetInternalName()];
	if (Meta.PropertyRuntimes.Contains(FINProp->GetInternalName())) {
		switch (Meta.PropertyRuntimes[FINProp->GetInternalName()]) {
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
	}

	checkf(CheckName(FINProp->GetInternalName()), TEXT("Invalid property name '%s' for class '%s'"), *FINProp->GetInternalName(), *Class->GetFullName());
	
	return FINProp;
}

UFINProperty* UFINUReflectionSource::GenerateProperty(FFINReflection* Ref, const FFINTypeMeta& Meta, UClass* Class, UFunction* Get) const {
	FProperty* GetProp = nullptr;
	for (TFieldIterator<FProperty> Param(Get); Param; ++Param) {
		if (Param->PropertyFlags & CPF_Parm) {
			check(Param->PropertyFlags & CPF_OutParm);
			check(GetProp == nullptr);
			GetProp = *Param;
		}
	}
	UFINProperty* FINProp = FINCreateFINPropertyFromFProperty(GetProp, nullptr, Ref->FindClass(Class, false, false));
	FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Attrib | FIN_Prop_RT_Parallel;
	FINProp->InternalName = GetPropertyNameFromUFunction(Get);
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
	if (Meta.PropertyInternalNames.Contains(FINProp->GetInternalName())) FINProp->InternalName = Meta.PropertyInternalNames[FINProp->GetInternalName()];
	if (Meta.PropertyDisplayNames.Contains(FINProp->GetInternalName())) FINProp->DisplayName = Meta.PropertyDisplayNames[FINProp->GetInternalName()];
	if (Meta.PropertyDescriptions.Contains(FINProp->GetInternalName())) FINProp->Description = Meta.PropertyDescriptions[FINProp->GetInternalName()];
	if (Meta.PropertyRuntimes.Contains(FINProp->GetInternalName())) {
		switch (Meta.PropertyRuntimes[FINProp->GetInternalName()]) {
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
	}
	
	checkf(CheckName(FINProp->GetInternalName()), TEXT("Invalid property name '%s' for class '%s'"), *FINProp->GetInternalName(), *Class->GetFullName());
	
	return FINProp;
}

UFINSignal* UFINUReflectionSource::GenerateSignal(FFINReflection* Ref, UClass* Class, UFunction* Func) const {
	FFINSignalMeta Meta = GetSignalMeta(Class, Func);
	
	UFINSignal* FINSignal = NewObject<UFINSignal>(Ref->FindClass(Class, false, false));
	FINSignal->InternalName = GetSignalNameFromUFunction(Func);
	FINSignal->DisplayName = FText::FromString(FINSignal->InternalName);
	
	if (Meta.InternalName.Len()) FINSignal->InternalName = Meta.InternalName;
	if (!Meta.DisplayName.IsEmpty()) FINSignal->DisplayName = Meta.DisplayName;
	if (!Meta.Description.IsEmpty()) FINSignal->Description = Meta.Description;
	for (TFieldIterator<FProperty> Param(Func); Param; ++Param) {
		if (!(Param->PropertyFlags & CPF_Parm)) continue;
		int i = FINSignal->Parameters.Num();
		UFINProperty* FINProp = FINCreateFINPropertyFromFProperty(*Param, FINSignal);
		FINProp->InternalName = Param->GetName();
		FINProp->DisplayName = FText::FromString(FINProp->InternalName);
		if (Meta.ParameterInternalNames.Num() > i) FINProp->InternalName = Meta.ParameterInternalNames[i];
		if (Meta.ParameterDisplayNames.Num() > i) FINProp->DisplayName = Meta.ParameterDisplayNames[i];
		if (Meta.ParameterDescriptions.Num() > i) FINProp->Description = Meta.ParameterDescriptions[i];
		FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Param;
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
	void* ParamStruct = FMemory::Malloc(Stack.CurrentNativeFunction->PropertiesSize);
	FMemory::Memzero(((uint8*)ParamStruct) + Stack.CurrentNativeFunction->ParmsSize, Stack.CurrentNativeFunction->PropertiesSize - Stack.CurrentNativeFunction->ParmsSize);
	Stack.CurrentNativeFunction->InitializeStruct(ParamStruct);
	for (FProperty* LocalProp = Stack.CurrentNativeFunction->FirstPropertyToInit; LocalProp != NULL; LocalProp = (FProperty*)LocalProp->Next) {
		LocalProp->InitializeValue_InContainer(ParamStruct);
	}

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

	// destroy parameter struct
	for (FProperty* P = Stack.CurrentNativeFunction->DestructorLink; P; P = P->DestructorLinkNext) {
		if (!P->IsInContainer(Stack.CurrentNativeFunction->ParmsSize)) {
			P->DestroyValue_InContainer(ParamStruct);
		}
	}
	FMemory::Free(ParamStruct);

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
