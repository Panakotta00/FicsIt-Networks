#pragma once

#include "CoreMinimal.h"
#include "FGLegacyItemStateActorInterface.h"
#include "FGSaveInterface.h"
#include "FINItemStateFileSystem.h"
#include "GameFramework/Actor.h"
#include "FicsItFileSystem/Device.h"
#include "FicsItFileSystem/Path.h"
#include "FINFileSystemState_Legacy.generated.h"

UCLASS()
class FICSITNETWORKSCOMPUTER_API AFINFileSystemState_Legacy : public AActor, public IFGSaveInterface, public IFGLegacyItemStateActorInterface {
	GENERATED_BODY()

private:
	TSharedPtr<CodersFileSystem::Device> Device;

	bool bUseOldSerialization = false;
	bool bUsePreBinarySupportSerialization = true;
	
public:
	void SerializePath(TSharedRef<CodersFileSystem::Device> SerializeDevice, FStructuredArchive::FRecord Record, CodersFileSystem::Path Path, FString Name, int& KeepDisk);

	UPROPERTY(SaveGame)
	FGuid ID;

	UPROPERTY(SaveGame)
	bool IdCreated = false;

	UPROPERTY(SaveGame, EditDefaultsOnly)
	int32 Capacity = 0;

	UPROPERTY(SaveGame)
	FString Label;
	
	AFINFileSystemState_Legacy();

	// Begin UObject
	virtual void Serialize(FStructuredArchive::FRecord Record) override;
	// End UObject

	// Begin IFGSaveInterface
	virtual void PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	virtual FFGDynamicStruct ConvertToItemState( TSubclassOf<UFGItemDescriptor> itemDescriptor ) const {
		FFINItemStateFileSystem state;

		state.ID = ID;
		state.Capacity = Capacity;
		state.Label = Label;

		return FFGDynamicStruct(state);
	}
	
	TSharedPtr<CodersFileSystem::Device> GetDevice(bool bInForceUpdate = false, bool bInForeCreate = false);
};