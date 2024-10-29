#pragma once

#include "CoreMinimal.h"
#include "FIRGlobalRegisterHelper.h"
#include "LuaUtil.h"
#include "FINLuaModule.generated.h"

// TODO: Make Comment Documentation Parsing more Unified and DRY, so field names etc all have same parsing rules
// TODO: Setup bare value AFTER everything else has been setup (before post setup). So bare values can get access to metatables if needed. But still consider setup order. (generally a setup order for bare values is needed)

#undef UNIQUE_FUNCTION_ID

#if defined(_MSC_VER)
  #define UNIQUE_FUNCTION_ID __FUNCSIG__
#else
  #if defined( __GNUG__ )
	#define UNIQUE_FUNCTION_ID __PRETTY_FUNCTION__
  #endif
#endif

/**
 * Represents a Lua Module Value
 */
USTRUCT()
struct FICSITNETWORKSLUA_API FFINLuaModuleValue {
	GENERATED_BODY()

	FFINLuaModuleValue() = default;
	virtual ~FFINLuaModuleValue() = default;

	virtual UStruct* TypeID() const { return StaticStruct(); }

	virtual void PushLuaValue(lua_State* L, const FString& PersistName) {
		lua_pushnil(L);
	}
};

/**
 * Represents any C++ Code as a Lua Module Value
 */
USTRUCT()
struct FICSITNETWORKSLUA_API FFINLuaModuleBareValue : public FFINLuaModuleValue {
	GENERATED_BODY()

	virtual UStruct* TypeID() const { return StaticStruct(); }

	FFINLuaModuleBareValue() = default;
	FFINLuaModuleBareValue(const TFunction<void(lua_State* L, const FString&)>& Function) : Function(Function) {}

	TFunction<void(lua_State* L, const FString&)> Function;

	FString Type;

	virtual void PushLuaValue(lua_State* L, const FString& PersistName) override;
};

/**
 * Represents a Lua Module Function Parameter or Returnvalue
 */
struct FICSITNETWORKSLUA_API FFINLuaFunctionParameter {
	FString InternalName;

	FString Type;

	FText DisplayName;

	FText Description;
};

/**
 * Represents a Lua Module Function
 */
USTRUCT()
struct FICSITNETWORKSLUA_API FFINLuaFunction : public FFINLuaModuleValue {
	GENERATED_BODY()

	virtual UStruct* TypeID() const { return StaticStruct(); }

	FFINLuaFunction() = default;
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
struct FICSITNETWORKSLUA_API FFINLuaTableField {
	FString Key;

    FText DisplayName;

    FText Description;

	TSharedPtr<FFINLuaModuleValue> Value;
};

/**
 * Represents a Lua Module Table
 */
USTRUCT()
struct FICSITNETWORKSLUA_API FFINLuaTable : public FFINLuaModuleValue {
	GENERATED_BODY()

	virtual UStruct* TypeID() const override { return StaticStruct(); }

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
	void AddTableFieldByDocumentationComment(TSharedRef<FFINLuaTable> Table, const FString& Comment, const TCHAR* InternalName);
};

/**
 * Represents a Lua Module Global Value
 */
struct FICSITNETWORKSLUA_API FFINLuaGlobal {
	FString InternalName;

	FText DisplayName;

	FText Description;

	TSharedPtr<FFINLuaModuleValue> Value;
};

/**
 * Represents a Lua Module Metatable
 */
struct FICSITNETWORKSLUA_API FFINLuaMetatable {
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
struct FICSITNETWORKSLUA_API FFINLuaModule : TSharedFromThis<FFINLuaModule> {
	FFINLuaModule() = default;
	FFINLuaModule(const FString& InternalName, const FText& DisplayName, const FText& Description) : InternalName(InternalName), DisplayName(DisplayName), Description(Description) {}

	FString InternalName;

	FText DisplayName;

	FText Description;

	TArray<FString> Dependencies;

	TArray<FFINLuaMetatable> Metatables;
	TArray<FFINLuaGlobal> Globals;

	TDelegate<void(FFINLuaModule&, lua_State*)> PreSetup;
	TDelegate<void(FFINLuaModule&, lua_State*)> PostSetup;

	void SetupModule(lua_State* L);

	void ParseDocumentationComment(const FString& Comment, const TCHAR* InternalName);

	void AddLibraryByDocumentationComment(const TSharedRef<FFINLuaTable>& Table, const FString& Comment, const TCHAR* InternalName);
	void AddMetatableByDocumentationComment(const TSharedRef<FFINLuaTable>& Table, const FString& Comment, const TCHAR* InternalName);
	void AddGlobalBareValueByDocumentationComment(TFunction<void(lua_State* L, const FString&)> Function, const FString& Comment, const TCHAR* InternalName);
};

/**
 * This singleton is used to register statically defined Lua modules
 */
struct FICSITNETWORKSLUA_API FFINLuaModuleRegistry {
private:
	FFINLuaModuleRegistry() = default;
	FFINLuaModuleRegistry(const FFINLuaModuleRegistry&) = delete;

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

#define LuaModule(Documentation, InternalName) \
	namespace InternalName { \
		static TSharedRef<FFINLuaModule> Module = MakeShared<FFINLuaModule>(); \
		static FFIRStaticGlobalRegisterFunc RegisterModule([]() { \
			Module->ParseDocumentationComment(TEXT(Documentation), TEXT(#InternalName)); \
			FFINLuaModuleRegistry::GetInstance().AddModule(Module); \
		}, 0); \
	} \
	namespace InternalName
#define LuaModulePreSetup() \
	void PreSetup(FFINLuaModule&, lua_State*); \
	static FFIRStaticGlobalRegisterFunc RegisterPreSetup([]() { \
		Module->PreSetup.BindStatic(&PreSetup); \
	}); \
	void PreSetup(FFINLuaModule& InModule, lua_State* L)
#define LuaModulePostSetup() \
	void PostSetup(FFINLuaModule&, lua_State*); \
	static FFIRStaticGlobalRegisterFunc RegisterPostSetup([]() { \
		Module->PostSetup.BindStatic(&PostSetup); \
	}); \
	void PostSetup(FFINLuaModule& InModule, lua_State* L)
#define LuaModuleLibrary(Documentation, InternalName) \
	namespace InternalName { \
		static TSharedRef<FFINLuaTable> Table = MakeShared<FFINLuaTable>(); \
		static FFIRStaticGlobalRegisterFunc RegisterLibrary([]() { \
			Module->AddLibraryByDocumentationComment(Table, Documentation, TEXT(#InternalName)); \
		}, 1); \
	} \
	namespace InternalName
#define LuaModuleMetatable(Documentation, InternalName) \
	namespace InternalName { \
		const char* _Name = #InternalName; \
		static TSharedRef<FFINLuaTable> Table = MakeShared<FFINLuaTable>(); /* TODO: Maybe update to correct updated internal name within the function. Because Comment Documentation can change internal name. */ \
		static FFIRStaticGlobalRegisterFunc RegisterMetatable([]() { \
			Module->AddMetatableByDocumentationComment(Table, Documentation, TEXT(#InternalName)); \
		}, 1); \
	} \
	namespace InternalName
#define LuaModuleTableFunction(Documentation, _FunctionName) \
	int _FunctionName (lua_State* L); \
	static FFIRStaticGlobalRegisterFunc RegisterField_ ## _FunctionName ([]() { \
		Table->AddFunctionFieldByDocumentationComment(&_FunctionName, Documentation, TEXT(#_FunctionName)); \
	}, 2); \
	int _FunctionName (lua_State* L)
#define LuaModuleTableBareField(Documentation, _FieldName) \
	const char* _FieldName ## _Name = #_FieldName; \
	void _FieldName (lua_State*, const FString&); /* TODO: Maybe update to correct updated internal name within the function. Because Comment Documentation can change internal name. */ \
	static FFIRStaticGlobalRegisterFunc RegisterField_ ## _FieldName ([]() { \
		Table->AddBareFieldByDocumentationComment(&_FieldName, Documentation, TEXT(#_FieldName)); \
	}, 2); \
	void _FieldName (lua_State* L, const FString& PersistName)
#define LuaModuleTableTable(Documentation, _TableName) \
	namespace _TableName { \
		static TSharedRef<FFINLuaTable> _TableName = MakeShared<FFINLuaTable>(); \
		static FFIRStaticGlobalRegisterFunc RegisterTable ([]() { \
			Table->AddTableFieldByDocumentationComment(_TableName, Documentation, TEXT(#_TableName)); \
		}, 2); \
		static TSharedRef<FFINLuaTable>& Table = _TableName; \
	} \
	namespace _TableName
#define LuaModuleGlobalBareValue(Documentation, _GlobalName) \
	void _GlobalName (lua_State*, const FString&); \
	static FFIRStaticGlobalRegisterFunc RegisterGlobal_ ## _GlobalName ([]() { \
		Module->AddGlobalBareValueByDocumentationComment(&_GlobalName, Documentation, TEXT(#_GlobalName)); \
	}, 1); \
	void _GlobalName (lua_State* L, const FString& PersistName)
