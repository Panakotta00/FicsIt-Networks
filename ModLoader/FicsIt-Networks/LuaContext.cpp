#include "stdafx.h"
#include "LuaContext.h"

#include "Config.h"

#include <sstream>

#include "LuaLib.h"

ULuaContext* ULuaContext::ctx = nullptr;

void luaHook(lua_State *L, lua_Debug *ar) {
	if (lua_gc(L, LUA_GCCOUNT, 0) > ULuaContext::ctx->memory)
		luaL_error(L, "out of memory");
	lua_yield(L, 0);
}

void ULuaContext::construct() {
	new (&code) SML::Objects::FString("");
	new (&log) SML::Objects::FString("");
	new (&exception) SML::Objects::FString("");
	new (&signals) std::queue<Signal>();
	new (&signalSources) std::vector<std::unique_ptr<SignalSource>>();
	state = ELuaState::HALTED;
	controller = nullptr;
	L = nullptr;
	timeout = -1;
	memory = 0;
	pullStart = std::chrono::time_point<std::chrono::high_resolution_clock>();

	reset();
}

void ULuaContext::destructor() {
	code.~FString();
	log.~FString();
	exception.~FString();
	signals.~queue();
	signalSources.~vector();

	if (L) lua_close(L);
	L = nullptr;
}

void ULuaContext::setCode(SML::Objects::FString code) {
	this->code = code;
	reset();
}

void ULuaContext::reset() {
	// remove signalsenders
	signalSources.clear();
	timeout = -1;

	if (L) {
		lua_close(L);
	}

	auto s = signals.size();
	for (int i = 0; i < s; ++i) signals.pop();

	L = luaL_newstate();
	loadLibs(L);
	if (code.isValid()) luaL_loadstring(L, code.toStr().c_str());
	lua_sethook(L, &luaHook, LUA_MASKCOUNT, 1);
	state = HALTED;
}

void ULuaContext::execSteps(std::int32_t count) {
	ctx = this;

	switch (state) {
	case RUNNING:
	{
		if (!L) return;

		int status = 0;
		if (timeout > -1) {
			if (signals.size() > 0) {
				timeout = -1;
				status = lua_resume(L, nullptr, doSignal(L));
			} else if (timeout == 0 || timeout > std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - pullStart).count()) return;
			else status = lua_resume(L, nullptr, 0);
		} else status = lua_resume(L, nullptr, 0);
		if (status == LUA_YIELD) {
		} else if (status == LUA_OK) {
			state = FINISHED;
		} else {
			state = CRASHED;
			setException(std::string(lua_tostring(L, -1)).c_str());
		}
	} break;
	}
}

int ULuaContext::doSignal(lua_State* L) {
	if (signals.size() < 1) return 0;
	int props = 1;
	lua_pushstring(L, signals.front()->getName().c_str());
	props += (int) signals.front()->toLua(L);
	signals.pop();
	return props;
}

bool ULuaContext::pushSignal(Signal * sig) {
	if (signals.size() >= config["SignalQueueSize"].get<int>()) return false;
	signals.push(std::unique_ptr<Signal>(sig));
	return true;
}

void ULuaContext::setException(const std::string ex) {
	Log(ex.c_str());
	state = CRASHED;
}

void ULuaContext::Log(const SML::Objects::FString & str) {
	std::string s = log.toStr();
	s += str.toStr() + "\n";
	log = s.c_str();
	SML::Utility::warning(str.toStr());
}

UNetworkConnector* ULuaContext::getConnector() const {
	UNetworkConnector* connector = nullptr;
	for (auto f : *this->outer->clazz) {
		auto p = static_cast<SML::Objects::UProperty*>(f);
		if (!(p->clazz->castFlags & SML::Objects::EClassCastFlags::CAST_UObjectProperty)) continue;
		auto v = *p->getValue<UNetworkConnector*>(this->outer);
		if (!v || !v->IsA((SDK::UClass*)SML::Paks::ClassBuilder<UNetworkConnector>::staticClass())) continue;
		connector = v;
		break;
	}
	return connector;
}

SML::Objects::UObject* ULuaContext::getComponent(const std::string & addr) const {
	auto connector = getConnector();
	if (!connector) return nullptr;
	auto found = connector->circuit->findComponent(addr);
	if (found) return found;
	return nullptr;
}

// BPI wrappings

void ULuaContext::execExecSteps(ULuaContext * self, SML::Objects::FFrame & stack, void * p) {
	std::int32_t count;
	stack.stepCompIn(&count);

	stack.code += !!stack.code;

	self->execSteps(count);
}

void ULuaContext::execReset(ULuaContext * self, SML::Objects::FFrame & stack, void * p) {
	stack.code += !!stack.code;

	self->reset();
}

void ULuaContext::execSetCode(ULuaContext * self, SML::Objects::FFrame & stack, void * p) {
	SML::Objects::FString code;
	stack.stepCompIn(&code);

	stack.code += !!stack.code;

	self->setCode(code);
}

void ULuaContext::execSignalSlot(ULuaContext * self, SML::Objects::FFrame & stack, void * params) {
	auto sig = std::shared_ptr<SignalProperty>(new SignalProperty{stack.nativeFunc, (void*)((size_t)malloc(stack.nativeFunc->parmsSize)), self});
	
	memset((void*)((size_t)sig->data), 0, stack.nativeFunc->parmsSize);
	for (auto p : *stack.nativeFunc) {
		auto dp = (void*)((size_t)sig->data + ((SML::Objects::UProperty*)p)->internalOffset);
		if (stack.code) stack.step(stack.obj, dp);
		else stack.stepProp(dp, (SML::Objects::UProperty*)p);
	}

	SML::Objects::TArray<ULuaContext*> ctxs;
	self->findFunction(L"luaGetSignalListeners")->invoke(self, &ctxs);
	for (auto ctx : ctxs) if (ctx->signals.size() < config["SignalQueueSize"].get<int>()) {
		auto con = self->getConnector();
		if (!con->circuit || !con->circuit->hasNode((UObject*)ctx->getConnector())) continue;
		ctx->signals.push(std::shared_ptr<Signal>(sig));
	}

	stack.code += !!stack.code;
}

SignalProperty::SignalProperty(SML::Objects::UFunction * func, void * data, SML::Objects::UObject* sender) : func(func), data(data), sender(sender) {}

SignalProperty::SignalProperty(const SignalProperty & o) {
	*this = o;
}

SignalProperty::~SignalProperty() {
	if (data) free((void*)((size_t)data));
	data = nullptr;
}

SignalProperty& SignalProperty::operator=(const SignalProperty& o) {
	func = o.func;
	auto s = func->parmsSize;
	data = std::malloc(s);
	std::memcpy(data, o.data, s);
	sender = o.sender;
	return *this;
}

std::string SignalProperty::getName() const {
	return func->getName().substr(7);
}

int SignalProperty::toLua(lua_State * L) const {
	int props = 1;
	newInstance(L, (SDK::UObject*)sender);
	for (auto p : *func) {
		propertyToLua(L, (SML::Objects::UProperty*)p, data);
		++props;
	}
	return props;
}

SignalFactoryHook::SignalFactoryHook(SML::Objects::UClass * item) : item(item) {}

SignalFactoryHook::SignalFactoryHook(const SignalFactoryHook & o) : item(item) {}

std::string SignalFactoryHook::getName() const {
	return "ItemTransfer";
}

int SignalFactoryHook::toLua(lua_State * L) const {
	newInstance(L, (SDK::UObject*)item);
	return 1;
}

SignalSource::SignalSource(ULuaContext * ctx) : ctx(ctx) {}

SignalSource::~SignalSource() {}

SignalSourceProperty::SignalSourceProperty(ULuaContext* ctx, SML::Objects::UObject * o) : SignalSource(ctx), obj(o) {}

SignalSourceProperty::~SignalSourceProperty() {
	auto c = *ctx;
	auto o = *obj;
	if (!c || !o) return;
	o->findFunction(L"luaRemoveSignalListener")->invoke(o, &c);
}

SignalSourceHook::SignalSourceHook(ULuaContext * ctx, SML::Objects::UObject * o) : SignalSource(ctx), obj(o) {}

SignalSourceHook::~SignalSourceHook() {
	auto c = (ULuaContext*)ctx.get();
	if (!c || !obj.isValid()) return;
	try {
		auto& d = hooks.at(obj).deleg;
		d.erase(d.find(c));
	} catch (...) {}
}
