#pragma once

#include "CoreMinimal.h"
#include "FIRInstancedStruct.generated.h"

#define FINMakeDynamicStruct(Type, ...) MakeShared<FFIRInstancedStruct>(TBaseStructure<Type>::Get(), new Type{__VA_ARGS__})

template<typename T>
class TFIRInstancedStruct;

/**
 * This structure allows you to store any kind of UStruct
 */
USTRUCT(BlueprintType)
struct FICSITREFLECTION_API FFIRInstancedStruct {
	GENERATED_BODY()
	
protected:
	void* Data = nullptr;
	UScriptStruct* Struct = nullptr;

public:
	FFIRInstancedStruct();
	FFIRInstancedStruct(UScriptStruct* Struct);
	FFIRInstancedStruct(UScriptStruct* Struct, void* Data);
	FFIRInstancedStruct(const FFIRInstancedStruct& Other);
	~FFIRInstancedStruct();
	FFIRInstancedStruct& operator=(const FFIRInstancedStruct& Other);

	template<typename T>
	FFIRInstancedStruct(const T& Struct) : FFIRInstancedStruct(Copy(TBaseStructure<T>::Get(), &Struct)) {}

	static FFIRInstancedStruct Copy(UScriptStruct* Struct, const void* Data);

	bool Serialize(FStructuredArchive::FSlot Slot);
	//bool NetSerialize( FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	void AddStructReferencedObjects(FReferenceCollector& ReferenceCollector) const;

	/**
	 * Returns the struct type stored in this holder.
	 *
	 * @return the stored structs type
	 */
	UScriptStruct* GetStruct() const;
	
	void* GetData() const;

	template<typename T>
    T& Get() const {
		return *static_cast<T*>(GetData());
	}

	template<typename T>
	T* GetPtr() const {
		if (!Struct->IsChildOf(TBaseStructure<T>::Get())) return nullptr;
		return &Get<T>();
	}

	template<typename T>
	TSharedPtr<T> SharedCopy() const {
		if (Struct->IsChildOf(TBaseStructure<T>::Get())) {
			void* Data = FMemory::Malloc(Struct->GetStructureSize());
			Struct->InitializeStruct(Data);
			Struct->CopyScriptStruct(Data, this->Data);
			return MakeShareable(reinterpret_cast<T*>(Data));
		}
		return nullptr;
	}

	template<typename T>
	operator TFIRInstancedStruct<T>() const {
		return TFIRInstancedStruct<T>(*this);
	}
};

FORCEINLINE void operator<<(FStructuredArchive::FSlot Slot, FFIRInstancedStruct& DynamicStruct) {
	DynamicStruct.Serialize(Slot);
}

template<>
struct TStructOpsTypeTraits<FFIRInstancedStruct> : public TStructOpsTypeTraitsBase2<FFIRInstancedStruct>
{
	enum
	{
		WithStructuredSerializer = true,
		//WithNetSerializer = true,
		WithAddStructReferencedObjects = true,
        WithCopy = true,
    };
};

template<typename T>
class TFIRInstancedStruct : public FFIRInstancedStruct {
public:
	TFIRInstancedStruct() : FFIRInstancedStruct(TBaseStructure<T>::Get()) {}
	TFIRInstancedStruct(UScriptStruct* Struct) : FFIRInstancedStruct(Struct) { check(Struct->IsChildOf(TBaseStructure<T>::Get())) }
	TFIRInstancedStruct(UScriptStruct* Struct, void* Data) : FFIRInstancedStruct(Struct, Data) { check(Struct->IsChildOf(TBaseStructure<T>::Get())) }
	template<typename K>
	TFIRInstancedStruct(const TFIRInstancedStruct<K>& Other) : FFIRInstancedStruct(FFIRInstancedStruct::Copy(Other.GetStruct(), Other.GetData())) {
		check(Other.GetStruct()->IsChildOf(TBaseStructure<T>::Get()));
	}
	TFIRInstancedStruct(const FFIRInstancedStruct& Other) : FFIRInstancedStruct(FFIRInstancedStruct::Copy(Other.GetStruct(), Other.GetData())) {
		check(Other.GetStruct()->IsChildOf(TBaseStructure<T>::Get()));
	}
	template<typename K>
    TFIRInstancedStruct(const K& Other) : FFIRInstancedStruct(FFIRInstancedStruct::Copy(TBaseStructure<K>::Get(), &Other)) {
		check(TBaseStructure<K>::Get()->IsChildOf(TBaseStructure<T>::Get()));
	}

	template<typename K>
	TFIRInstancedStruct<T>& operator=(const TFIRInstancedStruct<K>& Other) {
		check(Other.GetStruct()->IsChildOf(TBaseStructure<T>::Get()));
		FFIRInstancedStruct::operator=(Other);
		return *this;
	}
	
	TFIRInstancedStruct<T>& operator=(const FFIRInstancedStruct& Other) {
		check(Other.GetStruct()->IsChildOf(TBaseStructure<T>::Get()));
		FFIRInstancedStruct::operator=(Other);
		return *this;
	}
	
	T* operator->() const {
		return &Get<T>();
	}

	T* operator*() const {
		return &Get<T>();
	}

    TSharedPtr<T> SharedCopy() {
		return FFIRInstancedStruct::SharedCopy<T>();
	}

	operator FFIRInstancedStruct() const {
		return FFIRInstancedStruct::Copy(Struct, Data);
	}
};

