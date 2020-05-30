#pragma once

#include "CoreMinimal.h"

#include "FicsItKernel/Network/NetworkTrace.h"
#include "Lua.h"

#include <string>
#include <vector>
#include <map>
#include <functional>

#include "SubclassOf.h"

namespace FicsItKernel {
	namespace Lua {
		typedef std::function<int(lua_State*, int, const Network::NetworkTrace&)> LuaLibFunc;
		typedef std::function<int(lua_State*, int, UClass*)> LuaLibClassFunc;

		std::map<UClass*, std::map<std::string, LuaLibFunc>>& instanceClasses();
		std::map<UClass*, std::map<std::string, LuaLibClassFunc>>& instanceSubclasses();

		typedef Network::NetworkTrace LuaInstance;
		typedef UClass* LuaClassInstance;
		typedef std::pair<LuaInstance, std::pair<std::pair<UClass*, std::string>, LuaLibFunc>> LuaInstanceFunc;
		typedef std::pair<LuaInstance, UFunction*> LuaInstanceUFunc;
		typedef std::pair<LuaClassInstance, std::pair<std::string, LuaLibClassFunc>> LuaClassInstanceFunc;

		bool newInstance(lua_State* L, Network::NetworkTrace obj);
		bool newInstance(lua_State* L, UClass* clazz);

		Network::NetworkTrace getObjInstance(lua_State* L, int index, UClass* clazz);

		template<typename T>
		inline T* getObjInstance(lua_State* L, int index, Network::NetworkTrace* trace = nullptr) {
			auto obj = getObjInstance(L, index, T::StaticClass());
			if (trace) *trace = obj;
			return Cast<T>(*obj);
		}

		UClass* getClassInstance(lua_State* L, int index, UClass* clazz);

		template<typename T>
		inline TSubclassOf<T> getClassInstance(lua_State* L, int index) {
			return getClassInstance(L, index, T::StaticClass());
		}

		void setupInstanceSystem(lua_State* L);
	}
}
