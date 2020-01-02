#pragma once

#include <assets/BPInterface.h>
#include "../SatisfactorySDK/SDK.hpp"

class UModuleSystemPanel;

class IModuleSystemModule {
public:
	virtual void getModuleSize(int& width, int& height) const;
	virtual void setPanel(UModuleSystemPanel* panel, int x, int y, int rot);
	virtual SML::Objects::FName getName() const;
};

class UModuleSystemModule : public SDK::UInterface {
public:
	static void execGetModuleSize(IModuleSystemModule* self, SML::Objects::FFrame& stack, void* ret);
	static void execSetPanel(IModuleSystemModule* self, SML::Objects::FFrame& stack, void* ret);
	static void execGetName(IModuleSystemModule* self, SML::Objects::FFrame& stack, void* ret);

	static SML::Objects::UClass* staticClass();
};