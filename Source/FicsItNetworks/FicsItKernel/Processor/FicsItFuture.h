#pragma once

#include "CoreMinimal.h"
#include "Archive.h"

#define PtrFuncVarName(PtrName) PtrName ## _Name
#define FuturePersistPtr(PtrName) \
	FString PtrFuncVarName(PtrName) = FicsItKernel::FuturePersistencyHelper::Get().FindName(PtrName); \
	Ar << PtrFuncVarName(PtrName); \
	PtrName = static_cast<decltype(PtrName)>(FicsItKernel::FuturePersistencyHelper::Get().FindPtr(PtrFuncVarName(PtrName)));
#define RegisterPointerName(PtrName) PtrName ## _Registerer
#define RegisterFuturePointer(PtrName, Ptr) \
	FicsItKernel::FuturePointerRegisterer RegisterPointerName(PtrName) (#PtrName, Ptr);

namespace FicsItKernel {
	/**
	 * Base class for all Kernel Futures which will retrieve data in the main thread.
	 */
	class FicsItFuture {
	public:
		virtual ~FicsItFuture();

		/**
		 * This function will get called from the FicsIt-Kernel in the main thread if it is added to the
		 * future queue of the kernel.
		 */
		virtual void Excecute() = 0;
    };

	class FuturePersistencyHelper {
	private:
		TMap<FString, void*> NameToPtr;
		TMap<void*, FString> PtrToName;

		FuturePersistencyHelper() = default;

	public:
		/**
		 * Returns the instance of the singleton.
		 *
		 * @return the instance
		 */
		static FuturePersistencyHelper& Get();
		
		/**
		 * Adds a new ptr to the persist data
		 *
		 * @param[in]	name	the name of the ptr
		 * @param[in]	ptr		the ptr
		 */
		void RegisterPtr(const FString& name, void* ptr);

		/**
		 * Searches for the ptr with the given name.
		 *
		 * @param[in]	name	the name of the ptr you want to find
		 * @return	the ptr found, nullptr if not found
		 */
		void* FindPtr(const FString& name);

		/**
		 * Searches for the name of the given ptr
		 *
		 * @param[in]	ptr		the ptr you want to get the name for
		 * @return	the name of the ptr, empty if no name is found
		 */
		FString FindName(void* ptr);
	};

	struct FuturePointerRegisterer {
		FuturePointerRegisterer(FString name, void* ptr) {
			FuturePersistencyHelper::Get().RegisterPtr(name, ptr);
		}
	};
}
