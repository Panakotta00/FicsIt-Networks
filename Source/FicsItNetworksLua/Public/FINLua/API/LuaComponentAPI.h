#pragma once

#include "CoreMinimal.h"
#include "LuaUtil.h"
#include "FIRTrace.h"

class FICSITNETWORKSLUA_API IFINLuaComponentNetworkAccessInterface {
public:
	virtual ~IFINLuaComponentNetworkAccessInterface() = default;
	/**
	 * tries to find a component with the given ID.
	 *
	 * @return	the component you searched for, nullptr if it was not able to find the component
	 */
	virtual FFIRTrace GetComponentByID(const FGuid& InID) const { return FFIRTrace(); }

	/**
	 * returns the components in the network with the given nick.
	 */
	virtual TSet<FFIRTrace> GetComponentByNick(const FString& InNick) const { return {}; }

	/**
	 * returns the components in the network with of the given type.
	 */
	virtual TSet<FFIRTrace> GetComponentByClass(UClass* InClass, bool bInRedirect) const { return {}; }
};

struct FICSITNETWORKSLUA_API FFINLuaComponentNetworkAccessDelegates : public IFINLuaComponentNetworkAccessInterface {
	TDelegate<FFIRTrace(const FGuid&)> OnGetComponentByID;
	TDelegate<TSet<FFIRTrace>(const FString&)> OnGetComponentByNick;
	TDelegate<TSet<FFIRTrace>(UClass* Class, bool bInRedirect)> OnGetComponentByClass;

	virtual FFIRTrace GetComponentByID(const FGuid& InID) const override {
		return OnGetComponentByID.Execute(InID);
	}

	virtual TSet<FFIRTrace> GetComponentByNick(const FString& InNick) const override {
		return OnGetComponentByNick.Execute(InNick);
	}

	virtual TSet<FFIRTrace> GetComponentByClass(UClass* InClass, bool bInRedirect) const {
		return OnGetComponentByClass.Execute(InClass, bInRedirect);
	}
};

namespace FINLua {
	FICSITNETWORKSLUA_API void luaFIN_setComponentNetwork(lua_State* L, IFINLuaComponentNetworkAccessInterface* ComponentNetwork);
	FICSITNETWORKSLUA_API IFINLuaComponentNetworkAccessInterface* luaFIN_getComponentNetwork(lua_State* L);
}
