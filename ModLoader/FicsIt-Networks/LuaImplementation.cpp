#include "stdafx.h"
#include "LuaImplementation.h"

void ILuaImplementation::luaSetup(ULuaContext * ctx) {}
void ILuaImplementation::luaAddSignalListener(ULuaContext * ctx) {}
void ILuaImplementation::luaRemoveSignalListener(ULuaContext * ctx) {}
SML::Objects::TArray<ULuaContext*> ILuaImplementation::luaGetSignalListeners() {
	return SML::Objects::TArray<ULuaContext*>();
}

bool ILuaImplementation::luaIsReachableFrom(SML::Objects::UObject * listener) {
	return false;
}

void ULuaImplementation::execLuaSetup(SML::Objects::FFrame & stack, void * retVals) {
	ULuaContext* ctx = nullptr;
	stack.stepCompIn(&ctx);
	stack.code += !!stack.code;

	((ILuaImplementation*)this)->luaSetup(ctx);
}

void ULuaImplementation::execAddSignalListener(SML::Objects::FFrame & stack, void * retVals) {
	ULuaContext* ctx = nullptr;
	stack.stepCompIn(&ctx);
	stack.code += !!stack.code;

	((ILuaImplementation*)this)->luaAddSignalListener(ctx);
}

void ULuaImplementation::execRemoveSignalListener(SML::Objects::FFrame & stack, void * retVals) {
	ULuaContext* ctx = nullptr;
	stack.stepCompIn(&ctx);
	stack.code += !!stack.code;

	((ILuaImplementation*)this)->luaRemoveSignalListener(ctx);
}

void ULuaImplementation::execGetSignalListeners(SML::Objects::FFrame & stack, void * retVals) {
	stack.code += !!stack.code;

	*((SML::Objects::TArray<ULuaContext*>*)retVals) = ((ILuaImplementation*)this)->luaGetSignalListeners();
}

void ULuaImplementation::execIsReachableFrom(SML::Objects::FFrame & stack, void * retVals) {
	SML::Objects::UObject* listener = nullptr;
	stack.stepCompIn(&listener);

	stack.code += !!stack.code;

	*(bool*)retVals = ((ILuaImplementation*)this)->luaIsReachableFrom(listener);
}
