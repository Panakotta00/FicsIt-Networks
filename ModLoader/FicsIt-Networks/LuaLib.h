#pragma once

#include <string>
#include <queue>
#include <functional>
#include <chrono>
#include <map>
#include <set>

#include <util/Objects/UClass.h>
#include <util/Objects/SmartPointer.h>
#include <util/Objects/Delegate.h>

#include "LuaException.h"

extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

#include "FileSystem.h"

enum LuaDataType {
	LUA_NIL,
	LUA_STR,
	LUA_INT,
	LUA_NUM,
	LUA_BOOL,
	LUA_TBL,
	LUA_OBJ,
};

class ULuaContext;

struct FactoryHook {
	size_t refs = 1;
	std::queue<std::chrono::high_resolution_clock::time_point> iperm;
	std::set<ULuaContext*> deleg;
	void update();
};

extern std::map<SML::Objects::FWeakObjectPtr, FactoryHook> hooks;

struct LuaObjectPtr {
protected:
	SML::Objects::FWeakObjectPtr ptr;

public:
	LuaObjectPtr(SML::Objects::UObject* obj);
	virtual ~LuaObjectPtr();

	virtual SML::Objects::UObject* getObject() const;
};

struct LuaComponentPtr : public LuaObjectPtr {
protected:
	SML::Objects::FWeakObjectPtr comp;

public:
	LuaComponentPtr(SML::Objects::UObject* obj, SML::Objects::UObject* component);
	virtual ~LuaComponentPtr();

	virtual SML::Objects::UObject* getObject() const override;
	virtual SML::Objects::UObject* getComponent() const;
};

struct LuaFile {
	std::unique_ptr<FileSystemFileStream> file;
};

struct LuaClass {
	LuaObjectPtr* ptr;

	LuaClass(SML::Objects::UObject* obj, SML::Objects::UObject* component = nullptr);
	~LuaClass();
};

struct LuaClassFunc : public LuaClass {
	std::uint16_t func;

	LuaClassFunc(SML::Objects::UObject* obj, std::uint16_t func, SML::Objects::UObject* component = nullptr);
};

struct LuaClassUFunc : public LuaClass {
	SML::Objects::UFunction* func;

	LuaClassUFunc(SML::Objects::UObject* obj, SML::Objects::UFunction* func, SML::Objects::UObject* component = nullptr);
};

void luaInit();

struct LuaFuncContext {
	SML::Objects::UObject* obj;
	SML::Objects::UObject* compObj;
	INetworkComponent* compImpl;

	LuaFuncContext(SML::Objects::UObject* obj, SML::Objects::UObject* comp = nullptr);

	SML::Objects::UObject* operator*();
	SML::Objects::UObject* operator->();
};

struct LuaFunc {
	std::string name;
	std::function<int(lua_State*, long, LuaFuncContext&)> func;
};

LuaDataType propertyToLua(lua_State* L, SML::Objects::UProperty* p, void* data);
LuaDataType luaToProperty(lua_State* L, SML::Objects::UProperty* p, void* data, int i);
void registerClass(SDK::UClass* clazz, std::vector<LuaFunc> functions);
bool newInstance(lua_State* L, SDK::UObject* obj, SML::Objects::UObject* component = nullptr);
inline SML::Objects::UObject* getObjInstance(lua_State* L, int index, SML::Objects::UClass* c) {
	if (!lua_istable(L, index)) return nullptr;
	lua_getfield(L, index, "__object");
	auto ud = (LuaObjectPtr*) luaL_checkudata(L, -1, "WeakObjPtr");
	SML::Objects::UObject* o;
	lua_pop(L, 1);
	if (!ud || !(o = ud->getObject())) return nullptr;
	if (o->isA(c)) return o;
	return nullptr;
}
template<class T>
inline T* getObjInstance(lua_State* L, int index) {
	return (T*) getObjInstance(L, index, (SML::Objects::UClass*) T::StaticClass());
}
inline SML::Objects::UClass* getClassInstance(lua_State* L, int index, SML::Objects::UClass* c) {
	if (!lua_istable(L, index)) return nullptr;
	lua_getfield(L, index, "__object");
	auto o = (SML::Objects::UClass*) ((LuaObjectPtr*) luaL_checkudata(L, -1, "WeakObjPtr"))->getObject();
	lua_pop(L, 1);
	if (!o || !o->isA(SML::Objects::UClass::staticClass())) return nullptr;
	auto c1 = o;
	auto c2 = c;
	if (c1->isChild(c2)) return o;
	return nullptr;
}
template<class T>
inline SDK::UClass* getClassInstance(lua_State* L, int index) {
	return (SDK::UClass*) getClassInstance(L, index, (SML::Objects::UClass*) T::StaticClass());
}

void luaItemStack(lua_State* L, SDK::FInventoryStack stack);
void luaItem(lua_State* L, SDK::FInventoryItem item);
void luaFactoryHook(lua_State* L, SDK::UFGFactoryConnectionComponent* con);
void luaFile(lua_State* L, std::unique_ptr<FileSystemFileStream> file);

void loadLibs(lua_State* L);