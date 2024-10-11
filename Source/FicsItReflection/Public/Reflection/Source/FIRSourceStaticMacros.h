#pragma once

#include "FIRSourceStatic.h"
#include "FIRGlobalRegisterHelper.h"
#include "Reflection/FIRArrayProperty.h"
#include "Reflection/FIRBoolProperty.h"
#include "Reflection/FIRClassProperty.h"
#include "Reflection/FIRFloatProperty.h"
#include "Reflection/FIRIntProperty.h"
#include "Reflection/FIRObjectProperty.h"
#include "Reflection/FIRStrProperty.h"
#include "Reflection/FIRStructProperty.h"
#include "Reflection/FIRTraceProperty.h"

#define TypeClassName(Type) FIR_StaticRef_ ## Type
#define NSName "FicsItNetworks-StaticReflection"
#define FIRRefLocText(KeyName, Value) FInternationalization::ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText(Value, TEXT(NSName), KeyName)
#define FIRRefTypeLocText(KeyName, Value) FIRRefLocText(*(FString(TName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define BeginClass(Type, InternalName, DisplayName, Description) \
	namespace TypeClassName(Type) { \
		using T = Type; \
		constexpr auto TName = TEXT(#Type) ; \
		UClass* GetUType() { return T::StaticClass(); } \
		FORCEINLINE T* GetFromCtx(const FFIRExecutionContext& Ctx) { return Cast<T>(Ctx.GetObject()); } \
		FFIRStaticGlobalRegisterFunc RegClass([](){ \
			UFIRSourceStatic::AddClass(T::StaticClass(), FFIRStaticClassReg{TEXT(InternalName), FIRRefTypeLocText("DisplayName", DisplayName), FIRRefTypeLocText("Description", Description)}); \
		});
#define EndClass() };
#define ExtendClass(Type) \
	namespace TypeClassName(Type) { \
		using T = Type; \
		constexpr auto TName = TEXT(#Type); \
		UClass* GetUType() { return T::StaticClass(); } \
		FORCEINLINE T* GetFromCtx(const FFIRExecutionContext& Ctx) { return Cast<T>(Ctx.GetObject()); }
#define TypeStructName(Type) FIR_StaticRef_ ## Type
#define _BeginStruct(Type, InternalName, DisplayName, Description, bConstructable) \
	namespace TypeStructName(Type) { \
		using T = Type; \
		constexpr auto TName = TEXT(#Type) ; \
		UScriptStruct* GetUType() { return TBaseStructure<T>::Get(); } \
		FORCEINLINE T* GetFromCtx(const FFIRExecutionContext& Ctx) { return static_cast<T*>(Ctx.GetGeneric()); } \
		FFIRStaticGlobalRegisterFunc RegStruct([](){ \
			UFIRSourceStatic::AddStruct(GetUType(), FFIRStaticStructReg(TEXT(InternalName), FIRRefTypeLocText("DisplayName", DisplayName), FIRRefTypeLocText("Description", Description), bConstructable)); \
		});
#define BeginStruct(Type, InternalName, DisplayName, Description) _BeginStruct(Type, InternalName, DisplayName, Description, false)
#define BeginStructConstructable(Type, InternalName, DisplayName, Description) _BeginStruct(Type, InternalName, DisplayName, Description, true)
#define EndStruct() };
#define GetClassFunc [](){ return T::StaticClass(); }
#define FuncClassName(Prefix, Func) FIR_StaticRefFunc_ ## Prefix ## _ ## Func
#define FIRRefFuncLocText(KeyName, Value) FIRRefLocText(*(FString(TName) + TEXT("_") + FString(FName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define BeginFuncRT(Prefix, InternalName, DisplayName, Description, Varargs, FuncType, Runtime) \
	namespace FuncClassName(Prefix, InternalName) { \
		constexpr int F = __COUNTER__; \
		constexpr auto FName = TEXT(#InternalName) ; \
		void Execute(const FFIRExecutionContext& Ctx, TArray<FIRAny>& Params); \
		FFIRStaticGlobalRegisterFunc RegClass([](){ \
			UFIRSourceStatic::AddFunction(GetUType(), F, FFIRStaticFuncReg{TEXT(#InternalName), FIRRefFuncLocText("DisplayName", DisplayName), FIRRefFuncLocText("Description", Description), Varargs, &Execute, Runtime, FuncType}); \
			TArray<FIRAny> Params; \
			Execute(FIRTrace(nullptr), Params); \
		}); \
		void Execute(const FFIRExecutionContext& Ctx, TArray<FIRAny>& Params) { \
		static bool _bGotReg = false;
#define GET_MACRO(_0, VAL,...) VAL
#define BeginFunc(InternalName, DisplayName, Description, ...) BeginFuncRT(Member, InternalName, DisplayName, Description, false, 0, GET_MACRO(0 , ##__VA_ARGS__, 1) ) \
		T* self = GetFromCtx(Ctx);
#define BeginOp(InternalName, OperatorNum, DisplayName, Description, ...) BeginFuncRT(Member, InternalName ## _ ## OperatorNum, DisplayName, Description, false, 0, GET_MACRO(0 , ##__VA_ARGS__, 1) ) \
		T* self = GetFromCtx(Ctx);
#define BeginFuncVA(InternalName, DisplayName, Description, ...) BeginFuncRT(Member, InternalName, DisplayName, Description, true, 0, GET_MACRO(0, ##__VA_ARGS__, 1) ) \
		T* self = GetFromCtx(Ctx);
#define BeginClassFunc(InternalName, DisplayName, Description, VA, ...) BeginFuncRT(Class, InternalName, DisplayName, Description, VA, 1, GET_MACRO(0, ##__VA_ARGS__, 1) ) \
		TSubclassOf<T> self = Cast<UClass>(Ctx.GetObject());
#define BeginStaticFunc(InternalName, DisplayName, Description, VA, ...) BeginFuncRT(Static, InternalName, DisplayName, Description, VA, 2, GET_MACRO(0, ##__VA_ARGS__, 1) ) \
		void* self = Ctx.GetGeneric();
#define Body() \
			if (self && _bGotReg) {
#define EndFunc() \
			else if (!_bGotReg) _bGotReg = true; \
			} \
		} \
	};
#define PropClassName(Prefix, Prop) FIR_StaticRefProp_ ## Prefix ## _ ## Prop
#define FIRRefPropLocText(KeyName, Value) FIRRefLocText(*(FString(TName) + TEXT("_") + FString(PName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define BeginPropRT(Prefix, Type, InternalName, DisplayName, Description, PropType, Runtime) \
	namespace PropClassName(Prefix, InternalName) { \
		const int P = __COUNTER__; \
		constexpr auto PName = TEXT(#InternalName) ; \
		using PT = Type; \
		FIRAny Get(const FFIRExecutionContext& Ctx); \
		FFIRStaticGlobalRegisterFunc RegProp([](){ \
			UFIRSourceStatic::AddProp(GetUType(), P, FFIRStaticPropReg{TEXT(#InternalName), FIRRefPropLocText("DisplayName", DisplayName), FIRRefPropLocText("Description", Description), &Get, Runtime, PropType, &PT::PropConstructor}); \
		}); \
		FIRAny Get(const FFIRExecutionContext& Ctx) {
#define BeginProp(Type, InternalName, DisplayName, Description, ...) BeginPropRT(Member, Type, InternalName, DisplayName, Description, 0, GET_MACRO(0, ##__VA_ARGS__, 1) ) \
	T* self = GetFromCtx(Ctx);
#define BeginClassProp(Type, InternalName, DisplayName, Description, ...) BeginPropRT(Class, Type, InternalName, DisplayName, Description, 1, GET_MACRO(0, ##__VA_ARGS__, 1) ) \
	TSubclassOf<T> self = Cast<UClass>(Ctx.GetObject());
#define BeginStaticProp(Type, InternalName, DisplayName, Description, ...) BeginPropRT(Class, Type, InternalName, DisplayName, Description, 2, GET_MACRO(0, ##__VA_ARGS__, 1) )
#define FIRReturn \
		return (FIRAny)
#define PropSet() \
		} \
		void Set(const FFIRExecutionContext& Ctx, const FIRAny& Val); \
		FFIRStaticGlobalRegisterFunc RegPropSet([](){ \
			UFIRSourceStatic::AddPropSetter(GetUType(), P, &Set); \
		}); \
		void Set(const FFIRExecutionContext& Ctx, const FIRAny& AnyVal) { \
			T* self = GetFromCtx(Ctx); \
			PT::CppType Val = PT::Get(AnyVal);
#define EndProp() \
		} \
	};

#define FIRRefParamLocText(ParamName, KeyName, Value) FIRRefLocText(*(FString(TName) + TEXT("_") + FString(FName) + TEXT("_") + TEXT(ParamName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define InVal(Pos, Type, InternalName, DisplayName, Description) \
	Type::CppType InternalName = Type::CppType(); \
	if (!_bGotReg) { UFIRSourceStatic::AddFuncParam(GetUType(), F, Pos, FFIRStaticFuncParamReg{TEXT(#InternalName), FIRRefParamLocText(#InternalName, "DisplayName", DisplayName), FIRRefParamLocText(#InternalName, "Description", Description), 0, &Type::PropConstructor});  } \
	else InternalName = Type::Get(Params[Pos]);
#define OutVal(Pos, Type, InternalName, DisplayName, Description) \
	FIRAny& InternalName = _bGotReg ? Params[Pos] : *(FIRAny*)nullptr; \
	if (!_bGotReg) { UFIRSourceStatic::AddFuncParam(GetUType(), F, Pos, FFIRStaticFuncParamReg{TEXT(#InternalName), FIRRefParamLocText(#InternalName, "DisplayName", DisplayName), FIRRefParamLocText(#InternalName, "Description", Description), 1, &Type::PropConstructor}); }
#define RetVal(Pos, Type, InternalName, DisplayName, Description) \
	FIRAny& InternalName = _bGotReg ? Params[Pos] : *(FIRAny*)nullptr; \
	if (!_bGotReg) { UFIRSourceStatic::AddFuncParam(GetUType(), F, Pos, FFIRStaticFuncParamReg{TEXT(#InternalName), FIRRefParamLocText(#InternalName, "DisplayName", DisplayName), FIRRefParamLocText(#InternalName, "Description", Description), 3, &Type::PropConstructor}); }

#define FIRRefSignalLocText(KeyName, Value) FIRRefLocText(*(FString(TName) + TEXT("_") + FString(SName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define FIRRefSignalParamLocText(ParamName, KeyName, Value) FIRRefLocText(*(FString(TName) + TEXT("_") + FString(SName) + TEXT("_") + TEXT(ParamName) + TEXT("_") + TEXT(KeyName)), TEXT(Value))
#define SignalClassName(Prop) FIR_StaticRefSignal_ ## Prop
#define BeginSignal(InternalName, DisplayName, Description, ...) \
	namespace SignalClassName(InternalName) { \
		const int S = __COUNTER__; \
		constexpr auto SName = TEXT(#InternalName) ; \
		FFIRStaticGlobalRegisterFunc RegSignal([](){ \
			UFIRSourceStatic::AddSignal(GetUType(), S, FFIRStaticSignalReg{TEXT(#InternalName), FIRRefSignalLocText("DisplayName", DisplayName), FIRRefSignalLocText("Description", Description), GET_MACRO(0, ##__VA_ARGS__, false)});
#define SignalParam(Pos, Type, InternalName, DisplayName, Description) \
			UFIRSourceStatic::AddSignalParam(GetUType(), S, Pos, FFIRStaticSignalParamReg{TEXT(#InternalName), FIRRefSignalParamLocText(#InternalName, "DisplayName", DisplayName), FIRRefSignalParamLocText(#InternalName, "Description", Description), &Type::PropConstructor});
#define EndSignal() \
		}); \
	};

#define Hook(HookClass) \
	FFIRStaticGlobalRegisterFunc Hook([](){ \
		AFIRHookSubsystem::RegisterHook(GetUType(), HookClass::StaticClass()); \
	});

#define TFS(Str) FText::FromString( Str )

struct RInt {
	typedef FIRInt CppType;
	static FIRInt Get(const FFIRAnyValue& Any) { return Any.GetInt(); }
	static UFIRProperty* PropConstructor(UObject* Outer) {
		return NewObject<UFIRIntProperty>(Outer);
	}
};

struct RFloat {
	typedef FIRFloat CppType;
	static FIRFloat Get(const FIRAny& Any) { return Any.GetFloat(); }
	static UFIRProperty* PropConstructor(UObject* Outer) {
		return NewObject<UFIRFloatProperty>(Outer);
	}
};

struct RBool {
	typedef FIRBool CppType;
	static FIRBool Get(const FIRAny& Any) { return Any.GetBool(); }
	static UFIRProperty* PropConstructor(UObject* Outer) {
		return NewObject<UFIRBoolProperty>(Outer);
	}
};

struct RString {
	typedef FIRStr CppType;
	static FIRStr Get(const FIRAny& Any) { return Any.GetString(); }
	static UFIRProperty* PropConstructor(UObject* Outer) {
		return NewObject<UFIRStrProperty>(Outer);
	}
};

template<typename T>
struct RClass {
	typedef FIRClass CppType;
	static FIRClass Get(const FIRAny& Any) { return Any.GetClass(); }
	static UFIRProperty* PropConstructor(UObject* Outer) {
		UFIRClassProperty* Prop = NewObject<UFIRClassProperty>(Outer);
		Prop->Subclass = T::StaticClass();
		return Prop;
	}
};

template<typename T>
struct RObject {
	typedef TWeakObjectPtr<T> CppType;
	static CppType Get(const FIRAny& Any) { return CppType(Cast<T>(Any.GetObj().Get())); }
	static UFIRProperty* PropConstructor(UObject* Outer) {
		UFIRObjectProperty* Prop = NewObject<UFIRObjectProperty>(Outer);
		Prop->Subclass = T::StaticClass();
		return Prop;
	}
};

template<typename T>
struct RTrace {
	typedef FIRTrace CppType;
	static FIRTrace Get(const FIRAny& Any) { return Any.GetTrace(); }
	static UFIRProperty* PropConstructor(UObject* Outer) {
		UFIRTraceProperty* Prop = NewObject<UFIRTraceProperty>(Outer);
		Prop->Subclass = T::StaticClass();
		return Prop;
	}
};

template<typename T>
struct RStruct {
	typedef T CppType;
	static T Get(const FIRAny& Any) { return Any.GetStruct().Get<T>(); }
	static UFIRProperty* PropConstructor(UObject* Outer) {
		UFIRStructProperty* FIRProp = NewObject<UFIRStructProperty>(Outer);
		FIRProp->Struct = TBaseStructure<T>::Get();
		return FIRProp;
	}
};

template<typename T>
struct RArray {
	typedef FIRArray CppType;
	static FIRArray Get(const FIRAny& Any) { return Any.GetArray(); }
	static UFIRProperty* PropConstructor(UObject* Outer) {
		UFIRArrayProperty* FIRProp = NewObject<UFIRArrayProperty>(Outer);
		FIRProp->InnerType = T::PropConstructor(FIRProp);
		return FIRProp;
	}
};
