#pragma once

#include <assets/BPInterface.h>

#include "FileSystem.h"

#include "../SatisfactorySDK/SDK.hpp"

struct IEquipFileSystemSaveInterface {
	virtual bool NeedTransform();
	virtual bool ShouldSave();
	virtual void gatherDeps(SML::Objects::TArray<SML::Objects::UObject*>*);
	virtual void postLoad(int, int);
	virtual void preLoad(int, int);
	virtual void postSave(int, int);
	virtual void preSave(int, int);
	virtual SML::Objects::UObject* _getUObject() const;
};

class AEquip_FileSystem : public SDK::AFGEquipment {
public:
	void construct();
	void destruct();

	UFileSystem* filesystem;
	SDK::FInventoryStack stack;

	IEquipFileSystemSaveInterface save;

	bool shouldSaveState() const;

	void createState(SML::Objects::FFrame& stack, void* item);

	static SML::Objects::UClass* staticClass();
};