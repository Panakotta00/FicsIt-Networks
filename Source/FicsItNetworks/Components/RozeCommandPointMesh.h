#pragma once

#include "FGColoredInstanceMeshProxy.h"
#include "Engine.h"

#include "RozeCommandPointMesh.generated.h"


UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class URozeCommandPointMesh : public UFGColoredInstanceMeshProxy
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	UTexture2D* LoadTextureFromFile(FString str);
};


USTRUCT(Blueprintable)
struct FRozeCommandLabelStructure
{
	GENERATED_BODY()

	
	FRozeCommandLabelStructure(UObject* reference, int index, FString path, bool vertical, bool doUpdate, bool doUpdateText, FString text, FLinearColor color) : reference(reference), path(path), index(index), vertical(vertical), doUpdate(doUpdate), doUpdateText(doUpdateText), text(text), textColor(color) {}
	FRozeCommandLabelStructure() : reference(0), path(""), index(0), vertical(false), doUpdate(false), doUpdateText(false), text(""), textColor(0,0,0, 1)  {}
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* reference;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* textObjectReference;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString path;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int index;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool vertical;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool doUpdate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool doUpdateText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FLinearColor textColor;
	
};
