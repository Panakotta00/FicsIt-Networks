#pragma once

#include <exception>
#include <string>

struct lua_State;

namespace FicsItKernel {
	namespace Lua {
		class LuaException {
		private:
			std::string _what;
		public:
			LuaException(std::string what);
			virtual ~LuaException() {}
			virtual std::string what();
			virtual int lua(lua_State* L);
		};

		class LuaExceptionArg : public LuaException {
		private:
			int arg_;
		public:
			LuaExceptionArg(int arg, std::string what);
			virtual int lua(lua_State* L) override;
			int arg();
			int arg(int arg);
		};
	}
}