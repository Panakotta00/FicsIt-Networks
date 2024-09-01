#include "Reflection/Source/FIRSourceStaticMacros.h"

BeginClass(UFIRBase, "ReflectionBase", "Reflection Base", "The base class for all things of the reflection system.")
BeginProp(RString, name, "Name", "The internal name.") {
	Return self->GetInternalName();
} EndProp()
BeginProp(RString, displayName, "Display Name", "The display name used in UI which might be localized.") {
	Return self->GetDisplayName().ToString();
} EndProp()
BeginProp(RString, description, "Description", "The description of this base.") {
	Return self->GetDescription().ToString();
} EndProp()
EndClass()

BeginClass(UFIRStruct, "Struct", "Struct", "Reflection Object that holds information about structures.")
BeginProp(RBool, isConstructable, "Is Constructable", "True if this struct can be constructed by the user directly.") {
	Return (FIRBool)(self->GetStructFlags() & FIR_Struct_Constructable);
} EndProp()
BeginFunc(getParent, "Get Parent", "Returns the parent type of this type.", false) {
	OutVal(0, RObject<UFIRClass>, parent, "Parent", "The parent type of this type.");
	Body()
    if (self) parent = (FIRObj)self->GetParent();
} EndFunc()
BeginFunc(getProperties, "Get Properties", "Returns all the properties of this type.") {
	OutVal(0, RArray<RObject<UFIRProperty>>, properties, "Properties", "The properties this specific type implements (excluding properties from parent types).")
	Body()
	TArray<FIRAny> Props;
	for (UFIRProperty* Prop : self->GetProperties(false)) Props.Add((FIRObj)Prop);
	properties = Props;
} EndFunc()
BeginFunc(getAllProperties, "Get All Properties", "Returns all the properties of this and parent types.") {
	OutVal(0, RArray<RObject<UFIRProperty>>, properties, "Properties", "The properties this type implements including properties from parent types.")
    Body()
    TArray<FIRAny> Props;
	for (UFIRProperty* Prop : self->GetProperties(true)) Props.Add((FIRObj)Prop);
	properties = Props;
} EndFunc()
BeginFunc(getFunctions, "Get Functions", "Returns all the functions of this type.") {
	OutVal(0, RArray<RObject<UFIRFunction>>, functions, "Functions", "The functions this specific type implements (excluding properties from parent types).")
    Body()
    TArray<FIRAny> Funcs;
	for (UFIRFunction* Func : self->GetFunctions(false)) Funcs.Add((FIRObj)Func);
	functions = Funcs;
} EndFunc()
BeginFunc(getAllFunctions, "Get All Functions", "Returns all the functions of this and parent types.") {
	OutVal(0, RArray<RObject<UFIRProperty>>, functions, "Functions", "The functions this type implements including functions from parent types.")
    Body()
    TArray<FIRAny> Funcs;
	for (UFIRFunction* Func : self->GetFunctions(true)) Funcs.Add((FIRObj)Func);
	functions = Funcs;
} EndFunc()
BeginFunc(isChildOf, "Is Child Of", "Allows to check if this struct is a child struct of the given struct or the given struct it self.") {
	InVal(0, RObject<UFIRStruct>, parent, "Parent", "The parent struct you want to check if this struct is a child of.")
    OutVal(1, RBool, isChild, "Is Child", "True if this struct is a child of parent.")
    Body()
    if (self && parent.IsValid()) isChild = self->IsChildOf(Cast<UFIRStruct>(parent.Get()));
} EndFunc()
EndClass()

BeginClass(UFIRClass, "Class", "Class", "Object that contains all information about a type.")
BeginFunc(getSignals, "Get Signals", "Returns all the signals of this type.") {
	OutVal(0, RArray<RObject<UFIRSignal>>, signals, "Signals", "The signals this specific type implements (excluding properties from parent types).")
    Body()
    TArray<FIRAny> Sigs;
	for (UFIRSignal* Sig : self->GetSignals(false)) Sigs.Add((FIRObj)Sig);
	signals = Sigs;
} EndFunc()
BeginFunc(getAllSignals, "Get All Signals", "Returns all the signals of this and its parent types.") {
	OutVal(0, RArray<RObject<UFIRSignal>>, signals, "Signals", "The signals this type and all it parents implement.")
    Body()
    TArray<FIRAny> Sigs;
	for (UFIRSignal* Sig : self->GetSignals(true)) Sigs.Add((FIRObj)Sig);
	signals = Sigs;
} EndFunc()
EndClass()

BeginClass(UFIRProperty, "Property", "Property", "A Reflection object that holds information about properties and parameters.")
BeginProp(RInt, dataType, "Data Type", "The data type of this property.\n0: nil, 1: bool, 2: int, 3: float, 4: str, 5: object, 6: class, 7: trace, 8: struct, 9: array, 10: anything") {
	Return (FIRInt)self->GetType().GetValue();
} EndProp()
BeginProp(RInt, flags, "Flags", "The property bit flag register defining some behaviour of it.\n\nBits and their meaing (least significant bit first):\nIs this property a member attribute.\nIs this property read only.\nIs this property a parameter.\nIs this property a output paramter.\nIs this property a return value.\nCan this property get accessed in syncrounus runtime.\nCan this property can get accessed in parallel runtime.\nCan this property get accessed in asynchronus runtime.\nThis property is a class attribute.") {
	Return (FIRInt) self->GetPropertyFlags();
} EndProp()
EndClass()

BeginClass(UFIRArrayProperty, "ArrayProperty", "Array Property", "A reflection object representing a array property.")
BeginFunc(getInner, "Get Inner", "Returns the inner type of this array.") {
	OutVal(0, RObject<UFIRProperty>, inner, "Inner", "The inner type of this array.")
	Body()
	inner = (FIRObj) self->GetInnerType();
} EndFunc()
EndClass()

BeginClass(UFIRObjectProperty, "ObjectProperty", "Object Property", "A reflection object representing a object property.")
BeginFunc(getSubclass, "Get Subclass", "Returns the subclass type of this object. Meaning, the stored objects need to be of this type.") {
	OutVal(0, RObject<UFIRClass>, subclass, "Subclass", "The subclass of this object.")
    Body()
    subclass = (FIRObj) FFicsItReflectionModule::Get()->FindClass(self->GetSubclass());
} EndFunc()
EndClass()

BeginClass(UFIRTraceProperty, "TraceProperty", "Trace Property", "A reflection object representing a trace property.")
BeginFunc(getSubclass, "Get Subclass", "Returns the subclass type of this trace. Meaning, the stored traces need to be of this type.") {
	OutVal(0, RObject<UFIRClass>, subclass, "Subclass", "The subclass of this trace.")
    Body()
    subclass = (FIRObj) FFicsItReflectionModule::Get()->FindClass(self->GetSubclass());
} EndFunc()
EndClass()

BeginClass(UFIRClassProperty, "ClassProperty", "Class Property", "A reflection object representing a class property.")
BeginFunc(getSubclass, "Get Subclass", "Returns the subclass type of this class. Meaning, the stored classes need to be of this type.") {
	OutVal(0, RObject<UFIRClass>, subclass, "Subclass", "The subclass of this class property.")
    Body()
    subclass = (FIRObj) FFicsItReflectionModule::Get()->FindClass(self->GetSubclass());
} EndFunc()
EndClass()

BeginClass(UFIRStructProperty, "StructProperty", "Struct Property", "A reflection object representing a struct property.")
BeginFunc(getSubclass, "Get Subclass", "Returns the subclass type of this struct. Meaning, the stored structs need to be of this type.") {
	OutVal(0, RObject<UFIRStruct>, subclass, "Subclass", "The subclass of this struct.")
    Body()
    subclass = (FIRObj) FFicsItReflectionModule::Get()->FindStruct(self->GetInner());
} EndFunc()
EndClass()

BeginClass(UFIRFunction, "Function", "Function", "A reflection object representing a function.")
BeginFunc(getParameters, "Get Parameters", "Returns all the parameters of this function.") {
	OutVal(0, RArray<RObject<UFIRProperty>>, parameters, "Parameters", "The parameters this function.")
    Body()
    TArray<FIRAny> ParamArray;
	for (UFIRProperty* Param : self->GetParameters()) ParamArray.Add((FIRObj)Param);
	parameters = ParamArray;
} EndFunc()
BeginProp(RInt, flags, "Flags", "The function bit flag register defining some behaviour of it.\n\nBits and their meaing (least significant bit first):\nIs this function has a variable amount of input parameters.\nCan this function get called in syncrounus runtime.\nCan this function can get called in parallel runtime.\nCan this function get called in asynchronus runtime.\nIs this function a member function.\nThe function is a class function.\nThe function is a static function.\nThe function has a variable amount of return values.") {
	Return (FIRInt) self->GetFunctionFlags();
} EndProp()
EndClass()

BeginClass(UFIRSignal, "Signal", "Signal", "A reflection object representing a signal.")
BeginFunc(getParameters, "Get Parameters", "Returns all the parameters of this signal.") {
	OutVal(0, RArray<RObject<UFIRProperty>>, parameters, "Parameters", "The parameters this signal.")
    Body()
    TArray<FIRAny> ParamArray;
	for (UFIRProperty* Param : self->GetParameters()) ParamArray.Add((FIRObj)Param);
	parameters = ParamArray;
} EndFunc()
BeginProp(RBool, isVarArgs, "Is VarArgs", "True if this signal has a variable amount of arguments.") {
	Return (FIRBool) self->IsVarArgs();
} EndProp()
EndClass()
