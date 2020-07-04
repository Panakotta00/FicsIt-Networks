#pragma once

#include "SharedPointer.h"
#include "CoreMinimal.h"
#include "Signal.h"
#include "FicsItKernel/Processor/Lua/LuaStructs.h"

#include "NetworkFuture.generated.h"

struct FFINNetworkFutureData {
	virtual ~FFINNetworkFutureData() = default;
	virtual int WriteToReader(FicsItKernel::Network::SignalReader& reader) = 0;
};

struct FFINNetworkFutureVoidData : public FFINNetworkFutureData {
	virtual int WriteToReader(FicsItKernel::Network::SignalReader& reader) override {
		return 0;
	}
};

USTRUCT(BlueprintType)
struct FFINNetworkFuture {
	GENERATED_BODY()

public:
	FicsItKernel::Lua::LuaFuture Future;
};
