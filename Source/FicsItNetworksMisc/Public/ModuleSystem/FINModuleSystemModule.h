#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FINModuleSystemModule.generated.h"

class UFINModuleSystemPanel;

UINTERFACE()
class FICSITNETWORKSMISC_API UFINModuleSystemModule : public UInterface {
	GENERATED_BODY()
};

class FICSITNETWORKSMISC_API IFINModuleSystemModule {
	GENERATED_BODY()

public:
	/**
	 * Returns the size of the module.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ModuleSystem|Module")
		void getModuleSize(int& width, int& height) const;

	/**
	 * Adds itself to the given panel at the given postion and rotation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ModuleSystem|Module")
		void setPanel(UFINModuleSystemPanel* panel, int x, int y, int rot);

	/**
	 * Returns the name of the module.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ModuleSystem|Module")
		FName getName() const;
};
