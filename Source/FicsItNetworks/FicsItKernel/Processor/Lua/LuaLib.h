#pragma once

#include "LuaInstance.h"

namespace FicsItKernel {
	namespace Lua {
		/**
		* This function is used to manage the list pre defines lua library functions
		* which will get filled with registering closures static initialization.
		*/
		class LuaLib {
		public:
			/**
			 * The function type getting called when the LuaLib should get registered.
			 * The arguments should output the instance type, the instance type name
			 * and a set of LuaLibFunc/LuaLibClassFunc with name pairs.
			 */
			typedef std::function<void(UClass*&, std::string&, std::vector<std::pair<std::string, LuaLibFunc>>&)> ToRegisterFunc;
			typedef std::function<void(UClass*&, std::string&, std::vector<std::pair<std::string, LuaLibClassFunc>>&)> ToRegisterClassFunc;

		private:
			/**
			 * A set with to register functions which will get used to register
			 * when the library should get registered.
			 */
			std::vector<ToRegisterFunc> toRegister;
			std::vector<ToRegisterClassFunc> toRegisterClasses;
			
			LuaLib() = default;
		public:
            /**
            * Returns the instance of the LuaLib singleton.
            *
            * @return	instance of the LuaLib singleton.
            */
            static LuaLib* get();

			/**
			 * Gets called when the module gets load to register all functions needed to get registered.
			 */
			void registerLib();

			/**
			 * Adds a new register function to the register functions.
			 *
			 * @param[in]	func	the to register func
			 */
			void registerRegFunc(const ToRegisterFunc& func);

			/**
             * Adds a new register function to the register class functions.
             *
             * @param[in]	func	the to register class func
             */
			void registerRegFunc(const ToRegisterClassFunc& func);
		};
	}
}