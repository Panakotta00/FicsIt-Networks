#pragma once

#include <assets/BPInterface.h>
#include <util/Objects/FVector.h>
#include <util/Objects/Delegate.h>

#include "../SatisfactorySDK/SDK.hpp"

#include "ModuleSystemModule.h"

class UModuleSystemPanel : public SDK::USceneComponent {
public:
	int panelWidth;
	int panelHeight;
	SML::Objects::TArray<SML::Objects::UClass*> allowedModules;
	SDK::AActor*** grid;
	SML::Objects::FMulticastScriptDelegate onModuleChanged;
	std::uint64_t something;

	void construct();
	void destruct();

	void postLoad();
	void beginPlay();
	void endPlay(SDK::EEndPlayReason reason);

	SDK::AActor* getModule(int x, int y);

	void execAddModule(SML::Objects::FFrame& stack, void* ret);
	void execRemoveModule(SML::Objects::FFrame& stack, void* ret);
	void execGetModule(SML::Objects::FFrame& stack, void* ret);
	void execGetModules(SML::Objects::FFrame& stack, void* ret);
	void execGetDismantleRefund(SML::Objects::FFrame& stack, void* ret);

	static void getModuleSpace(SML::Objects::FVector loc, int rot, SML::Objects::FVector msize, SML::Objects::FVector& min, SML::Objects::FVector& max);
	static SML::Objects::UClass* staticClass();
};