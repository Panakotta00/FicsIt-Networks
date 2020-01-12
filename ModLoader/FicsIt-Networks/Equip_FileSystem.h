#pragma once

#include <assets/BPInterface.h>

#include "FileSystem.h"

#include "../SatisfactorySDK/SDK.hpp"

class AEquip_FileSystem : public SDK::AFGEquipment {
public:
	void construct();
	void destruct();

	UFileSystem* filesystem;

	bool shouldSaveState() const;

	void moveSelfToItem(SML::Objects::FFrame& stack, void* item);
};