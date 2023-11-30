#include "Reflection/FINFunction.h"
//#include "tracy/Tracy.hpp"

TArray<FFINAnyNetworkValue> UFINFunction::Execute(const FFINExecutionContext& Ctx, const TArray<FFINAnyNetworkValue>& Params) const {
//	ZoneScopedN("FINFunction Execute");
	if (NativeFunction) return NativeFunction(Ctx, Params);
	return TArray<FFINAnyNetworkValue>();
}
