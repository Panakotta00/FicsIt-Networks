#pragma once

#include "Dom/JsonObject.h"
#include "FicsItNetworks/Reflection/FINReflection.h"

TSharedPtr<FJsonObject> FINGenDataType(UFINProperty* Property, TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>());
void FINGenRefBase(TSharedPtr<FJsonObject> Obj, UFINBase* Base);
void FINGenRefProp(TSharedPtr<FJsonObject> Obj, UFINProperty* Prop);
void FINGenRefFunc(TSharedPtr<FJsonObject> Obj, UFINFunction* Func);
void FINGenRefStruct(TSharedPtr<FJsonObject> Obj, UFINStruct* Struct);
void FINGenRefSignal(TSharedPtr<FJsonObject> Obj, UFINSignal* Signal);
void FINGenRefClass(TSharedPtr<FJsonObject> Obj, UFINClass* Class);
bool FINGenReflectionDoc(UWorld* World, const TCHAR* Command, FOutputDevice& Ar);