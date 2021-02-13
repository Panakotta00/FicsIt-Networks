#pragma once

#include "CoreMinimal.h"
#include "FINDynamicStructHolder.generated.h"

#define MakeDynamicStruct(Type, ...) MakeShared<FFINDynamicStructHolder>(TBaseStructure<Type>::Get(), new Type{__VA_ARGS__})

template<typename T>
class TFINDynamicStruct;

/**
 * This structure allows you to store any kind of UStruct
 */
USTRUCT(BlueprintType)
struct FICSITNETWORKS_API FFINDynamicStructHolder {
	GENERATED_BODY()
	
protected:
	void* Data = nullptr;
	UScriptStruct* Struct = nullptr;

public:
	FFINDynamicStructHolder();
	FFINDynamicStructHolder(UScriptStruct* Struct);
	FFINDynamicStructHolder(UScriptStruct* Struct, void* Data);
	FFINDynamicStructHolder(const FFINDynamicStructHolder& Other);
	~FFINDynamicStructHolder();
	FFINDynamicStructHolder& operator=(const FFINDynamicStructHolder& Other);

	template<typename T>
	FFINDynamicStructHolder(const T& Struct) : FFINDynamicStructHolder(Copy(TBaseStructure<T>::Get(), &Struct)) {}

	static FFINDynamicStructHolder Copy(UScriptStruct* Struct, const void* Data);
	
	bool Serialize(FArchive& Ar);

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
	operator TFINDynamicStruct<T>() const {
		return TFINDynamicStruct<T>(*this);
	}
};

template<>
struct TStructOpsTypeTraits<FFINDynamicStructHolder> : public TStructOpsTypeTraitsBase2<FFINDynamicStructHolder>
{
	enum
	{
		WithSerializer = true,
    };
};

inline void operator<<(FArchive& Ar, FFINDynamicStructHolder& Struct) {
	Struct.Serialize(Ar);
}

template<typename T>
class TFINDynamicStruct : public FFINDynamicStructHolder {
public:
	TFINDynamicStruct() : FFINDynamicStructHolder(TBaseStructure<T>::Get()) {}
	TFINDynamicStruct(UScriptStruct* Struct) : FFINDynamicStructHolder(Struct) { check(Struct->IsChildOf(TBaseStructure<T>::Get())) }
	TFINDynamicStruct(UScriptStruct* Struct, void* Data) : FFINDynamicStructHolder(Struct, Data) { check(Struct->IsChildOf(TBaseStructure<T>::Get())) }
	template<typename K>
	TFINDynamicStruct(const TFINDynamicStruct<K>& Other) : FFINDynamicStructHolder(FFINDynamicStructHolder::Copy(Other.GetStruct(), Other.GetData())) {
		check(Other.GetStruct()->IsChildOf(TBaseStructure<T>::Get()));
	}
	TFINDynamicStruct(const FFINDynamicStructHolder& Other) : FFINDynamicStructHolder(FFINDynamicStructHolder::Copy(Other.GetStruct(), Other.GetData())) {
		check(Other.GetStruct()->IsChildOf(TBaseStructure<T>::Get()));
	}
	template<typename K>
    TFINDynamicStruct(const K& Other) : FFINDynamicStructHolder(FFINDynamicStructHolder::Copy(TBaseStructure<K>::Get(), &Other)) {
		check(TBaseStructure<K>::Get()->IsChildOf(TBaseStructure<T>::Get()));
	}

	template<typename K>
	TFINDynamicStruct<T>& operator=(const TFINDynamicStruct<K>& Other) {
		check(Other.GetStruct()->IsChildOf(TBaseStructure<T>::Get()));
		FFINDynamicStructHolder::operator=(Other);
		return *this;
	}
	
	TFINDynamicStruct<T>& operator=(const FFINDynamicStructHolder& Other) {
		check(Other.GetStruct()->IsChildOf(TBaseStructure<T>::Get()));
		FFINDynamicStructHolder::operator=(Other);
		return *this;
	}
	
	T* operator->() const {
		return &Get<T>();
	}

	T* operator*() const {
		return &Get<T>();
	}

    TSharedPtr<T> SharedCopy() {
		return FFINDynamicStructHolder::SharedCopy<T>();
	}

	operator FFINDynamicStructHolder() const {
		return FFINDynamicStructHolder::Copy(Struct, Data);
	}
};

