#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTracy, Verbose, All);

class FTracyModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	virtual bool IsGameModule() const override { return false; }

private:
	FDelegateHandle OnBeginFrameHandle;
};