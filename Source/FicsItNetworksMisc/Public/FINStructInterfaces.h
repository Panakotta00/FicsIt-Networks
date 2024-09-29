#pragma once

#include "CoreMinimal.h"
#include "FGDynamicStruct.h"
#include "FIRGlobalRegisterHelper.h"

#define FIN_STRUCT_INTERFACE(Interface) \
	public: \
	virtual ~Interface() = default; \
	static constexpr const TCHAR* InterfaceName = TEXT(#Interface);

#define FIN_STRUCT_INTERFACE_REGISTER_NAME(Interface) RegisterInterface ## Interface
#define FIN_STRUCT_IMPLEMENT_INTERFACE(Struct, Interface) \
	FFIRStaticGlobalRegisterFunc FIN_STRUCT_INTERFACE_REGISTER_NAME(Interface) ([](){ \
		FFINStructInterfaces::Get().RegisterInterface<Struct, Interface>(); \
	});

struct FFINStructInterfaces {
private:
	FFINStructInterfaces() = default;

	TMap<FName, TMap<UStruct*, uint64>> Offsets;

public:
	static FFINStructInterfaces& Get();

	template<typename S, typename I>
	void RegisterInterface() {
		const S sample;
		const S* samplePtr = &sample;
		const I* interfacePtr = dynamic_cast<const I*>(samplePtr);
		uint64 samplePtr64 = reinterpret_cast<uint64>(samplePtr);
		uint64 interfacePtr64 = reinterpret_cast<uint64>(interfacePtr);
		uint64 offset = interfacePtr64 - samplePtr64;

		Offsets.FindOrAdd(I::InterfaceName).Add(S::StaticStruct(), offset);
	}

	template<typename I>
	bool Implements(UStruct* Struct) const {
		auto structs = Offsets.Find(I::InterfaceName);
		for (const auto& [ structType, _ ] : structs) {
			if (Struct->IsChildOf(structType)) {
				return true;
			}
		}
		return false;
	}

	template<typename I, typename S>
	I* GetInterface(S* Struct) const {
		auto structs = Offsets.Find(I::InterfaceName);
		for (const auto& [ structType, offset ] : structs) {
			if (S::StaticStruct()->IsChildOf(structType)) {
				uint64 structPtr64 = reinterpret_cast<uint64>(Struct);
				uint64 interfacePtr64 = structPtr64 + offset;
				return reinterpret_cast<I>(interfacePtr64);
			}
		}
		return nullptr;
	}

	template<typename I>
	I* GetInterface(const UStruct* StructType, void* Struct) const {
		auto structs = Offsets.Find(I::InterfaceName);
		for (const auto& [ possibleStructType, offset ] : structs) {
			if (StructType->IsChildOf(possibleStructType)) {
				uint64 structPtr64 = reinterpret_cast<uint64>(Struct);
				uint64 interfacePtr64 = structPtr64 + offset;
				return reinterpret_cast<I>(interfacePtr64);
			}
		}
		return nullptr;
	}

	template<typename I>
	I* GetInterface(const FFGDynamicStruct& Struct) const {
		return GetInterface<I>(Struct.GetStruct(), Struct.GetStructValueRaw());
	}
};