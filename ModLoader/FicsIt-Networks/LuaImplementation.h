#pragma once

#include <assets/BPInterface.h>
#include <util/Objects/TArray.h>

class ULuaContext;

struct LuaIsReachableFromParams {
	bool is;
	SML::Objects::UObject* listener;
};

class ILuaImplementation {
public:
	virtual void luaSetup(ULuaContext* ctx);
	virtual void luaAddSignalListener(ULuaContext* ctx);
	virtual void luaRemoveSignalListener(ULuaContext* ctx);
	virtual SML::Objects::TArray<ULuaContext*> luaGetSignalListeners();
	virtual bool luaIsReachableFrom(SML::Objects::UObject* listener);
};

class ULuaImplementation : public SDK::UInterface {
public:
	void execLuaSetup(SML::Objects::FFrame& stack, void* retVals);
	void execAddSignalListener(SML::Objects::FFrame& stack, void* retVals);
	void execRemoveSignalListener(SML::Objects::FFrame& stack, void* retVals);
	void execGetSignalListeners(SML::Objects::FFrame& stack, void* retVals);
	void execIsReachableFrom(SML::Objects::FFrame& stack, void* retVals);

	static inline SML::Objects::UClass* staticClass() {
		return SML::Paks::ClassBuilder<ULuaImplementation>::staticClass();
	}
};