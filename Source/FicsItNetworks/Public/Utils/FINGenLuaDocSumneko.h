#pragma once

#include "Reflection/FINReflection.h"

void FINGenLuaClassSumneko(FString& Documentation, FFINReflection& Ref, const UFINClass* Class);
bool FINGenLuaDocSumneko(UWorld* World, const TCHAR* Command, FOutputDevice& Ar);