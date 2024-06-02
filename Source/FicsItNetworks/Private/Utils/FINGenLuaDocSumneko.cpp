#include "Utils/FINGenLuaDocSumneko.h"

#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Reflection/FINArrayProperty.h"
#include "Reflection/FINClassProperty.h"
#include "Reflection/FINObjectProperty.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStructProperty.h"
#include "Reflection/FINTraceProperty.h"
#include "UObject/Package.h"

#pragma optimize("", off)
FString FINGenLuaSumnekoGetTypeName(const UFINBase *Base) {
	TArray<FString> SplitBasePackageName;
	Base->GetPackage()->GetName().ParseIntoArray(SplitBasePackageName, TEXT("/"), true);

	FString BasePackageName;
	for (auto &NamePart : SplitBasePackageName) {
		if (NamePart.Equals(TEXT("Script"))) {
			continue;
		}

		// we only want the the first not filtered out name should be the mod name
		if (NamePart.Equals(TEXT("CoreUObject"))) {
			BasePackageName += TEXT("Engine.");
			break;
		}

		// replace mod and Satisfactory name to make type names smaller
		else if (NamePart.Equals(TEXT("FactoryGame"))
			|| NamePart.Equals(TEXT("Game"))) {
			BasePackageName += TEXT("Satis.");
			break;
		} else if (NamePart.Equals(TEXT("FicsItNetworks"))
			|| NamePart.Equals(TEXT("FicsItNetworksLua"))) {
			BasePackageName += TEXT("FIN.");
			break;
		}

		BasePackageName += NamePart + TEXT(".");
		break;
	}

	return BasePackageName + Base->GetInternalName();
}

FString FINGenLuaSumnekoGetType(FFINReflection &Ref, const UFINProperty *Prop) {
	if (!Prop) {
		return "any";
	}

	switch (Prop->GetType()) {
	case FIN_NIL:
		return "nil";
	case FIN_BOOL:
		return "boolean";
	case FIN_INT:
	case FIN_FLOAT:
		return "number";
	case FIN_STR:
		return "string";
	case FIN_OBJ: {
		const UFINObjectProperty *ObjProp = Cast<UFINObjectProperty>(Prop);
		const UFINClass *Class = Ref.FindClass(ObjProp->GetSubclass());
		if (!Class)
			return "Engine.Object";
		return FINGenLuaSumnekoGetTypeName(Class);
	}
	case FIN_TRACE: {
		const UFINTraceProperty *TraceProp = Cast<UFINTraceProperty>(Prop);
		const UFINClass *Class = Ref.FindClass(TraceProp->GetSubclass());
		if (!Class)
			return "Engine.Object";
		return FINGenLuaSumnekoGetTypeName(Class);
	}
	case FIN_CLASS: {
		const UFINClassProperty *ClassProp = Cast<UFINClassProperty>(Prop);
		const UFINClass *Class = Ref.FindClass(ClassProp->GetSubclass());
		if (!Class)
			return "Engine.Object";
		return FINGenLuaSumnekoGetTypeName(Class);
	}
	case FIN_STRUCT: {
		const UFINStructProperty *StructProp = Cast<UFINStructProperty>(Prop);
		const UFINStruct *Struct = Ref.FindStruct(StructProp->GetInner());
		if (!Struct)
			return "any";
		return FINGenLuaSumnekoGetTypeName(Struct);
	}
	case FIN_ARRAY: {
		const UFINArrayProperty *ArrayProp = Cast<UFINArrayProperty>(Prop);
		return FINGenLuaSumnekoGetType(Ref, ArrayProp->GetInnerType()) + "[]";
	}
	default:
		return "any";
	}
}

FString FormatDescription(FString Description) {
	// assuming its only LF line endings
	Description.ReplaceCharInline('\n', ' ');
	return Description;
}

void FINGenLuaSumnekoDescription(FString &Str, FString Description) {
	Description.ReplaceInline(TEXT("\r\n"), TEXT("\n"));

	FString Line;
	while (Description.Split(TEXT("\n"), &Line, &Description)) {
		Str.Append(TEXT("--- ") + Line + TEXT("<br>\n"));
	}
	Str.Append(TEXT("--- ") + Description + TEXT("\n"));
}

FString FINGenLuaSumnekoProperty(FFINReflection &Ref, const FString &Parent,
                                 const UFINProperty *Prop) {
	FString PropertyDocumentation = "\n";

	const EFINRepPropertyFlags PropFlags = Prop->GetPropertyFlags();
	FINGenLuaSumnekoDescription(PropertyDocumentation, TEXT("### Flags:"));

	if (PropFlags & FIN_Prop_RT_Sync) {
		FINGenLuaSumnekoDescription(PropertyDocumentation,
		                            TEXT("* Runtime Synchronous - Can be called/changed in Game Tick."));
	}

	if (PropFlags & FIN_Prop_RT_Parallel) {
		FINGenLuaSumnekoDescription(PropertyDocumentation,
		                            TEXT("* Runtime Parallel - Can be called/changed in Satisfactory Factory Tick."));
	}

	if (PropFlags & FIN_Prop_RT_Async) {
		FINGenLuaSumnekoDescription(PropertyDocumentation, TEXT("* Runtime Asynchronous - Can be changed anytime."));
	}

	if (PropFlags & FIN_Prop_ReadOnly) {
		FINGenLuaSumnekoDescription(PropertyDocumentation,
		                            TEXT("* Read Only - The value of this property can not be changed by code."));
	}

	PropertyDocumentation.Append(FString::Printf(
		TEXT("---@type %s\n%s.%s = nil\n"), *FINGenLuaSumnekoGetType(Ref, Prop), *Parent, *Prop->GetInternalName()));

	return PropertyDocumentation;
}

FString FINGenLuaSumnekoFunction(FFINReflection &Ref, const FString &Parent,
                                 const UFINFunction *Func) {
	FString FunctionDocumentation = "\n";

	FINGenLuaSumnekoDescription(FunctionDocumentation, Func->GetDescription().ToString());

	const EFINFunctionFlags funcFlags = Func->GetFunctionFlags();
	FINGenLuaSumnekoDescription(FunctionDocumentation, TEXT("### Flags:"));

	if (funcFlags & FIN_Func_RT_Sync) {
		FINGenLuaSumnekoDescription(FunctionDocumentation,
		                            TEXT("* Runtime Synchronous - Can be called/changed in Game Tick."));
	}

	if (funcFlags & FIN_Func_RT_Parallel) {
		FINGenLuaSumnekoDescription(FunctionDocumentation,
		                            TEXT("* Runtime Parallel - Can be called/changed in Satisfactory Factory Tick."));
	}

	if (funcFlags & FIN_Func_RT_Async) {
		FINGenLuaSumnekoDescription(FunctionDocumentation, TEXT("* Runtime Asynchronous - Can be changed anytime."));
	}

	FString ParamDocumentation;
	FString ReturnDocumentation;
	FString ParamList;
	for (const UFINProperty *Prop : Func->GetParameters()) {
		const EFINRepPropertyFlags Flags = Prop->GetPropertyFlags();

		if (!(Flags & FIN_Prop_Param)) {
			continue;
		}

		if (Flags & FIN_Prop_OutParam) {
			ReturnDocumentation.Append(FString::Printf(TEXT("---@return %s %s @%s\n"),
			                                           *FINGenLuaSumnekoGetType(Ref, Prop),
			                                           *Prop->GetInternalName(),
			                                           *FormatDescription(Prop->GetDescription().ToString())));
		} else {
			ParamDocumentation.Append(FString::Printf(
				TEXT("---@param %s %s @%s\n"),
				*Prop->GetInternalName(),
				*FINGenLuaSumnekoGetType(Ref, Prop),
				*FormatDescription(Prop->GetDescription().ToString())
			));

			if (ParamList.Len() > 0) {
				ParamList.Append(", ");
			}

			ParamList.Append(Prop->GetInternalName());
		}
	}

	if (funcFlags & FIN_Func_VarArgs) {
		ParamDocumentation.Append(TEXT("---@param ... any @additional arguments as described\n"));

		if (ParamList.Len() > 0) {
			ParamList.Append(", ");
		}

		ParamList.Append("...");
	}

	FunctionDocumentation.Append(
		FString::Printf(TEXT("%s%sfunction %s:%s(%s) end\n"),
		                *ParamDocumentation,
		                *ReturnDocumentation,
		                *Parent,
		                *Func->GetInternalName(),
		                *ParamList));

	return FunctionDocumentation;
}

FString FINGenLuaSumnekoSignal(FFINReflection &Ref, const FString &Parent,
                               const UFINSignal *Signal) {
	FString SignalDocumentation = "\n";

	FINGenLuaSumnekoDescription(SignalDocumentation, Signal->GetDescription().ToString() + TEXT("\n"));

	FINGenLuaSumnekoDescription(SignalDocumentation, TEXT("### returns from event.pull:\n```"));

	SignalDocumentation.Append(TEXT("--- local signalName, component"));
	for (const UFINProperty *Prop : Signal->GetParameters()) {
		SignalDocumentation.Append(TEXT(", ") + Prop->GetDisplayName().ToString().Replace(TEXT(" "), TEXT("")));
	}
	if (Signal->IsVarArgs()) {
		SignalDocumentation.Append(TEXT(", ..."));
	}
	SignalDocumentation.Append(TEXT(" = event.pull()\n--- ```\n"));

	FINGenLuaSumnekoDescription(SignalDocumentation,
	                            FString::Printf(TEXT("- `signalName: \"%s\"`"), *Signal->GetInternalName()));
	FINGenLuaSumnekoDescription(SignalDocumentation,
	                            FString::Printf(TEXT("- `component: %s`"), *Parent));

	for (const UFINProperty *Prop : Signal->GetParameters()) {
		FINGenLuaSumnekoDescription(SignalDocumentation,
		                            FString::Printf(
			                            TEXT("- `%s: %s` \n%s"),
			                            *Prop->GetDisplayName().ToString().Replace(TEXT(" "), TEXT("")),
			                            *FINGenLuaSumnekoGetType(Ref, Prop),
			                            *Prop->GetDescription().ToString()
		                            ));
	}

	// hard coding the type is maybe not the best choice
	SignalDocumentation.Append(TEXT("---@deprecated\n---@type FIN.Signal\n"));
	SignalDocumentation.Append(FString::Printf(
		TEXT("%s.%s = { isVarArgs = %s }\n"),
		*Parent,
		*Signal->GetInternalName(),
		Signal->IsVarArgs()
			? TEXT("true")
			: TEXT("false")
	));

	return SignalDocumentation;
}

void FINGenLuaSumnekoClass(FString &Documentation, FFINReflection &Ref, const UFINClass *Class) {
	Documentation.Append(TEXT("\n"));

	FINGenLuaSumnekoDescription(Documentation, Class->GetDescription().ToString());
	Documentation.Append(FString::Printf(
		TEXT("---@class %s%s\nlocal %s\n"),
		*FINGenLuaSumnekoGetTypeName(Class),
		*(Class->GetParent()
			  ? TEXT(" : ") + FINGenLuaSumnekoGetTypeName(Class->GetParent())
			  : TEXT("")),
		*Class->GetInternalName()
	));

	for (const UFINProperty *Prop : Class->GetProperties(false)) {
		if (Prop->GetPropertyFlags() & FIN_Prop_Attrib) {
			Documentation.Append(FINGenLuaSumnekoProperty(Ref, Class->GetInternalName(), Prop));
		}
	}

	for (const UFINFunction *Func : Class->GetFunctions(false)) {
		//TODO: filter FIN_Operators
		if (Func->GetFunctionFlags() & FIN_Func_MemberFunc) {
			Documentation.Append(FINGenLuaSumnekoFunction(Ref, Class->GetInternalName(), Func));
		}
	}

	for (const UFINSignal *Signal : Class->GetSignals(false)) {
		Documentation.Append(FINGenLuaSumnekoSignal(Ref, Class->GetInternalName(), Signal));
	}
}

void FINGenLuaSumnekoStruct(FString &Documentation, FFINReflection &Ref, const UFINStruct *Struct) {
	Documentation.Append(TEXT("\n"));

	FINGenLuaSumnekoDescription(Documentation, Struct->GetDescription().ToString());
	Documentation.Append(FString::Printf(
		TEXT("---@class %s\nlocal %s\n"),
		*FINGenLuaSumnekoGetTypeName(Struct),
		*Struct->GetInternalName()
	));

	for (const UFINProperty *Prop : Struct->GetProperties(false)) {
		if (Prop->GetPropertyFlags() & FIN_Prop_Attrib) {
			Documentation.Append(FINGenLuaSumnekoProperty(Ref, Struct->GetInternalName(), Prop));
		}
	}

	for (const UFINFunction *Func : Struct->GetFunctions(false)) {
		//TODO: filter FIN_Operators
		if (Func->GetFunctionFlags() & FIN_Func_MemberFunc) {
			Documentation.Append(FINGenLuaSumnekoFunction(Ref, Struct->GetInternalName(), Func));
		}
	}
}

bool FINGenLuaDocSumneko(UWorld *World, const TCHAR *Command, FOutputDevice &Ar) {
	if (FParse::Command(&Command, TEXT("FINGenLuaDocSumneko"))) {
		FFINReflection &Ref = *FFINReflection::Get();
		FString Documentation;
		Documentation.Append(FINGenLuaSumnekoDocumentationStart);
		
		FString ClassesString = "---@class FIN.classes\n";
		FString StructsString = "---@class FIN.structs\n";

		{
			// adding "do" and "end" to get rid of local maximum variables reached
			int32_t count = 0;
			for (TPair<UClass*, UFINClass*> const Class : Ref.GetClasses()) {
				if (count == 0) {
					Documentation.Append("do\n");
				}

				FINGenLuaSumnekoClass(Documentation, Ref, Class.Value);
				ClassesString.Append(FString::Printf(
					TEXT("---@field %s %s\n"),
					*Class.Value->GetInternalName(),
					*FINGenLuaSumnekoGetTypeName(Class.Value)
				));
				count++;

				if (count == 180) {
					Documentation.Append("\nend\n");
					count = 0;
				}
			}
			for (TPair<UScriptStruct*, UFINStruct*> const Struct : Ref.GetStructs()) {
				if (count == 0) {
					Documentation.Append("do\n");
				}

				FINGenLuaSumnekoStruct(Documentation, Ref, Struct.Value);
				StructsString.Append(FString::Printf(
					TEXT("---@field %s %s\n"),
					*Struct.Value->GetInternalName(),
					*FINGenLuaSumnekoGetTypeName(Struct.Value)
				));
				count++;

				if (count == 180) {
					Documentation.Append("\nend\n");
					count = 0;
				}
			}

			if (count != 0) {
				Documentation.Append("\nend\n");
			}
		}

		// this looks wierd, but gets indented this way by my formatting settings
		Documentation.Append(FString::Printf(TEXT(R"(
%s
classes = {}

%s
strcuts = {}
)"), *ClassesString, *StructsString));

		Documentation.Append(FINGenLuaSumnekoDocumentationEnd);

		FString Path = FPaths::Combine(FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT("Saved/"));
		Path = FPaths::Combine(Path, TEXT("FINLuaDocumentationSumneko.lua"));
		FFileHelper::SaveStringToFile(Documentation, *Path);

		return true;
	}
	return false;
}
#pragma optimize("", on)

[[maybe_unused]]
static FStaticSelfRegisteringExec FINGenLuaDocSumnekoStaticExec(&FINGenLuaDocSumneko);
