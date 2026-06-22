// FIN-1.2-PORT: Header-free Portal reflection.
//
// The portal C++ headers (FGBuildablePortalBase.h / FGBuildablePortal.h /
// FGBuildablePortalSatellite.h) MUST NOT be included here: the teleport
// UFUNCTIONs reference AFGCharacterPlayer, which transitively pulls in the
// Chaos physics headers. ChaosSolverConfiguration.generated.h does not compile
// inside this mod module's UHT macro context (nested-enum UEnum specialization
// bug: TIsUEnumClass<Chaos::FConstraintSolverBody::EClusterUnionMethod>).
// Verified that bUseUnity=false, Chaos/PhysicsCore module deps and pre-including
// SolverBody.h all fail to fix it. So we avoid the headers entirely.
//
// Instead of FIN's BeginClass(...) macros (which bind to Type::StaticClass() and
// thus need the C++ type), we register the FIN reflection classes MANUALLY:
//  - The portal UClasses are resolved at runtime via FindObject<UClass> on their
//    /Script/FactoryGame.* path. The register lambdas run at
//    ELifecyclePhase::CONSTRUCTION (FIRModModule) / OnPostEngineInit, long after
//    the engine UClasses are loaded, so the lookup succeeds.
//  - All portal operations use generic Unreal reflection (FProperty reads +
//    UFunction/ProcessEvent calls), never a static_cast to the portal type.
//
// FillData() auto-parents each FIN class from Class->GetSuperClass(), so
// registering all three UClasses yields the correct hierarchy
// (Main/Satellite -> PortalBase -> ...Factory chain that FIN already reflects).

#include "Reflection/Source/FIRSourceStaticMacros.h"

// IMPORTANT: do NOT include FGInventoryComponent.h (or any engine header that
// transitively pulls in Chaos/Evolution/SolverBody.h). FIN's macros header above
// defines a function-like macro `Body()`; in a unity build it leaks into
// SolverBody.h's member-init line `: Body(nullptr)` and expands it -> C4002 plus
// a cascade of Chaos compile errors. We therefore resolve UFGInventoryComponent's
// UClass at runtime via FindObject (header-free), the same way we resolve the
// portal UClasses, and keep this translation unit free of physics headers.
#include "FIRException.h"
#include "UObject/UnrealType.h"

namespace FIR_PortalReflection {

// ---- runtime-resolved portal UClasses (filled lazily on first registration) ----
static UClass* GetPortalBaseClass() {
	return FindObject<UClass>(nullptr, TEXT("/Script/FactoryGame.BuildablePortalBase"));
}
static UClass* GetPortalMainClass() {
	return FindObject<UClass>(nullptr, TEXT("/Script/FactoryGame.BuildablePortal"));
}
static UClass* GetPortalSatelliteClass() {
	return FindObject<UClass>(nullptr, TEXT("/Script/FactoryGame.BuildablePortalSatellite"));
}

// ---------------------------------------------------------------------------
// Generic reflection helpers
// ---------------------------------------------------------------------------

// Read an FProperty value generically. Returns false if the property is missing.
template<typename FPropType, typename ValueType>
static bool ReadProp(UObject* Obj, const TCHAR* Name, ValueType& Out) {
	if (!IsValid(Obj)) return false;
	FProperty* P = Obj->GetClass()->FindPropertyByName(FName(Name));
	FPropType* Typed = CastField<FPropType>(P);
	if (!Typed) return false;
	Out = Typed->GetPropertyValue_InContainer(Obj);
	return true;
}

// Read a TObjectPtr/raw object FObjectProperty generically.
static UObject* ReadObjectProp(UObject* Obj, const TCHAR* Name) {
	if (!IsValid(Obj)) return nullptr;
	FObjectProperty* P = CastField<FObjectProperty>(Obj->GetClass()->FindPropertyByName(FName(Name)));
	if (!P) return nullptr;
	return P->GetObjectPropertyValue_InContainer(Obj);
}

// Read an FText property generically.
static bool ReadTextProp(UObject* Obj, const TCHAR* Name, FText& Out) {
	if (!IsValid(Obj)) return false;
	FTextProperty* P = CastField<FTextProperty>(Obj->GetClass()->FindPropertyByName(FName(Name)));
	if (!P) return false;
	Out = P->GetPropertyValue_InContainer(Obj);
	return true;
}

// Call a no-arg, single-float-return BlueprintPure UFunction via ProcessEvent.
static bool CallFloatGetter(UObject* Obj, const TCHAR* FuncName, float& Out) {
	if (!IsValid(Obj)) return false;
	UFunction* Fn = Obj->FindFunction(FName(FuncName));
	if (!Fn) return false;
	struct { float ReturnValue; } Params{0.0f};
	Obj->ProcessEvent(Fn, &Params);
	Out = Params.ReturnValue;
	return true;
}

// Class checks.
static bool IsSatellite(UObject* Obj) {
	UClass* Sat = GetPortalSatelliteClass();
	return Sat && IsValid(Obj) && Obj->GetClass()->IsChildOf(Sat);
}
static bool IsMain(UObject* Obj) {
	UClass* Main = GetPortalMainClass();
	return Main && IsValid(Obj) && Obj->GetClass()->IsChildOf(Main);
}

// ---------------------------------------------------------------------------
// Manual registration plumbing.
//
// We mirror the macro expansion (AddClass / AddFunction / AddFuncParam /
// AddProp / AddPropSetter) but feed a runtime-loaded UClass*. Counters give each
// member a unique, stable id within its class.
// ---------------------------------------------------------------------------

static FText NS(const FString& Key, const FString& Value) {
	return FText::AsLocalizable_Advanced(TEXT("FicsItNetworks-StaticReflection"), Key, Value);
}

struct FClassBuilder {
	UClass* Class;
	int FuncCounter = 0;
	int PropCounter = 0;

	FClassBuilder(UClass* InClass) : Class(InClass) {}

	void DefineClass(const TCHAR* Internal, const TCHAR* Display, const TCHAR* Desc) {
		UFIRSourceStatic::AddClass(Class, FFIRStaticClassReg{Internal, NS(FString(Internal) + TEXT("_DisplayName"), Display), NS(FString(Internal) + TEXT("_Description"), Desc)});
	}

	// Read-only property backed by a getter lambda.
	void Prop(const TCHAR* Internal, const TCHAR* Display, const TCHAR* Desc,
	          UFIRProperty*(*PropCtor)(UObject*),
	          TFunction<FIRAny(const FFIRExecutionContext&)> Getter,
	          int Runtime = 1) {
		int Id = PropCounter++;
		UFIRSourceStatic::AddProp(Class, Id, FFIRStaticPropReg{Internal, NS(FString(Internal) + TEXT("_DisplayName"), Display), NS(FString(Internal) + TEXT("_Description"), Desc), Getter, Runtime, 0, PropCtor});
	}

	// Read/write property. Setter runs game-thread (Runtime 0) by default.
	void PropRW(const TCHAR* Internal, const TCHAR* Display, const TCHAR* Desc,
	            UFIRProperty*(*PropCtor)(UObject*),
	            TFunction<FIRAny(const FFIRExecutionContext&)> Getter,
	            TFunction<void(const FFIRExecutionContext&, const FIRAny&)> Setter,
	            int Runtime = 0) {
		int Id = PropCounter++;
		UFIRSourceStatic::AddProp(Class, Id, FFIRStaticPropReg{Internal, NS(FString(Internal) + TEXT("_DisplayName"), Display), NS(FString(Internal) + TEXT("_Description"), Desc), Getter, Runtime, 0, PropCtor});
		UFIRSourceStatic::AddPropSetter(Class, Id, Setter);
	}

	// Function with explicit param specs.
	struct FParamSpec {
		const TCHAR* Internal;
		const TCHAR* Display;
		const TCHAR* Desc;
		int ParamType; // 0 = in, 1 = out, 2 = retval(out)
		UFIRProperty*(*PropCtor)(UObject*);
	};

	void Func(const TCHAR* Internal, const TCHAR* Display, const TCHAR* Desc,
	          int Runtime,
	          TFunction<void(const FFIRExecutionContext&, TArray<FIRAny>&)> Body,
	          std::initializer_list<FParamSpec> Params) {
		int Id = FuncCounter++;
		UFIRSourceStatic::AddFunction(Class, Id, FFIRStaticFuncReg{Internal, NS(FString(Internal) + TEXT("_DisplayName"), Display), NS(FString(Internal) + TEXT("_Description"), Desc), false, Body, Runtime, 0});
		int Pos = 0;
		for (const FParamSpec& Spec : Params) {
			UFIRSourceStatic::AddFuncParam(Class, Id, Pos, FFIRStaticFuncParamReg{Spec.Internal, NS(FString(Internal) + TEXT("_") + Spec.Internal + TEXT("_DisplayName"), Spec.Display), NS(FString(Internal) + TEXT("_") + Spec.Internal + TEXT("_Description"), Spec.Desc), Spec.ParamType, Spec.PropCtor});
			++Pos;
		}
	}
};

// PropConstructor trampolines (same pattern the R* structs use).
static UFIRProperty* MakeStrProp(UObject* Outer)  { return RString::PropConstructor(Outer); }
static UFIRProperty* MakeBoolProp(UObject* Outer) { return RBool::PropConstructor(Outer); }
static UFIRProperty* MakeFloatProp(UObject* Outer){ return RFloat::PropConstructor(Outer); }

// ---------------------------------------------------------------------------
// Operation bodies (shared base portal)
// ---------------------------------------------------------------------------

static FIRAny Get_Name(const FFIRExecutionContext& Ctx) {
	FText T;
	ReadTextProp(Ctx.GetObject(), TEXT("mPortalName"), T);
	return (FIRAny)(FIRStr)T.ToString();
}

static void Set_Name(const FFIRExecutionContext& Ctx, const FIRAny& Val) {
	UObject* Obj = Ctx.GetObject();
	if (!IsValid(Obj)) return;
	// SetPortalName(const FText&) does representation/UI work -> game thread (Runtime 0).
	UFunction* Fn = Obj->FindFunction(FName(TEXT("SetPortalName")));
	if (!Fn) return;
	struct { FText InPortalName; } Params;
	Params.InPortalName = FText::FromString(Val.GetString());
	Obj->ProcessEvent(Fn, &Params);
}

static FIRAny Get_IsTraversable(const FFIRExecutionContext& Ctx) {
	bool b = false;
	ReadProp<FBoolProperty>(Ctx.GetObject(), TEXT("mIsPortalTraversable"), b);
	return (FIRAny)(FIRBool)b;
}

static FIRAny Get_IsMain(const FFIRExecutionContext& Ctx) {
	return (FIRAny)(FIRBool)IsMain(Ctx.GetObject());
}

static FIRAny Get_IsSatellite(const FFIRExecutionContext& Ctx) {
	return (FIRAny)(FIRBool)IsSatellite(Ctx.GetObject());
}

static void Exec_GetLinkedPortal(const FFIRExecutionContext& Ctx, TArray<FIRAny>& Params) {
	UObject* linked = ReadObjectProp(Ctx.GetObject(), TEXT("mLinkedPortal"));
	Params[0] = (FIRAny)(FIRTrace)(Ctx.GetTrace() / linked);
}

static void Exec_LinkTo(const FFIRExecutionContext& Ctx, TArray<FIRAny>& Params) {
	UObject* self = Ctx.GetObject();
	if (!IsValid(self)) throw FFIRException(TEXT("Invalid portal"));
	UObject* other = Params[0].GetTrace().Get();
	if (!IsValid(other)) throw FFIRException(TEXT("Invalid portal to link to"));
	UClass* Base = GetPortalBaseClass();
	if (Base && !other->GetClass()->IsChildOf(Base)) throw FFIRException(TEXT("Target is not a portal"));
	// MakeLinkToPortal(AFGBuildablePortalBase* otherPortal) -> mutates state, game thread.
	UFunction* Fn = self->FindFunction(FName(TEXT("MakeLinkToPortal")));
	if (!Fn) throw FFIRException(TEXT("MakeLinkToPortal not found"));
	struct { UObject* OtherPortal; } P;
	P.OtherPortal = other;
	self->ProcessEvent(Fn, &P);
}

static void Exec_Unlink(const FFIRExecutionContext& Ctx, TArray<FIRAny>& Params) {
	UObject* self = Ctx.GetObject();
	if (!IsValid(self)) throw FFIRException(TEXT("Invalid portal"));
	// DisconnectLinkedPortal() -> mutates state, game thread.
	UFunction* Fn = self->FindFunction(FName(TEXT("DisconnectLinkedPortal")));
	if (!Fn) throw FFIRException(TEXT("DisconnectLinkedPortal not found"));
	self->ProcessEvent(Fn, nullptr);
}

static void Exec_GetEstPower(const FFIRExecutionContext& Ctx, TArray<FIRAny>& Params) {
	float v = 0.0f;
	CallFloatGetter(Ctx.GetObject(), TEXT("GetEstimatedPowerConsumptionForTeleport"), v);
	Params[0] = (FIRAny)(FIRFloat)v;
}

// ---- Main-portal-only ops ----

static FIRAny Get_IsHeatUpComplete(const FFIRExecutionContext& Ctx) {
	bool b = false;
	ReadProp<FBoolProperty>(Ctx.GetObject(), TEXT("mHeatUpComplete"), b);
	return (FIRAny)(FIRBool)b;
}

static FIRAny Get_HeatUpProgress(const FFIRExecutionContext& Ctx) {
	float v = 0.0f;
	// mCurrentHeatUpProgress is a float UPROPERTY on the main portal.
	ReadProp<FFloatProperty>(Ctx.GetObject(), TEXT("mCurrentHeatUpProgress"), v);
	return (FIRAny)(FIRFloat)v;
}

static void Exec_GetFuelInventory(const FFIRExecutionContext& Ctx, TArray<FIRAny>& Params) {
	UObject* inv = ReadObjectProp(Ctx.GetObject(), TEXT("mFuelInventory"));
	Params[0] = (FIRAny)(FIRTrace)(Ctx.GetTrace() / inv);
}

static UFIRProperty* MakePortalBaseTraceProp(UObject* Outer) {
	UFIRTraceProperty* Prop = NewObject<UFIRTraceProperty>(Outer);
	UClass* Base = GetPortalBaseClass();
	Prop->Subclass = Base ? Base : UObject::StaticClass();
	return Prop;
}

static UFIRProperty* MakeInventoryTraceProp(UObject* Outer) {
	UFIRTraceProperty* Prop = NewObject<UFIRTraceProperty>(Outer);
	UClass* InvClass = FindObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGInventoryComponent"));
	Prop->Subclass = InvClass ? InvClass : UObject::StaticClass();
	return Prop;
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

static void RegisterPortalReflection() {
	UClass* BaseClass = GetPortalBaseClass();
	UClass* MainClass = GetPortalMainClass();
	UClass* SatClass  = GetPortalSatelliteClass();

	if (!BaseClass) {
		UE_LOG(LogTemp, Warning, TEXT("[FIN] Portal reflection: AFGBuildablePortalBase UClass not found, skipping."));
		return;
	}

	// ----- Base portal (covers Main + Satellite) -----
	{
		FClassBuilder B(BaseClass);
		B.DefineClass(TEXT("Portal"), TEXT("Portal"),
			TEXT("A portal building (main or satellite) used for teleporting players. Covers naming, linking/pairing, traversability and teleport-power estimate shared by both portal types."));

		B.PropRW(TEXT("name"), TEXT("Name"),
			TEXT("The portal's name (shown in the UI / on the map). Settable."),
			&MakeStrProp, &Get_Name, &Set_Name);

		B.Prop(TEXT("isTraversable"), TEXT("Is Traversable"),
			TEXT("True if the portal is currently open for transfer with its linked portal (heated up + linked)."),
			&MakeBoolProp, &Get_IsTraversable);

		B.Prop(TEXT("isMainPortal"), TEXT("Is Main Portal"),
			TEXT("True if this is a main (fuel-burning) portal."),
			&MakeBoolProp, &Get_IsMain);

		B.Prop(TEXT("isSatellite"), TEXT("Is Satellite"),
			TEXT("True if this is a satellite portal node."),
			&MakeBoolProp, &Get_IsSatellite);

		B.Func(TEXT("getLinkedPortal"), TEXT("Get Linked Portal"),
			TEXT("Returns the portal this one is currently linked/paired with, or nil if unlinked."),
			1, &Exec_GetLinkedPortal,
			{ {TEXT("portal"), TEXT("Portal"), TEXT("The linked portal, or nil."), 1, &MakePortalBaseTraceProp} });

		B.Func(TEXT("linkTo"), TEXT("Link To"),
			TEXT("Links/pairs this portal with the given other portal (establishes the teleport connection between a main and a satellite). Mutates game state."),
			0, &Exec_LinkTo,
			{ {TEXT("other"), TEXT("Other"), TEXT("The portal to link with."), 0, &MakePortalBaseTraceProp} });

		B.Func(TEXT("unlink"), TEXT("Unlink"),
			TEXT("Disconnects this portal from its linked partner, clearing the link on both portals. Mutates game state."),
			0, &Exec_Unlink, {});

		B.Func(TEXT("getEstimatedTeleportPower"), TEXT("Get Estimated Teleport Power"),
			TEXT("Returns the estimated power consumption (MW over 1 second) for a teleport from/to this portal."),
			1, &Exec_GetEstPower,
			{ {TEXT("power"), TEXT("Power"), TEXT("Estimated teleport power consumption."), 2, &MakeFloatProp} });
	}

	// ----- Main portal only -----
	if (MainClass) {
		FClassBuilder M(MainClass);
		M.DefineClass(TEXT("PortalMain"), TEXT("Main Portal"),
			TEXT("A main portal: burns fuel and heats up to power a teleport connection with a satellite portal."));

		M.Prop(TEXT("isHeatUpComplete"), TEXT("Is Heat Up Complete"),
			TEXT("True if this main portal has finished its heat-up sequence and is ready to open a connection."),
			&MakeBoolProp, &Get_IsHeatUpComplete);

		M.Prop(TEXT("heatUpProgress"), TEXT("Heat Up Progress"),
			TEXT("Current heat-up progress of this main portal, normalized 0.0 (cool) .. 1.0 (active)."),
			&MakeFloatProp, &Get_HeatUpProgress);

		M.Func(TEXT("getFuelInventory"), TEXT("Get Fuel Inventory"),
			TEXT("Returns the fuel inventory of this main portal."),
			1, &Exec_GetFuelInventory,
			{ {TEXT("inventory"), TEXT("Inventory"), TEXT("The fuel inventory, or nil."), 1, &MakeInventoryTraceProp} });
	} else {
		UE_LOG(LogTemp, Warning, TEXT("[FIN] Portal reflection: AFGBuildablePortal (main) UClass not found, main-only ops skipped."));
	}

	// ----- Satellite portal -----
	if (SatClass) {
		FClassBuilder S(SatClass);
		S.DefineClass(TEXT("PortalSatellite"), TEXT("Satellite Portal"),
			TEXT("A satellite portal node: links to a main portal to receive a teleport connection."));
	} else {
		UE_LOG(LogTemp, Warning, TEXT("[FIN] Portal reflection: AFGBuildablePortalSatellite UClass not found, satellite class skipped."));
	}
}

// Register at the same deferred point the macros use (FFIRGlobalRegisterHelper),
// which runs at module construction / OnPostEngineInit when the engine UClasses
// already exist.
static FFIRStaticGlobalRegisterFunc RegPortal([]() {
	RegisterPortalReflection();
});

} // namespace FIR_PortalReflection
