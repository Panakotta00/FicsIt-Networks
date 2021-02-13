#include "FINStruct.h"

UFINProperty* UFINStruct::FindFINProperty(const FString& Name, EFINRepPropertyFlags FilterFlags) {
	for (UFINProperty* Property : GetProperties()) {
		if (Property->GetInternalName() == Name && Property->GetPropertyFlags() & FilterFlags) return Property;
	}
	return nullptr;
}

UFINFunction* UFINStruct::FindFINFunction(const FString& Name, EFINFunctionFlags FilterFlags) {
	for (UFINFunction* Function : GetFunctions()) {
		if (Function->GetInternalName() == Name && Function->GetFunctionFlags() & FilterFlags) return Function;
	}
	return nullptr;
}
