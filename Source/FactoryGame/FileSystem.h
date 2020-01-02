#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NetworkComponent.h"
#include "FileSystem.generated.h"


UCLASS( BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FACTORYGAME_API UFileSystem : public UActorComponent, public INetworkComponent
{
	GENERATED_BODY()

public:	
	UFileSystem();

	UPROPERTY(BlueprintReadOnly, SaveGame)
		FGuid id;
	UPROPERTY(SaveGame)
		bool idCreated;
	UPROPERTY(EditDefaultsOnly)
		int capacity;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
