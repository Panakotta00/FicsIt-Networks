#pragma once

#include <queue>
#include <deque>
#include <chrono>
#include <memory>

#include <util/Objects/Delegate.h>
#include <util/Objects/SmartPointer.h>
#include <assets/BPInterface.h>
#include <SDK.hpp>
#include "NetworkConnector.h"
#include "LuaLib.h"

extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

class ULuaContext;

struct Signal {
	virtual std::string getName() const = 0;
	virtual int toLua(lua_State* L) const = 0;
};

struct SignalProperty : public Signal {
	void* data = nullptr;
	SML::Objects::UObject* sender;
	SML::Objects::UFunction* func = nullptr;

	SignalProperty(SML::Objects::UFunction* func, void* data, SML::Objects::UObject* sender);
	SignalProperty(const SignalProperty& o);
	virtual ~SignalProperty();

	SignalProperty& operator=(const SignalProperty& o);

	std::string getName() const override;
	int toLua(lua_State* L) const override;
};

struct SignalFactoryHook : public Signal {
	SML::Objects::UClass* item;

	SignalFactoryHook(SML::Objects::UClass* item);
	SignalFactoryHook(const SignalFactoryHook& o);

	std::string getName() const override;
	int toLua(lua_State* L) const override;
};

struct SignalPowerFused: public Signal {
	std::string getName() const override;
	int toLua(lua_State* L) const override;
};

struct SignalSource {
	SML::Objects::FWeakObjectPtr ctx;

	SignalSource(ULuaContext* ctx);
	virtual ~SignalSource();
};

struct SignalSourceProperty : public SignalSource {
	SML::Objects::FWeakObjectPtr obj;

	SignalSourceProperty(ULuaContext* ctx, SML::Objects::UObject* o);
	~SignalSourceProperty();
};

struct SignalSourceHook : public SignalSource{
	SML::Objects::FWeakObjectPtr obj;

	SignalSourceHook(ULuaContext* ctx, SML::Objects::UObject* o);
	~SignalSourceHook();
};

enum ELuaState : std::int32_t {
	RUNNING = 1,
	HALTED = 0,
	FINISHED = 2,
	CRASHED = 3
};

class ULuaContext : public SML::Objects::UObject {
public:
	static ULuaContext* ctx;

	int memory;
	SML::Objects::FString code;
	SML::Objects::FString log;
	SML::Objects::FString exception;
	ELuaState state;
	SML::Objects::UObject* component;
	int timeout = -1;
	std::chrono::time_point<std::chrono::high_resolution_clock> pullStart;

	lua_State* L;
	std::queue<std::shared_ptr<Signal>> signals;
	std::vector<std::unique_ptr<SignalSource>> signalSources;

	void construct();
	void destructor();

	void setCode(SML::Objects::FString code);
	void reset();
	void execSteps(std::int32_t count);
	int doSignal(lua_State* L);
	bool pushSignal(std::shared_ptr<Signal> sig);
	bool pushSignal(Signal* sig);

	void setException(const std::string ex);
	void Log(const SML::Objects::FString& str);
	UNetworkConnector* getConnector() const;
	SML::Objects::UObject* getComponent(const std::string& addr) const;
	SML::Objects::FGuid getComponentByNick(const std::string& nick) const;

	// BPI wrappings
	static void execExecSteps(ULuaContext* self, SML::Objects::FFrame& stack, void* p);
	static void execReset(ULuaContext* self, SML::Objects::FFrame& stack, void* p);
	static void execSetCode(ULuaContext* self, SML::Objects::FFrame& stack, void* p);
	static void execSignalSlot(ULuaContext* self, SML::Objects::FFrame& stack, void* p);
};