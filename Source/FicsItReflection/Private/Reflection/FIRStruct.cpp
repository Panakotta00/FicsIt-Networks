#include "Reflection/FIRStruct.h"

#include "tracy/Tracy.hpp"

UFIRProperty* UFIRStruct::FindFIRProperty(const FString& Name, EFIRPropertyFlags FilterFlags) {
	ZoneScoped;
	FRWScopeLock nameCacheLock(NameCacheMutex, SLT_ReadOnly);
	if (Name2Property.IsEmpty()) {
		nameCacheLock.ReleaseReadOnlyLockAndAcquireWriteLock_USE_WITH_CAUTION();
		// USE_WITH_CAUTION clarification:
		// Second check if other thread filled this map while we were between ReadUnlock and WriteLock.
		if (Name2Property.IsEmpty()) {
			for (UFIRProperty* Property : GetProperties()) {
				Name2Property.FindOrAdd(Property->GetInternalName()).Add(Property);
			}
		}
	}
	TArray<UFIRProperty*>* Props = Name2Property.Find(Name);
	if (Props) for (UFIRProperty* Property : *Props) {
		if (Property->GetPropertyFlags() & FilterFlags) return Property;
	}
	return nullptr;
}

UFIRFunction* UFIRStruct::FindFIRFunction(const FString& Name, EFIRFunctionFlags FilterFlags) {
	ZoneScoped;
	FRWScopeLock nameCacheLock(NameCacheMutex, SLT_ReadOnly);
	if (Name2Function.IsEmpty()) {
		nameCacheLock.ReleaseReadOnlyLockAndAcquireWriteLock_USE_WITH_CAUTION();
		// USE_WITH_CAUTION clarification:
		// Second check if other thread filled this map while we were between ReadUnlock and WriteLock.
		if (Name2Function.IsEmpty()) {
			for (UFIRFunction* Function : GetFunctions()) {
				Name2Function.FindOrAdd(Function->GetInternalName()).Add(Function);
			}
		}
	}
	TArray<UFIRFunction*>* Funcs = Name2Function.Find(Name);
	if (Funcs) for (UFIRFunction* Function : *Funcs) {
		if (Function->GetFunctionFlags() & FilterFlags) return Function;
	}
	return nullptr;
}
