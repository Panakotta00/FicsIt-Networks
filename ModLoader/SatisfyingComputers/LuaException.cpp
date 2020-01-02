#include "stdafx.h"
#include "LuaException.h"

extern "C" {
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

LuaException::LuaException(std::string what) : _what(what) {}

std::string LuaException::what() {
	return _what;
}

int LuaException::lua(lua_State * L) {
	return luaL_error(L, this->what().c_str());
}

LuaExceptionArg::LuaExceptionArg(int arg, std::string what) : LuaException(what), arg_(arg) {}

int LuaExceptionArg::lua(lua_State * L) {
	return luaL_argerror(L, arg_, what().c_str());
}

int LuaExceptionArg::arg() {
	return arg_;
}

int LuaExceptionArg::arg(int arg) {
	return arg_ = arg;
}
