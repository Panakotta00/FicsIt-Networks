#include "Reflection/FINStruct.h"

#include "tracy/Tracy.hpp"

UFINProperty* UFINStruct::FindFINProperty(const FString& Name, EFINRepPropertyFlags FilterFlags) {
	ZoneScoped;
	if (Name2Property.IsEmpty()) {
		for (UFINProperty* Property : GetProperties()) {
			Name2Property.FindOrAdd(Property->GetInternalName()).Add(Property);
		}
	}
	TArray<UFINProperty*>* Props = Name2Property.Find(Name);
	if (Props) for (UFINProperty* Property : *Props) {
		if (Property->GetPropertyFlags() & FilterFlags) return Property;
	}
	return nullptr;
}

UFINFunction* UFINStruct::FindFINFunction(const FString& Name, EFINFunctionFlags FilterFlags) {
	ZoneScoped;
	if (Name2Function.IsEmpty()) {
		for (UFINFunction* Function : GetFunctions()) {
			Name2Function.FindOrAdd(Function->GetInternalName()).Add(Function);
		}
	}
	TArray<UFINFunction*>* Funcs = Name2Function.Find(Name);
	if (Funcs) for (UFINFunction* Function : *Funcs) {
		if (Function->GetFunctionFlags() & FilterFlags) return Function;
	}
	return nullptr;
}
