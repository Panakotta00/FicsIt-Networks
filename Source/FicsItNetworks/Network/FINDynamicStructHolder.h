#pragma once

#include "CoreMinimal.h"
#include "FINDynamicStructHolder.generated.h"

#define MakeDynamicStruct(Type, ...) MakeShared<FDynamicStructHolder>(Type::StaticStruct(), new Type{__VA_ARGS__})

/**
 * This structure allows you to store any kind of UStruct
 */
USTRUCT()
struct FFINDynamicStructHolder {
	GENERATED_BODY()
	
private:
	void* Data = nullptr;
	UStruct* Struct = nullptr;

public:
	FFINDynamicStructHolder();
	FFINDynamicStructHolder(UStruct* Struct);
	FFINDynamicStructHolder(UStruct* Struct, void* Data);
	FFINDynamicStructHolder(const FFINDynamicStructHolder& Other);
	~FFINDynamicStructHolder();
	FFINDynamicStructHolder& operator=(const FFINDynamicStructHolder& Other);
	
	bool Serialize(FArchive& Ar);

	UStruct* GetStruct() const;
	void* GetData() const;

	template<typename T>
    T& Get() {
		return *static_cast<T*>(GetData());
	}
};

inline void operator<<(FArchive& Ar, FFINDynamicStructHolder& Struct) {
	Struct.Serialize(Ar);
}