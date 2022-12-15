#pragma once

#include "Hologram/FGBuildableHologram.h"
#include "FINDefaultExtendedHolo.generated.h"

UCLASS()
class UBuildMode_Snapped45 : public UFGHologramBuildModeDescriptor {
	GENERATED_BODY()
	
	public:
	UBuildMode_Snapped45();
};

UCLASS()
class UBuildMode_Default : public UFGHologramBuildModeDescriptor {
	GENERATED_BODY()
	
	public:
	UBuildMode_Default();
};

UCLASS()
class UBuildMode_FreePlacement : public UFGHologramBuildModeDescriptor {
	GENERATED_BODY()
	
	public:
	UBuildMode_FreePlacement();
};

UCLASS()
class AFINDefaultExtendedHolo : public AFGBuildableHologram{
	GENERATED_BODY()
	
	public:

	AFINDefaultExtendedHolo();
	
	virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;
	
	UPROPERTY( EditDefaultsOnly, Category = "Hologram|BuildMode" )
	TSubclassOf< class UFGHologramBuildModeDescriptor > mBuildModeAuto;
	
	UPROPERTY( EditDefaultsOnly, Category = "Hologram|BuildMode" )
	TSubclassOf< class UFGHologramBuildModeDescriptor > mBuildModeFree;
	
	UPROPERTY( EditDefaultsOnly, Category = "Hologram|BuildMode" )
	TSubclassOf< class UFGHologramBuildModeDescriptor > mBuildMode45;
	
	virtual void GetSupportedBuildModes_Implementation( TArray< TSubclassOf<UFGHologramBuildModeDescriptor> >& out_buildmodes ) const override;
};
