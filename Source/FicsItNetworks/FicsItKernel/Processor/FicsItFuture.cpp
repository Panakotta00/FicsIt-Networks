#include "FicsItFuture.h"

namespace FicsItKernel {
	FicsItFuture::~FicsItFuture() {}

	FuturePersistencyHelper& FuturePersistencyHelper::Get() {
		static FuturePersistencyHelper* instance = nullptr;
		if (!instance) instance = new FuturePersistencyHelper();
		return *instance;
	}

	void FuturePersistencyHelper::RegisterPtr(const FString& name, void* ptr) {
		PtrToName.Add(ptr, name);
		NameToPtr.Add(name, ptr);
	}

	void* FuturePersistencyHelper::FindPtr(const FString& name) {
		void** ptr = NameToPtr.Find(name);
		if (!ptr) return nullptr;
		return *ptr;
	}

	FString FuturePersistencyHelper::FindName(void* ptr) {
		FString* name = PtrToName.Find(ptr);
		if (!name) return "";
		return *name;
	}
}
