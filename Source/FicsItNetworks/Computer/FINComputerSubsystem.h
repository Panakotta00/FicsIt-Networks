#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "FGSubsystem.h"
#include "Queue.h"
#include "Engine/Engine.h"
#include "Network/FINNetworkTrace.h"

#include "FINComputerSubsystem.generated.h"

UCLASS()
class AFINComputerSubsystem : public AFGSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
	
public:
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	UFUNCTION(BlueprintCallable, Category = "Computer", meta = (WorldContext = "WorldContext"))
	static AFINComputerSubsystem* GetComputerSubsystem(UObject* WorldContext);
};
