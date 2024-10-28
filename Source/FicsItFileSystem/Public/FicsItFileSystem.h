#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "File.h"
#include "Device.h"

namespace CodersFileSystem {
	FICSITFILESYSTEM_API void CopyPath(TSharedRef<Device> FromDevice, TSharedRef<Device> ToDevice, Path Path);
	FICSITFILESYSTEM_API void DeleteEntries(TSharedRef<Device> Device);
}

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItFileSystem, Warning, All)

class FFicsItFileSystemModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
