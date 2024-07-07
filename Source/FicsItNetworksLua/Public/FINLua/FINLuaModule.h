#pragma once

#include "CoreMinimal.h"
#include "FINGlobalRegisterHelper.h"
#include "LuaUtil.h"

#undef UNIQUE_FUNCTION_ID

#if defined(_MSC_VER)
  #define UNIQUE_FUNCTION_ID __FUNCSIG__
#else
  #if defined( __GNUG__ )
	#define UNIQUE_FUNCTION_ID __PRETTY_FUNCTION__
  #endif
#endif

template<typename T>
class FINTypeId {
public:
	static int ID() {
		static uint32 id = GetTypeHash(UNIQUE_FUNCTION_ID);
		return id;
	}
};

/**
 * Represents a Lua Module Value
 */
struct FFINLuaModuleValue : TSharedFromThis<FFINLuaModuleValue> {
	virtual ~FFINLuaModuleValue() = default;

	virtual uint32 TypeID() const = 0;

	virtual void PushLuaValue(lua_State* L, const FString& PersistName) = 0;
};

/**
 * Represents any C++ Code as a Lua Module Value
 */
struct FFINLuaModuleBareValue : FFINLuaModuleValue {
	virtual uint32 TypeID() const { return FINTypeId<FFINLuaModuleBareValue>::ID(); }

	FFINLuaModuleBareValue(const TFunction<void(lua_State* L, const FString&)>& Function) : Function(Function) {}

	TFunction<void(lua_State* L, const FString&)> Function;

	virtual void PushLuaValue(lua_State* L, const FString& PersistName) override;
};

/**
 * Represents a Lua Module Function
 */
struct FFINLuaFunction : FFINLuaModuleValue {
	virtual uint32 TypeID() const { return FINTypeId<FFINLuaFunction>::ID(); }

	FFINLuaFunction(lua_CFunction Function) : Function(Function) {}

	lua_CFunction Function;

	virtual void PushLuaValue(lua_State* L, const FString& PersistName) override;
};

/**
 * Represents a Lua Module Table Field
 */
struct FFINLuaTableField {
	FString Key;

    FText DisplayName;

    FText Description;

	TSharedPtr<FFINLuaModuleValue> Value;
};

/**
 * Represents a Lua Module Table
 */
struct FFINLuaTable : FFINLuaModuleValue {
	virtual uint32 TypeID() const { return FINTypeId<FFINLuaTable>::ID(); }

	TArray<FFINLuaTableField> Fields;

	virtual void PushLuaValue(lua_State* L, const FString& PersistName) override;
};

/**
 * Represents a Lua Module Global Value
 */
struct FFINLuaGlobal {
	FString InternalName;

	FText DisplayName;

	FText Description;

	TSharedPtr<FFINLuaModuleValue> Value;
};

/**
 * Represents a Lua Module Metatable
 */
struct FFINLuaMetatable {
	FString InternalName;

	FText DisplayName;

	FText Description;

	TSharedPtr<FFINLuaTable> Table;
};

/**
 * A FINLuaModule is collection of global functions, tables, metatables and more.
 * These modules can be dynamically discovered and loaded at runtime.
 * They allow the easy extension of the Lua Runtime through external sources (like extension mods).
 * Another main purpose is to provide documentation for the different functions etc. to be usable within the game.
 * A module can also be just a wrapper around a simple Lua library, but providing documentation.
 */
struct FFINLuaModule : TSharedFromThis<FFINLuaModule> {
	FFINLuaModule(const FString& InternalName, const FText& DisplayName, const FText& Description) : InternalName(InternalName), DisplayName(DisplayName), Description(Description) {}

	FString InternalName;

	FText DisplayName;

	FText Description;

	TArray<FFINLuaMetatable> Metatables;
	TArray<FFINLuaGlobal> Globals;

	TDelegate<void(FFINLuaModule&, lua_State*)> PreSetup;
	TDelegate<void(FFINLuaModule&, lua_State*)> PostSetup;

	void SetupModule(lua_State* L);

	static void AddLuaFunctionsToTable(lua_State* L, int index, const TArray<FFINLuaFunction>& Functions);
};

/**
 * This singleton is used to register statically defined Lua modules
 */
struct FFINLuaModuleRegistry {
private:
	FFINLuaModuleRegistry() = default;

public:
	[[nodiscard]] static FFINLuaModuleRegistry& GetInstance();

	TArray<TSharedRef<FFINLuaModule>> Modules;

	void AddModule(const TSharedRef<FFINLuaModule>& Module) {
		if (Module->InternalName == "ModuleSystem") {
			Modules.Insert(Module, 0);
		} else {
			Modules.Add(Module);
		}
	}
};

#define BeginLuaModule(_InternalName, _DisplayName, _Description) \
	namespace _InternalName ## Module { \
		static TSharedRef<FFINLuaModule> Module = MakeShared<FFINLuaModule>( \
			TEXT(#_InternalName), \
			_DisplayName, \
			_Description \
		); \
		static FFINStaticGlobalRegisterFunc RegisterModule([]() { \
				FFINLuaModuleRegistry::GetInstance().AddModule(Module); \
		}, 0);
#define ModulePreSetup() \
		void PreSetup(FFINLuaModule&, lua_State*); \
		static FFINStaticGlobalRegisterFunc RegisterPreSetup([]() { \
			Module->PreSetup.BindStatic(&PreSetup); \
		}); \
		void PreSetup(FFINLuaModule& InModule, lua_State* L)
#define ModulePostSetup() \
		void PostSetup(FFINLuaModule&, lua_State*); \
		static FFINStaticGlobalRegisterFunc RegisterPostSetup([]() { \
			Module->PostSetup.BindStatic(&PostSetup); \
		}); \
		void PostSetup(FFINLuaModule& InModule, lua_State* L)
#define BeginMetatable(_InternalName, _DisplayName, _Description) \
		namespace _InternalName ## Metatable { \
			static TSharedRef<FFINLuaTable> Table = MakeShared<FFINLuaTable>(); \
			static FFINStaticGlobalRegisterFunc RegisterMetatable([]() { \
				Module->Metatables.Add(FFINLuaMetatable{ \
					.InternalName = TEXT(#_InternalName), \
					.DisplayName = _DisplayName, \
					.Description = _Description, \
					.Table = Table, \
				}); \
			}, 1);
#define EndMetatable() \
		}
#define BeginLibrary(_InternalName, _DisplayName, _Description) \
		namespace _InternalName ## Library { \
			static TSharedRef<FFINLuaTable> Table = MakeShared<FFINLuaTable>(); \
			static FFINStaticGlobalRegisterFunc RegisterLibrary([]() { \
				Module->Globals.Add(FFINLuaGlobal{ \
					.InternalName = TEXT(#_InternalName), \
					.DisplayName = _DisplayName, \
					.Description = _Description, \
					.Value = Table, \
				}); \
			}, 1);
#define FieldFunction(_InternalName, _DisplayName, _Description) \
			int luaFunc_ ## _InternalName (lua_State* L); \
			static FFINStaticGlobalRegisterFunc RegisterField ## _InternalName ([]() { \
				Table->Fields.Add(FFINLuaTableField{ \
					.Key = TEXT(#_InternalName), \
					.DisplayName = _DisplayName, \
					.Description = _Description, \
					.Value = MakeShared<FFINLuaFunction>(&luaFunc_ ## _InternalName), \
				}); \
			}, 2); \
			int luaFunc_ ## _InternalName (lua_State* L)
#define BeginFieldTable(_InternalName, _DisplayName, _Description) \
			static FFINStaticGlobalRegisterFunc RegisterField ## _InternalName ([]() { \
				Table->Fields.Add(FFINLuaTableField{ \
					.Key = TEXT(#_InternalName), \
					.DisplayName = _DisplayName, \
					.Description = _Description, \
					.Value = _InternalName::Table, \
				}); \
			}, 2); \
			namespace _InternalName { \
				static TSharedRef<FFINLuaTable> Table = MakeShared<FFINLuaTable>();
#define EndFieldTable \
			}
#define FieldBare(_InternalName, _DisplayName, _Description) \
			void lua_ ## _InternalName (lua_State*, const FString&); \
			static FFINStaticGlobalRegisterFunc RegisterField ## _InternalName ([]() { \
				Table->Fields.Add(FFINLuaTableField{ \
					.Key = TEXT(#_InternalName), \
					.DisplayName = _DisplayName, \
					.Description = _Description, \
					.Value = MakeShared<FFINLuaModuleBareValue>(lua_ ## _InternalName), \
				}); \
			}, 2); \
			void lua_ ## _InternalName (lua_State* L, const FString& PersistName)
#define EndLibrary() \
		}
#define EndLuaModule() \
	}
