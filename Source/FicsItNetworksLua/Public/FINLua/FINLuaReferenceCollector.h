#pragma once

#include "CoreMinimal.h"
#include "FINLuaRuntime.h"
#include "FINLuaReferenceCollector.generated.h"

UE_DISABLE_OPTIMIZATION_SHIP
USTRUCT()
struct FFINLuaReferenceCollector {
	GENERATED_BODY()
private:
	FCriticalSection ReferenceCollectorsMutex;
	TMap<void*, TFunction<void(void*, FReferenceCollector&)>> ReferencedCollectors;

public:
	FFINLuaReferenceCollector() = default;
	FFINLuaReferenceCollector(const FFINLuaReferenceCollector&) = delete;
	FFINLuaReferenceCollector(FFINLuaReferenceCollector&&) = delete;
	FFINLuaReferenceCollector& operator=(const FFINLuaReferenceCollector&) = delete;
	FFINLuaReferenceCollector& operator=(FFINLuaReferenceCollector&&) = delete;

	void AddStructReferencedObjects(FReferenceCollector& ReferenceCollector) {
		ReferenceCollector.AddPropertyReferences(StaticStruct(), this);

		FScopeLock Lock(&ReferenceCollectorsMutex);
		for (const auto& [instance, func] : ReferencedCollectors) {
			func(instance, ReferenceCollector);
		}
	}

	void AddReferencer(void* Referencer, const TFunction<void(void*, FReferenceCollector&)>& CollectorFunc) {
		FScopeLock Lock(&ReferenceCollectorsMutex);
		ReferencedCollectors.FindOrAdd(Referencer) = CollectorFunc;
	}

	void RemoveReferencer(void* Referencer) {
		FScopeLock Lock(&ReferenceCollectorsMutex);
		ReferencedCollectors.Remove(Referencer);
	}
};
UE_ENABLE_OPTIMIZATION_SHIP

template<>
struct TStructOpsTypeTraits<FFINLuaReferenceCollector> : TStructOpsTypeTraitsBase2<FFINLuaReferenceCollector> {
	enum {
		WithAddStructReferencedObjects = true,
		WithCopy = false,
	};
};
UE_DISABLE_OPTIMIZATION_SHIP
USTRUCT()
struct FFINLuaReferenceCollected {
	GENERATED_BODY()
public:
	FFINLuaReferenceCollector* ReferenceCollector = nullptr;

	static void CollectReferencesInternal(void* self, FReferenceCollector& Collector) {
		((FFINLuaReferenceCollected*)self)->CollectReferences(Collector);
	}

public:
	FFINLuaReferenceCollected() = default;
	FFINLuaReferenceCollected(FFINLuaReferenceCollector* ReferenceCollector) : ReferenceCollector(ReferenceCollector) {
		fgcheck(ReferenceCollector);
		ReferenceCollector->AddReferencer(this, &CollectReferencesInternal);
	}
	FFINLuaReferenceCollected(const FFINLuaReferenceCollected& Other) : ReferenceCollector(Other.ReferenceCollector) {
		fgcheck(ReferenceCollector);
    	ReferenceCollector->AddReferencer(this, &CollectReferencesInternal);
	}
	FFINLuaReferenceCollected(FFINLuaReferenceCollected&&) = delete;
	FFINLuaReferenceCollected& operator=(const FFINLuaReferenceCollected& Other) {
		if (ReferenceCollector) {
    		ReferenceCollector->RemoveReferencer(this);
    	}
    	ReferenceCollector = Other.ReferenceCollector;
    	fgcheck(ReferenceCollector);
    	ReferenceCollector->AddReferencer(this, &CollectReferencesInternal);
    	return *this;
	}
	FFINLuaReferenceCollected& operator=(FFINLuaReferenceCollected&&) = delete;

	virtual ~FFINLuaReferenceCollected() {
		if (ReferenceCollector) ReferenceCollector->RemoveReferencer(this);
	}

	virtual void CollectReferences(FReferenceCollector& Collector) {}
};
UE_ENABLE_OPTIMIZATION_SHIP
template<>
struct TStructOpsTypeTraits<FFINLuaReferenceCollected> : TStructOpsTypeTraitsBase2<FFINLuaReferenceCollected> {
	enum {
		WithCopy = true,
	};
};

namespace FINLua {
	FORCEINLINE void luaFIN_setReferenceCollector(lua_State* L, FFINLuaReferenceCollector& ReferenceCollector) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		runtime.GlobalPointers.Add(TEXT("ReferenceCollector"), &ReferenceCollector);
	}

	FORCEINLINE FFINLuaReferenceCollector* luaFIN_getReferenceCollector(lua_State* L) {
		FFINLuaRuntime& runtime = luaFIN_getRuntime(L);
		FFINLuaReferenceCollector** collector = reinterpret_cast<FFINLuaReferenceCollector**>(runtime.GlobalPointers.Find(TEXT("ReferenceCollector")));
		if (!collector) return nullptr;
		return *collector;
	}
}
