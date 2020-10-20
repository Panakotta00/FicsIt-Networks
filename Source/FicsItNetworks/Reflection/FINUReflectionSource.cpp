#include "FINUReflectionSource.h"

#include "FGBuildable.h"
#include "FINFuncProperty.h"
#include "FINReflection.h"

bool UFINUReflectionSource::ProvidesRequirements(UClass* Class) const {
	for (TFieldIterator<UField> Field(Class); Field; ++Field) {
		UProperty* Prop = Cast<UProperty>(*Field);
		UFunction* Func = Cast<UFunction>(*Field);
		if (Prop) {
			if (Field->GetName().StartsWith("netProp_")) return true;
		} else if (Func) {
			if (Field->GetName().StartsWith("netPropGet_")) return true;
			if (Field->GetName().StartsWith("netFunc_")) return true;
			if (Field->GetName().StartsWith("netSig_")) return true;
			if (Field->GetName() == "netDesc" && Cast<UTextProperty>(Func->GetReturnProperty()) && Func->ParmsSize == sizeof(FText)) return true;
		}
	}
	return false;
}

void UFINUReflectionSource::FillData(FFINReflection* Ref, UFINClass* ToFillClass, UClass* Class) const {
	ToFillClass->InternalName = Class->GetName();
	ToFillClass->DisplayName = FText::FromString(ToFillClass->InternalName);

	FFINTypeMeta Meta = GetClassMeta(Class);

	if (Meta.InternalName.Len()) ToFillClass->InternalName = Meta.InternalName;
	if (!Meta.DisplayName.IsEmpty()) ToFillClass->DisplayName = Meta.DisplayName;
	if (!Meta.Description.IsEmpty()) ToFillClass->DisplayName = Meta.Description;

	for (TFieldIterator<UField> Field(Class); Field; ++Field) {
		UProperty* Prop = Cast<UProperty>(*Field);
		UFunction* Func = Cast<UFunction>(*Field);
		if (Prop) {
			if (Field->GetName().StartsWith("netProp_")) {}
		} else if (Func) {
			if (Field->GetName().StartsWith("netPropGet_")) {
			} else if (Field->GetName().StartsWith("netFunc_")) {
				ToFillClass->Functions.Add(GenerateFunction(Class, Func));
			} else if (Field->GetName().StartsWith("netSig_")) {
			} else if (Field->GetName() == "netDesc" && Cast<UTextProperty>(Func->GetReturnProperty()) && Func->ParmsSize == sizeof(FText)) {
				FText Desc;
				Class->GetDefaultObject()->ProcessEvent(Func, &Desc);
				ToFillClass->Description = Desc;
			}
		}
	}

	ToFillClass->Parent = Ref->FindClass(Class->GetOwnerClass());
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
	if (MetaFunc) {
		// allocate parameter space
		uint8* Params = (uint8*)FMemory::Malloc(MetaFunc->PropertiesSize);
		FMemory::Memzero(Params + MetaFunc->ParmsSize, MetaFunc->PropertiesSize - MetaFunc->ParmsSize);
		MetaFunc->InitializeStruct(Params);
		bool bInvalidDeclaration = false;
		for (UProperty* LocalProp = MetaFunc->FirstPropertyToInit; LocalProp != NULL; LocalProp = (UProperty*)LocalProp->Next) {
			LocalProp->InitializeValue_InContainer(Params);
			if (!(LocalProp->PropertyFlags & CPF_OutParm)) bInvalidDeclaration = true;
		}

		if (!bInvalidDeclaration) {
			Class->GetDefaultObject()->ProcessEvent(MetaFunc, Params);

			for (TFieldIterator<UProperty> Property(MetaFunc); Property; ++Property) {
				UTextProperty* TextProp = Cast<UTextProperty>(*Property);
				UStrProperty* StrProp = Cast<UStrProperty>(*Property);
				UMapProperty* MapProp = Cast<UMapProperty>(*Property);
				if (StrProp && Property->GetName() == "InternalName") Meta.InternalName = StrProp->GetPropertyValue_InContainer(Params);
				else if (TextProp && Property->GetName() == "DisplayName") Meta.DisplayName = TextProp->GetPropertyValue_InContainer(Params);
				else if (TextProp && Property->GetName() == "Description") Meta.Description = TextProp->GetPropertyValue_InContainer(Params);
				else if (MapProp && Cast<UStrProperty>(MapProp->KeyProp)) {
					if (Cast<UStrProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyInternalNames") {
						Meta.PropertyInternalNames = *MapProp->ContainerPtrToValuePtr<TMap<FString, FString>>(Params);
						Meta.PropertyDisplayNames.Empty();
						for (const TPair<FString, FString>& InternalName : Meta.PropertyInternalNames) {
							Meta.PropertyDisplayNames.Add(InternalName.Key, FText::FromString(InternalName.Value));
						}
					} else if (Cast<UTextProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyDisplayNames") {
						for (const TPair<FString, FText>& DisplayName : *MapProp->ContainerPtrToValuePtr<TMap<FString, FText>>(Params)) {
							Meta.PropertyDisplayNames.FindOrAdd(DisplayName.Key) = DisplayName.Value;
						}
					} else if (Cast<UTextProperty>(MapProp->ValueProp) && Property->GetName() == "PropertyDescriptions") {
						for (const TPair<FString, FText>& Description : *MapProp->ContainerPtrToValuePtr<TMap<FString, FText>>(Params)) {
							Meta.PropertyDescriptions.FindOrAdd(Description.Key) = Description.Value;
						}
					}
				}
			}
		}
		
		for (UProperty* P = MetaFunc->DestructorLink; P; P = P->DestructorLinkNext) {
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
		for (UProperty* LocalProp = MetaFunc->FirstPropertyToInit; LocalProp != NULL; LocalProp = (UProperty*)LocalProp->Next) {
			LocalProp->InitializeValue_InContainer(Params);
			if (!(LocalProp->PropertyFlags & CPF_OutParm)) bInvalidDeclaration = true;
		}

		if (!bInvalidDeclaration) {
			Class->GetDefaultObject()->ProcessEvent(MetaFunc, Params);

			for (TFieldIterator<UProperty> Property(MetaFunc); Property; ++Property) {
				UTextProperty* TextProp = Cast<UTextProperty>(*Property);
				UStrProperty* StrProp = Cast<UStrProperty>(*Property);
				UArrayProperty* ArrayProp = Cast<UArrayProperty>(*Property);
				if (StrProp && Property->GetName() == "InternalName") Meta.InternalName = StrProp->GetPropertyValue_InContainer(Params);
				else if (TextProp && Property->GetName() == "DisplayName") Meta.DisplayName = TextProp->GetPropertyValue_InContainer(Params);
				else if (TextProp && Property->GetName() == "Description") Meta.Description = TextProp->GetPropertyValue_InContainer(Params);
				else if (ArrayProp && Cast<UStrProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterInternalNames") {
					Meta.ParameterInternalNames = *ArrayProp->ContainerPtrToValuePtr<TArray<FString>>(Params);
					Meta.ParameterDisplayNames.Empty();
					for (const FString& InternalName : Meta.ParameterInternalNames) {
						Meta.ParameterDisplayNames.Add(FText::FromString(InternalName));
					}
				} else if (ArrayProp && Cast<UTextProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterDisplayNames") {
					int i = 0;
					for (const FText& DisplayName : *ArrayProp->ContainerPtrToValuePtr<TArray<FText>>(Params)) {
						if (Meta.ParameterDisplayNames.Num() > i) Meta.ParameterDisplayNames[i] = DisplayName;
						else Meta.ParameterDisplayNames.Add(DisplayName);
						++i;
					}
				} else if (ArrayProp && Cast<UTextProperty>(ArrayProp->Inner) && Property->GetName() == "ParameterDescriptions") {
					int i = 0;
					for (const FText& Description : *ArrayProp->ContainerPtrToValuePtr<TArray<FText>>(Params)) {
						if (Meta.ParameterDescriptions.Num() > i) Meta.ParameterDescriptions[i] = Description;
						else Meta.ParameterDescriptions.Add(Description);
						++i;
					}
				}
			}
		}
	
		for (UProperty* P = MetaFunc->DestructorLink; P; P = P->DestructorLinkNext) {
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

FString UFINUReflectionSource::GetPropertyNameFromUProperty(UProperty* Prop, bool& bReadOnly) const {
	FString Name = Prop->GetName();
	if (!Name.RemoveFromStart("netProp_")) bReadOnly = Name.RemoveFromStart("netPropReadOnly_");
	return Name;
}

UFINFunction* UFINUReflectionSource::GenerateFunction(UClass* Class, UFunction* Func) const {
	FFINFunctionMeta Meta = GetFunctionMeta(Class, Func);
	
	UFINFunction* FINFunc = NewObject<UFINFunction>();
	FINFunc->RefFunction = Func;
	FINFunc->InternalName = GetFunctionNameFromUFunction(Func);
	FINFunc->DisplayName = FText::FromString(FINFunc->InternalName);
	
	if (Meta.InternalName.Len()) FINFunc->InternalName = Meta.InternalName;
	if (!Meta.DisplayName.IsEmpty()) FINFunc->DisplayName = Meta.DisplayName;
	if (!Meta.Description.IsEmpty()) FINFunc->Description = Meta.Description;
	for (TFieldIterator<UProperty> Param(Func); Param; ++Param) {
		if (!(Param->PropertyFlags & CPF_Parm)) continue;
		int i = FINFunc->Parameters.Num();
		UFINProperty* FINProp = FINCreateFINPropertyFromUProperty(*Param);
		FINProp->InternalName = Param->GetName();
		FINProp->DisplayName = FText::FromString(FINProp->InternalName);
		if (Meta.ParameterInternalNames.Num() > i) FINProp->InternalName = Meta.ParameterInternalNames[i];
		if (Meta.ParameterDisplayNames.Num() > i) FINProp->DisplayName = Meta.ParameterDisplayNames[i];
		if (Meta.ParameterDescriptions.Num() > i) FINProp->Description = Meta.ParameterDescriptions[i];
		FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Param;
		FINFunc->Parameters.Add(FINProp);
	}
	return FINFunc;
}

UFINProperty* UFINUReflectionSource::GenerateProperty(const FFINTypeMeta& Meta, UClass* Class, UProperty* Prop) const {
	UFINProperty* FINProp = FINCreateFINPropertyFromUProperty(Prop);
	FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Attrib;
	bool bReadOnly = false;
	FINProp->InternalName = GetPropertyNameFromUProperty(Prop, bReadOnly);
	if (bReadOnly) FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_ReadOnly;
	if (Meta.PropertyInternalNames.Contains(FINProp->GetInternalName())) FINProp->InternalName = Meta.PropertyInternalNames[FINProp->GetInternalName()];
	if (Meta.PropertyDisplayNames.Contains(FINProp->GetInternalName())) FINProp->DisplayName = Meta.PropertyDisplayNames[FINProp->GetInternalName()];
	if (Meta.PropertyDescriptions.Contains(FINProp->GetInternalName())) FINProp->Description = Meta.PropertyDescriptions[FINProp->GetInternalName()];
	return FINProp;
}

UFINProperty* UFINUReflectionSource::GenerateProperty(const FFINTypeMeta& Meta, UClass* Class, UFunction* Get) const {
	UProperty* GetProp = nullptr;
	for (TFieldIterator<UProperty> Param(Get); Param; ++Param) {
		if (Param->PropertyFlags & CPF_Parm) {
			check(Param->PropertyFlags & CPF_OutParm);
			check(GetProp == nullptr);
			GetProp = *Param;
		}
	}
	UFINProperty* FINProp = FINCreateFINPropertyFromUProperty(GetProp, nullptr);
	FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_Attrib;
	FINProp->InternalName = GetPropertyNameFromUFunction(Get);
	if (UFINFuncProperty* FINSProp = Cast<UFINFuncProperty>(FINProp)) {
		FINSProp->GetterFunc.Function = Get;
		FINSProp->GetterFunc.Property = FINCreateFINPropertyFromUProperty(GetProp);
	}
	UFunction* Set = Class->FindFunctionByName(*(FString("netPropSet_") + FINProp->InternalName));
	if (Set) {
		UProperty* SetProp = nullptr;
		for (TFieldIterator<UProperty> Param(Get); Param; ++Param) {
			if (Param->PropertyFlags & CPF_Parm) {
				check(!(Param->PropertyFlags & CPF_OutParm));
				check(SetProp == nullptr);
				check(Param->GetClass() == GetProp->GetClass());
				SetProp = *Param;
			}
		}
		if (UFINFuncProperty* FINSProp = Cast<UFINFuncProperty>(FINProp)) {
			FINSProp->SetterFunc.Function = Set;
			FINSProp->SetterFunc.Property = FINCreateFINPropertyFromUProperty(SetProp);
		}
	} else {
		FINProp->PropertyFlags = FINProp->PropertyFlags | FIN_Prop_ReadOnly;
	}
	if (Meta.PropertyInternalNames.Contains(FINProp->GetInternalName())) FINProp->InternalName = Meta.PropertyInternalNames[FINProp->GetInternalName()];
	if (Meta.PropertyDisplayNames.Contains(FINProp->GetInternalName())) FINProp->DisplayName = Meta.PropertyDisplayNames[FINProp->GetInternalName()];
	if (Meta.PropertyDescriptions.Contains(FINProp->GetInternalName())) FINProp->Description = Meta.PropertyDescriptions[FINProp->GetInternalName()];
	
	return FINProp;
}
