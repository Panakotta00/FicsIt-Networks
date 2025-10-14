#pragma once

#include "FINDynamicModuleSystemHolo.h"
#include "FINModuleBase.h"

#include "FINDefaultDynamicSizedModule.generated.h"


UENUM(BlueprintType)
enum EFINDynamicMeshSpawnMode{
	EFINDynamicMeshSpawnMode_None,
	EFINDynamicMeshSpawnMode_Tile,
	EFINDynamicMeshSpawnMode_Stretch,
};

UCLASS()
class FICSITNETWORKS_API AFINDefaultDynamicSizedModule : public AFINModuleBase, public IFINDynamicModuleBuildable {
	GENERATED_BODY()

	public:

		UPROPERTY(SaveGame, Replicated, BlueprintReadOnly)
		int DynamicWidth = 1;
		UPROPERTY(SaveGame, Replicated, BlueprintReadOnly)
		int DynamicHeight = 1;
		
		UPROPERTY(EditDefaultsOnly, Category="Meshes")
		UStaticMesh* MeshA;
		
		UPROPERTY(EditDefaultsOnly, Category="Meshes")
		UStaticMesh* MeshB;
		
		UPROPERTY(EditDefaultsOnly, Category="Meshes")
		UStaticMesh* MeshC;
		
		UPROPERTY(EditDefaultsOnly, Category="Meshes")
		UStaticMesh* MeshD;
		
		UPROPERTY(EditDefaultsOnly, Category="Meshes")
		UStaticMesh* MeshE;
		
		UPROPERTY(EditDefaultsOnly, Category="Meshes")
		UStaticMesh* MeshF;
		
		UPROPERTY(EditDefaultsOnly, Category="Meshes")
		UStaticMesh* MeshG;
		
		UPROPERTY(EditDefaultsOnly, Category="Meshes")
		UStaticMesh* MeshH;
		
		UPROPERTY(EditDefaultsOnly, Category="Meshes")
		UStaticMesh* MeshI;
	
		UPROPERTY()
		TArray<USceneComponent*> Parts;

		UPROPERTY(EditDefaultsOnly)
		int MinWidth;
		
		UPROPERTY(EditDefaultsOnly)
		int MinHeight;
		
		UPROPERTY(EditDefaultsOnly)
		int MaxWidth = 100;
		
		UPROPERTY(EditDefaultsOnly)
		int MaxHeight = 100;

		UPROPERTY(EditDefaultsOnly, Category="Mesh Spawn Settings")
		int EdgeSpawnMinimumWidth;
		
		UPROPERTY(EditDefaultsOnly, Category="Mesh Spawn Settings")
		int EdgeSpawnMinimumHeight;

		UPROPERTY(EditDefaultsOnly, Category="Mesh Spawn Settings")
		int CenterSpawnMinimumWidth;
		
		UPROPERTY(EditDefaultsOnly, Category="Mesh Spawn Settings")
		int CenterSpawnMinimumHeight;

		UPROPERTY(EditDefaultsOnly, Category="Mesh Spawn Settings")
		int CenterSpawnSizeModifier;
		
		UPROPERTY(EditDefaultsOnly, Category="Mesh Spawn Settings")
		FVector MeshOffset;
		
		UPROPERTY(EditDefaultsOnly, Category="Mesh Spawn Settings")
		FRotator MeshRotation;

		UPROPERTY(EditDefaultsOnly, Category="Mesh Spawn Settings")
		bool bRotateMeshes = true;

		UPROPERTY(EditDefaultsOnly, Category="Mesh Spawn Settings - Size")
		double CenterMeshSizeMultiplierX = 1;
		
		UPROPERTY(EditDefaultsOnly, Category="Mesh Spawn Settings - Size")
		double CenterMeshSizeMultiplierY = 1;

		UPROPERTY(EditDefaultsOnly, Category="Mesh Spawn Settings")
		TEnumAsByte<EFINDynamicMeshSpawnMode> EdgeTilingMode = EFINDynamicMeshSpawnMode_Stretch;
		
		UPROPERTY(EditDefaultsOnly, Category="Mesh Spawn Settings")
		TEnumAsByte<EFINDynamicMeshSpawnMode> CenterTilingMode = EFINDynamicMeshSpawnMode_Stretch;
		
		AFINDefaultDynamicSizedModule();

		// Begin AActor
		virtual void BeginPlay() override;
		virtual void OnConstruction(const FTransform& transform) override;
		virtual void Tick(float DeltaSeconds) override;
		virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
		// End AActor

		// Begin AFGBuildable
		virtual int32 GetDismantleRefundReturnsMultiplier() const override;
		// End AFGBuildable

		// Begin IFGSaveInterface
		virtual bool ShouldSave_Implementation() const override;
		// End IFGSaveInterface

		void ConstructParts();
		virtual void SpawnComponents_Implementation(int Width, int Height, AActor* Parent, USceneComponent* Attach,
													TArray<USceneComponent*>& OutParts) override;

		UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SpawnAdditionalComponents(int Width, int Height, AActor* Parent, USceneComponent* Attach, UPARAM(Ref) TArray<USceneComponent*>& OutParts);
		
		virtual void SetDynamicSize_Implementation(int Width, int Height) override;
		
		static void SpawnEdgeComponent(TSubclassOf<UStaticMeshComponent> Class, int x, int y, int r, UStaticMesh* EdgePartMesh, AActor* Parent, USceneComponent* Attach, int Width, int Height, int SizeModifier, bool Rotate, double MeshScale, EFINDynamicMeshSpawnMode TileMode, TArray<USceneComponent*>& OutParts); 
		static void SpawnCornerComponent(TSubclassOf<UStaticMeshComponent> Class, int x, int y, int r, UStaticMesh* CornerPartMesh, AActor* Parent, USceneComponent* Attach, int Width, int Height, bool Rotate, TArray<USceneComponent*>& OutParts);

		UFUNCTION(Blueprintable, BlueprintNativeEvent)
		void SetPanelSize(int width, int height);
		
		virtual void GetMaximumDynamicSize_Implementation(int &Width, int &Height) override;
		virtual void GetMinimumDynamicSize_Implementation(int &Width, int &Height) override;

		UFUNCTION(BlueprintCallable)
		UStaticMeshComponent* CreateMeshComponent(TSubclassOf<UStaticMeshComponent> Class, UStaticMesh* Mesh, FVector Location, FRotator Rotation, FVector Scale,  AActor* Parent, USceneComponent* Attach);
		virtual void getModuleSize_Implementation(int& Width, int& Height) const override;
};

