#include "FGFactoryConnectionComponent.h"
#include "Reflection/Source/FIRSourceStaticMacros.h"

#include "FGPipeConnectionComponent.h"
#include "FGPowerConnectionComponent.h"
#include "FicsItReflection.h"

BeginClass(UObject, "Object", "Object", "The base class of every object.")
	BeginProp(RInt, hash, "Hash", "A Hash of this object. This is a value that nearly uniquely identifies this object.") {
	FIRReturn (int64)GetTypeHash(self->GetPathName());
} EndProp()
BeginProp(RString, internalName, "internalName", "The unreal engine internal name of this object.") {
	FIRReturn (FIRStr) self->GetName();
} EndProp()
BeginProp(RString, internalPath, "internalPath", "The unreal engine internal path name of this object.") {
	FIRReturn (FIRStr) self->GetPathName();
} EndProp()
BeginFunc(getHash, "Get Hash", "Returns a hash of this object. This is a value that nearly uniquely identifies this object.") {
	OutVal(0, RInt, hash, "Hash", "The hash of this object.");
	Body()
	hash = (int64)GetTypeHash(self->GetPathName());
} EndFunc()
BeginFunc(getType, "Get Type", "Returns the type (aka class) of this object.") {
	OutVal(0, RObject<UFIRClass>, type, "Type", "The type of this object");
	Body()
	if (self) type = (FIRObj)FFicsItReflectionModule::Get().FindClass(self->GetClass());
} EndFunc()
BeginFunc(isA, "Is A", "Checks if this Object is a child of the given typen.") {
	InVal(0, RClass<UObject>, parent, "Parent", "The parent we check if this object is a child of.")
	OutVal(1, RBool, isChild, "Is Child", "True if this object is a child of the given type.")
	Body()
	isChild = (FIRBool)self->IsA(parent);
} EndFunc()
BeginClassProp(RInt, hash, "Hash", "A Hash of this object. This is a value that nearly uniquely identifies this object.") {
	FIRReturn (int64)GetTypeHash(self->GetPathName());
} EndProp()
BeginClassProp(RString, internalName, "internalName", "The unreal engine internal name of this object.") {
	FIRReturn (FIRStr) self->GetName();
} EndProp()
BeginClassProp(RString, internalPath, "internalPath", "The unreal engine internal path name of this object.") {
	FIRReturn (FIRStr) self->GetPathName();
} EndProp()
BeginClassFunc(getHash, "Get Hash", "Returns the hash of this class. This is a value that nearly uniquely idenfies this object.", false) {
	OutVal(0, RInt, hash, "Hash", "The hash of this class.");
	Body()
	hash = (int64) GetTypeHash(self->GetPathName());
} EndFunc()
BeginClassFunc(getType, "Get Type", "Returns the type (aka class) of this class instance.", false) {
	OutVal(0, RObject<UFIRClass>, type, "Type", "The type of this class instance");
	Body()
    if (self) type = (FIRObj)FFicsItReflectionModule::Get().FindClass(self);
} EndFunc()
BeginClassFunc(isChildOf, "Is Child Of", "Checks if this Type is a child of the given typen.", false) {
	InVal(0, RClass<UObject>, parent, "Parent", "The parent we check if this type is a child of.")
	OutVal(1, RBool, isChild, "Is Child", "True if this type is a child of the given type.")
	Body()
	isChild = (FIRBool)self->IsChildOf(parent);
} EndFunc()
EndClass()

BeginClass(AActor, "Actor", "Actor", "This is the base class of all things that can exist within the world by them self.")
BeginProp(RStruct<FVector>, location, "Location", "The location of the actor in the world.") {
	FIRReturn self->GetActorLocation();
} EndProp()
BeginProp(RStruct<FVector>, scale, "Scale", "The scale of the actor in the world.") {
	FIRReturn self->GetActorScale();
} EndProp()
BeginProp(RStruct<FRotator>, rotation, "Rotation", "The rotation of the actor in the world.") {
	FIRReturn self->GetActorRotation();
} EndProp()
BeginFunc(getPowerConnectors, "Get Power Connectors", "Returns a list of power connectors this actor might have.") {
	OutVal(0, RArray<RTrace<UFGPowerConnectionComponent>>, connectors, "Connectors", "The power connectors this actor has.");
	Body()
	FIRArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFGPowerConnectionComponent::StaticClass())) continue;
		UObject* Connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(Connector))) continue;
		Output.Add(Ctx.GetTrace() / Connector);
	}
	connectors = Output;
} EndFunc()
BeginFunc(getFactoryConnectors, "Get Factory Connectors", "Returns a list of factory connectors this actor might have.") {
	OutVal(0, RArray<RTrace<UFGFactoryConnectionComponent>>, connectors, "Connectors", "The factory connectors this actor has.");
	Body()
	FIRArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFGFactoryConnectionComponent::StaticClass())) continue;
		UObject* Connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(Connector))) continue;
		Output.Add(Ctx.GetTrace() / Connector);
	}
	connectors = Output;
} EndFunc()
BeginFunc(getPipeConnectors, "Get Pipe Connectors", "Returns a list of pipe (fluid & hyper) connectors this actor might have.") {
	OutVal(0, RArray<RTrace<UFGPipeConnectionComponentBase>>, connectors, "Connectors", "The pipe connectors this actor has.");
	Body()
	FIRArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	TSet<UObject*> Outputted;
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFGPipeConnectionComponentBase::StaticClass())) continue;
		UObject* Connector = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(Connector))) continue;
		if (Outputted.Contains(Connector)) continue;
		Outputted.Add(Connector);
		Output.Add(Ctx.GetTrace() / Connector);
	}
	connectors = Output;
} EndFunc()
BeginFunc(getInventories, "Get Inventories", "Returns a list of inventories this actor might have.") {
	OutVal(0, RArray<RTrace<UFGInventoryComponent>>, inventories, "Inventories", "The inventories this actor has.");
	Body()
	FIRArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(UFGInventoryComponent::StaticClass())) continue;
		UObject* inventory = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(inventory))) continue;
		Output.Add(Ctx.GetTrace() / inventory);
	}
	inventories = Output;
} EndFunc()
BeginFunc(getComponents, "Get Components", "Returns the components that make-up this actor.") {
	InVal(0, RClass<UActorComponent>, componentType, "Component Type", "The class will be used as filter.")
	OutVal(1, RArray<RTrace<UActorComponent>>, components, "Components", "The components of this actor.")
	Body()
	FIRArray Output;
	const TSet<UActorComponent*>& Components = self->GetComponents();
	for (TFieldIterator<FObjectProperty> prop(self->GetClass()); prop; ++prop) {
		if (!prop->PropertyClass->IsChildOf(componentType)) continue;
		UObject* component = *prop->ContainerPtrToValuePtr<UObject*>(self);
		if (!Components.Contains(Cast<UActorComponent>(component))) continue;
		Output.Add(Ctx.GetTrace() / component);
	}
	components = Output;
} EndFunc()
EndClass()

BeginClass(UActorComponent, "ActorComponent", "Actor Component", "A component/part of an actor in the world.")
BeginProp(RTrace<AActor>, owner, "Owner", "The parent actor of which this component is part of") {
	return Ctx.GetTrace() / self->GetOwner();
} EndProp()
EndClass()

BeginStructConstructable(FVector2D, "Vector2D", "Vector 2D", "Contains two cordinates (X, Y) to describe a position or movement vector in 2D Space")
BeginProp(RFloat, x, "X", "The X coordinate component", 2) {
	FIRReturn self->X;
} PropSet() {
	self->X = Val;
} EndProp()
BeginProp(RFloat, y, "Y", "The Y coordinate component", 2) {
	FIRReturn self->Y;
} PropSet() {
	self->Y = Val;
} EndProp()
BeginOp(FIR_Operator_Add, 0, "Operator Add", "The addition (+) operator for this struct.", 2) {
	InVal(0, RStruct<FVector2D>, other, "Other", "The other vector that should be added to this vector")
	OutVal(1, RStruct<FVector2D>, result, "Result", "The resulting vector of the vector addition")
	Body()
	result = (FIRStruct)(*self + other);
} EndFunc()
BeginOp(FIR_Operator_Sub, 0, "Operator Sub", "The subtraction (-) operator for this struct.", 2) {
	InVal(0, RStruct<FVector2D>, other, "Other", "The other vector that should be subtracted from this vector")
	OutVal(1, RStruct<FVector2D>, result, "Result", "The resulting vector of the vector subtraction")
	Body()
	result = (FIRStruct)(*self - other);
} EndFunc()
BeginOp(FIR_Operator_Neg, 0, "Operator Neg", "The Negation operator for this struct.", 2) {
	OutVal(0, RStruct<FVector2D>, result, "Result", "The resulting vector of the vector negation")
	Body()
	result = (FIRStruct)(-*self);
} EndFunc()
BeginOp(FIR_Operator_Mul, 0, "Scalar Product", "", 2) {
	InVal(0, RStruct<FVector2D>, other, "Other", "The other vector to calculate the scalar product with.")
	OutVal(1, RFloat, result, "Result", "The resulting scalar product.")
	Body()
	result = (FIRStruct)(*self * other);
} EndFunc()
BeginOp(FIR_Operator_Mul, 1, "Vector Factor Scaling", "", 2) {
	InVal(0, RFloat, factor, "Factor", "The factor with which this vector should be scaled with.")
	OutVal(1, RStruct<FVector2D>, result, "Result", "The resulting scaled vector.")
	Body()
	result = (FIRStruct)(*self * factor);
} EndFunc()
EndStruct()

BeginStructConstructable(FVector, "Vector", "Vector", "Contains three cordinates (X, Y, Z) to describe a position or movement vector in 3D Space")
BeginProp(RFloat, x, "X", "The X coordinate component", 2) {
	FIRReturn self->X;
} PropSet() {
	self->X = Val;
} EndProp()
BeginProp(RFloat, y, "Y", "The Y coordinate component", 2) {
	FIRReturn self->Y;
} PropSet() {
	self->Y = Val;
} EndProp()
BeginProp(RFloat, z, "Z", "The Z coordinate component", 2) {
	FIRReturn self->Z;
} PropSet() {
	self->Z = Val;
} EndProp()
BeginOp(FIR_Operator_Add, 0, "Operator Add", "The addition (+) operator for this struct.", 2) {
	InVal(0, RStruct<FVector>, other, "Other", "The other vector that should be added to this vector")
	OutVal(1, RStruct<FVector>, result, "Result", "The resulting vector of the vector addition")
	Body()
	result = (FIRStruct)(*self + other);
} EndFunc()
BeginOp(FIR_Operator_Sub, 0, "Operator Sub", "The subtraction (-) operator for this struct.", 2) {
	InVal(0, RStruct<FVector>, other, "Other", "The other vector that should be subtracted from this vector")
	OutVal(1, RStruct<FVector>, result, "Result", "The resulting vector of the vector subtraction")
	Body()
	result = (FIRStruct)(*self - other);
} EndFunc()
BeginOp(FIR_Operator_Neg, 0, "Operator Neg", "The Negation operator for this struct.", 2) {
	OutVal(0, RStruct<FVector>, result, "Result", "The resulting vector of the vector negation")
	Body()
	result = (FIRStruct)(-*self);
} EndFunc()
BeginOp(FIR_Operator_Mul, 0, "Operator Mul", "", 2) {
	InVal(0, RStruct<FVector>, other, "Other", "The multiplication (*) operator for this struct. (Each component gets multiplied with the component of the other vector)")
	OutVal(1, RStruct<FVector>, result, "Result", "The resulting vector of the vector multiplication.")
	Body()
	result = (FIRStruct)(*self * other);
} EndFunc()
BeginOp(FIR_Operator_Mul, 1, "Vector Factor Scaling", "", 2) {
	InVal(0, RFloat, factor, "Factor", "The factor with which this vector should be scaled with.")
	OutVal(1, RStruct<FVector>, result, "Result", "The resulting scaled vector.")
	Body()
	result = (FIRStruct)(*self * factor);
} EndFunc()
EndStruct()

BeginStructConstructable(FRotator, "Rotator", "Rotator", "Contains rotation information about a object in 3D spaces using 3 rotation axis in a gimble.")
BeginProp(RFloat, pitch, "Pitch", "The pitch component", 2) {
	FIRReturn self->Pitch;
} PropSet() {
	self->Pitch = Val;
} EndProp()
BeginProp(RFloat, yaw, "Yaw", "The yaw component", 2) {
	FIRReturn self->Yaw;
} PropSet() {
	self->Yaw = Val;
} EndProp()
BeginProp(RFloat, roll, "Roll", "The roll component", 2) {
	FIRReturn self->Roll;
} PropSet() {
	self->Roll = Val;
} EndProp()
BeginOp(FIR_Operator_Add, 0, "Operator Add", "The addition (+) operator for this struct.", 2) {
	InVal(0, RStruct<FRotator>, other, "Other", "The other rotator that should be added to this rotator")
	OutVal(1, RStruct<FRotator>, result, "Result", "The resulting rotator of the vector addition")
	Body()
	result = (FIRStruct)(*self + other);
} EndFunc()
BeginOp(FIR_Operator_Sub, 0, "Operator Sub", "The subtraction (-) operator for this struct.", 2) {
	InVal(0, RStruct<FRotator>, other, "Other", "The other rotator that should be subtracted from this rotator")
	OutVal(1, RStruct<FRotator>, result, "Result", "The resulting rotator of the vector subtraction")
	Body()
	result = (FIRStruct)(*self - other);
} EndFunc()
EndStruct()

BeginStructConstructable(FLinearColor, "Color", "Color", "A structure that holds a rgba color value")
BeginProp(RFloat, r, "Red", "The red portion of the color.", 2) {
	FIRReturn (FIRFloat) self->R;
} PropSet() {
	self->R = Val;
} EndProp()
BeginProp(RFloat, g, "Green", "The green portion of the color.", 2) {
	FIRReturn (FIRFloat) self->G;
} PropSet() {
	self->G = Val;
} EndProp()
BeginProp(RFloat, b, "Blue", "The blue portion of the color.", 2) {
	FIRReturn (FIRFloat) self->B;
} PropSet() {
	self->B = Val;
} EndProp()
BeginProp(RFloat, a, "Alpha", "The alpha (opacity) portion of the color.", 2) {
	FIRReturn (FIRFloat) self->A;
} PropSet() {
	self->A = Val;
} EndProp()
BeginOp(FIR_Operator_Add, 0, "Operator Add", "The addition (+) operator for this struct.", 2) {
	InVal(0, RStruct<FLinearColor>, other, "Other", "The other color that should be added to this color")
	OutVal(1, RStruct<FLinearColor>, result, "Result", "The resulting color of the color addition")
	Body()
	result = (FIRStruct)(*self + other);
} EndFunc()
BeginOp(FIR_Operator_Neg, 1, "Operator Neg", "The Negation operator for this struct. Does NOT make the color negative. Calculates 1 - this.", 2) {
	OutVal(0, RStruct<FLinearColor>, result, "Result", "The resulting color of the color addition")
	Body()
	result = (FIRStruct)(FLinearColor::White - *self);
} EndFunc()
BeginOp(FIR_Operator_Sub, 0, "Operator Sub", "The subtraction (-) operator for this struct.", 2) {
	InVal(0, RStruct<FLinearColor>, other, "Other", "The other color that should be subtracted from this color")
	OutVal(1, RStruct<FLinearColor>, result, "Result", "The resulting color of the color subtraction")
	Body()
	result = (FIRStruct)(*self - other);
} EndFunc()
BeginOp(FIR_Operator_Mul, 1, "Color Factor Scaling", "", 2) {
	InVal(0, RFloat, factor, "Factor", "The factor with which this color should be scaled with.")
	OutVal(1, RStruct<FVector>, result, "Result", "The resulting scaled color.")
	Body()
	result = (FIRStruct)(*self * factor);
} EndFunc()
BeginOp(FIR_Operator_Div, 1, "Color Inverse Factor Scaling", "", 2) {
	InVal(0, RFloat, factor, "Factor", "The factor with which this color should be scaled inversly with.")
	OutVal(1, RStruct<FVector>, result, "Result", "The resulting inverse scaled color.")
	Body()
	result = (FIRStruct)(*self / factor);
} EndFunc()
EndStruct()

BeginStructConstructable(FMargin, "Margin", "Margin", "A struct containing four floats that describe a margin around a box (like a 9-patch).")
BeginProp(RFloat, left, "Left", "The left edge of the rectangle.", 2) {
	FIRReturn FIRFloat(self->Left);
} PropSet() {
	self->Left = Val;
} EndProp()
BeginProp(RFloat, right, "Right", "The right edge of the rectangle.", 2) {
	FIRReturn FIRFloat(self->Right);
} PropSet() {
	self->Right = Val;
} EndProp()
BeginProp(RFloat, top, "Top", "The top edge of the rectangle.", 2) {
	FIRReturn FIRFloat(self->Top);
} PropSet() {
	self->Top = Val;
} EndProp()
BeginProp(RFloat, bottom, "Bottom", "The bottom edge of the rectangle.", 2) {
	FIRReturn FIRFloat(self->Left);
} PropSet() {
	self->Bottom = Val;
} EndProp()
EndStruct()

BeginStructConstructable(FVector4, "Vector4", "Vector4", "A Vector containing four values.")
BeginProp(RFloat, x, "X", "The first value in the Vector4.", 2) {
	FIRReturn self->X;
} PropSet() {
	self->X = Val;
} EndProp()
BeginProp(RFloat, y, "Y", "The second value in the Vector4.", 2) {
	FIRReturn self->Y;
} PropSet() {
	self->Y = Val;
} EndProp()
BeginProp(RFloat, z, "Z", "The third value in the Vector4.", 2) {
	FIRReturn self->Z;
} PropSet() {
	self->Z = Val;
} EndProp()
BeginProp(RFloat, w, "W", "The fourth value in the Vector4.", 2) {
	FIRReturn self->W;
} PropSet() {
	self->W = Val;
} EndProp()
EndStruct()
