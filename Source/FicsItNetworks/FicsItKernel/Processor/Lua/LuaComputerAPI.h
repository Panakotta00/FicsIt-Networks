#pragma once

#include "Lua.h"

#include "FicsItKernel/FicsItKernel.h"

#include "LuaComputerAPI.generated.h"

namespace FicsItKernel {
	namespace Lua {
		/**
		* Adds the Computer API to the top stack entry if it is a table in the given Lua state.
		*
		* @param[in]	L	The lua state you want to add the Computer API to. Make sure the top stack entry is a table.
		*/
		void setupComputerAPI(lua_State* L);
	}
}

USTRUCT()
struct FFINKernelFutureData {
	GENERATED_BODY()

	FicsItKernel::KernelSystem* kernel = nullptr;
	
	FFINKernelFutureData() = default;
	FFINKernelFutureData(FicsItKernel::KernelSystem* kernel) : kernel(kernel) {}

	inline bool Serialize(FArchive& Ar) {
		return true;
	}
};

inline void operator<<(FArchive& Ar, FFINKernelFutureData& InData) {
	InData.Serialize(Ar);
}
