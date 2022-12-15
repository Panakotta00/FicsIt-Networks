#pragma once

#include "Reflection/FINReflection.h"

void FINGenLuaClass(FString& Documentation, FFINReflection& Ref, UFINClass* Class);
bool FINGenLuaDoc(UWorld* World, const TCHAR* Command, FOutputDevice& Ar);