#pragma once

#include "CoreMinimal.h"
#include "FINDynamicStructHolder.generated.h"

#define MakeDynamicStruct(Type, ...) MakeShared<FFINDynamicStructHolder>(Type::StaticStruct(), new Type{__VA_ARGS__})

/**
 * This structure allows you to store any kind of UStruct
 */
USTRUCT(BlueprintType)
struct FFINDynamicStructHolder {
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
		if (Struct->IsChildOf(T::StaticStruct())) {
			void* Data = FMemory::Malloc(Struct->GetStructureSize());
			Struct->InitializeStruct(Data);
			Struct->CopyScriptStruct(Data, this->Data);
			return MakeShareable(reinterpret_cast<T*>(Data));
		}
		return nullptr;
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
	TFINDynamicStruct() : FFINDynamicStructHolder(T::StaticStruct()) {}
	TFINDynamicStruct(void* Data) : FFINDynamicStructHolder(T::StaticStruct(), Data) {}
	TFINDynamicStruct(UScriptStruct* Struct) : FFINDynamicStructHolder(Struct) { check(Struct->IsChildOf(T::StaticStruct())) }
	TFINDynamicStruct(UScriptStruct* Struct, void* Data) : FFINDynamicStructHolder(Struct, Data) { check(Struct->IsChildOf(T::StaticStruct())) }
	TFINDynamicStruct(const FFINDynamicStructHolder& Other) : FFINDynamicStructHolder(Other) { check(Other.GetStruct()->IsChildOf(T::StaticStruct())) }
	TFINDynamicStruct(const T& Other) : FFINDynamicStructHolder(FFINDynamicStructHolder::Copy(T::StaticStruct(), &Other)) {}

	TFINDynamicStruct<T>& operator=(const FFINDynamicStructHolder& Other) {
		check(Other.GetStruct()->IsChildOf(T::StaticStruct()));
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
};
