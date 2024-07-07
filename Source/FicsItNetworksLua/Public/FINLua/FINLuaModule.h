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
 * Represents a Lua Module Function Parameter or Returnvalue
 */
struct FFINLuaFunctionParameter {
	FString InternalName;

	FString Type;

	FText DisplayName;

	FText Description;
};

/**
 * Represents a Lua Module Function
 */
struct FFINLuaFunction : FFINLuaModuleValue {
	virtual uint32 TypeID() const { return FINTypeId<FFINLuaFunction>::ID(); }

	FFINLuaFunction(lua_CFunction Function) : Function(Function) {}

	lua_CFunction Function;

	FString ParameterSignature;
	FString ReturnValueSignature;

	TArray<FFINLuaFunctionParameter> Parameters;
	TArray<FFINLuaFunctionParameter> ReturnValues;

	virtual void PushLuaValue(lua_State* L, const FString& PersistName) override;

	void AddParameter(int Pos, const FFINLuaFunctionParameter& Parameter) {
		if (Parameters.Num() <= Pos) Parameters.SetNum(Pos+1);
		Parameters.Insert(Parameter, Pos);
	}

	void AddReturnValue(int Pos, const FFINLuaFunctionParameter& ReturnValue) {
		if (ReturnValues.Num() <= Pos) ReturnValues.SetNum(Pos+1);
		ReturnValues.Insert(ReturnValue, Pos);
	}

	FString GetSignature(const FString& Name) const {
		return FString::Printf(TEXT("%ls %ls(%ls)"), *ReturnValueSignature, *Name, *ParameterSignature);
	}
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

	FFINLuaTableField* FieldByName(const FString& Name) {
		for (FFINLuaTableField& field : Fields) {
			if (field.Key == Name) {
				return &field;
			}
		}
		return nullptr;
	}

	void AddFunctionFieldByDocumentationComment(lua_CFunction Function, const FString& Comment, const TCHAR* InternalName);
	void AddBareFieldByDocumentationComment(TFunction<void(lua_State* L, const FString&)> Function, const FString& Comment, const TCHAR* InternalName);
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
	FFINLuaModule() = default;
	FFINLuaModule(const FString& InternalName, const FText& DisplayName, const FText& Description) : InternalName(InternalName), DisplayName(DisplayName), Description(Description) {}

	FString InternalName;

	FText DisplayName;

	FText Description;

	TArray<FFINLuaMetatable> Metatables;
	TArray<FFINLuaGlobal> Globals;

	TDelegate<void(FFINLuaModule&, lua_State*)> PreSetup;
	TDelegate<void(FFINLuaModule&, lua_State*)> PostSetup;

	void SetupModule(lua_State* L);

	void ParseDocumentationComment(const FString& Comment, const TCHAR* InternalName);

	void AddLibraryByDocumentationComment(const TSharedRef<FFINLuaTable>& Table, const FString& Comment, const TCHAR* InternalName);
	void AddMetatableByDocumentationComment(const TSharedRef<FFINLuaTable>& Table, const FString& Comment, const TCHAR* InternalName);
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

#define LuaModule(Documentation, _NameSpace) \
	namespace _NameSpace { \
		static TSharedRef<FFINLuaModule> Module = MakeShared<FFINLuaModule>(); \
		static FFINStaticGlobalRegisterFunc RegisterModule([]() { \
			Module->ParseDocumentationComment(TEXT(Documentation), TEXT(#_NameSpace)); \
			FFINLuaModuleRegistry::GetInstance().AddModule(Module); \
		}, 0); \
	} \
	namespace _NameSpace
#define LuaModulePostSetup() \
	void PreSetup(FFINLuaModule&, lua_State*); \
	static FFINStaticGlobalRegisterFunc RegisterPreSetup([]() { \
		Module->PreSetup.BindStatic(&PreSetup); \
	}); \
	void PreSetup(FFINLuaModule& InModule, lua_State* L)
#define LuaModulePostSetup() \
	void PostSetup(FFINLuaModule&, lua_State*); \
	static FFINStaticGlobalRegisterFunc RegisterPostSetup([]() { \
		Module->PostSetup.BindStatic(&PostSetup); \
	}); \
	void PostSetup(FFINLuaModule& InModule, lua_State* L)
#define LuaModuleLibrary(Documentation, _NameSpace) \
	namespace _NameSpace { \
		static TSharedRef<FFINLuaTable> Table = MakeShared<FFINLuaTable>(); \
		static FFINStaticGlobalRegisterFunc RegisterLibrary([]() { \
			Module->AddLibraryByDocumentationComment(Table, Documentation, TEXT(#_NameSpace)); \
		}, 1); \
	} \
	namespace _NameSpace
#define LuaModuleMetatable(Documentation, _NameSpace) \
	namespace _NameSpace { \
		static TSharedRef<FFINLuaTable> Table = MakeShared<FFINLuaTable>(); \
		static FFINStaticGlobalRegisterFunc RegisterMetatable([]() { \
			Module->AddMetatableByDocumentationComment(Table, Documentation, TEXT(#_NameSpace)); \
		}, 1); \
	} \
	namespace _NameSpace
#define LuaModuleTableFunction(Documentation, _FunctionName) \
	int _FunctionName (lua_State* L); \
	static FFINStaticGlobalRegisterFunc RegisterField_ ## _FunctionName ([]() { \
		Table->AddFunctionFieldByDocumentationComment(&_FunctionName, Documentation, TEXT(#_FunctionName)); \
	}, 2); \
	int _FunctionName (lua_State* L)
#define LuaModuleTableBareField(Documentation, _FieldName) \
	void _FieldName (lua_State*, const FString&); \
	static FFINStaticGlobalRegisterFunc RegisterField_ ## _FieldName ([]() { \
		Table->AddBareFieldByDocumentationComment(&_FieldName, Documentation, TEXT(#_FieldName)); \
	}, 2); \
	void _FieldName (lua_State* L, const FString& PersistName)
