#pragma once

#include "FGColoredInstanceMeshProxy.h"
#include "FINCommandPointMesh.generated.h"

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class UFINCommandPointMesh : public UFGColoredInstanceMeshProxy {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	UTexture2D* LoadTextureFromFile(FString str);
};


USTRUCT(Blueprintable)
struct FFINCommandLabelReferences {
	GENERATED_BODY()

	FFINCommandLabelReferences(UObject* reference, UObject* textReference, int index) : Reference(reference), TextObjectReference(textReference), Index(index) {}
	FFINCommandLabelReferences() : Reference(nullptr), TextObjectReference(nullptr), Index(0)  {}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int Index;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* Reference;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* TextObjectReference;
	
};

USTRUCT(Blueprintable)
struct FFINCommandLabelData  {
	GENERATED_BODY()

	FFINCommandLabelData() : ImagePath(""), Text(""), TextColor(0,0,0,1), Vertical(false){}
	FFINCommandLabelData(FString imagePath, FString text, FLinearColor textColor, bool vertical) : ImagePath(imagePath), Text(text), TextColor(textColor), Vertical(vertical){}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString ImagePath;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool Vertical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FLinearColor TextColor;
	
};

USTRUCT(Blueprintable)
struct FFINCommandLabelStructure {
	GENERATED_BODY()
	
	FFINCommandLabelStructure(UObject* reference, int index, FString path, bool vertical, bool doUpdate, bool doUpdateText, FString text, FLinearColor color) : reference(reference), path(path), index(index), vertical(vertical), doUpdate(doUpdate), doUpdateText(doUpdateText), text(text), textColor(color) {}
	FFINCommandLabelStructure() : reference(0), path(""), index(0), vertical(false), doUpdate(false), doUpdateText(false), text(""), textColor(0,0,0, 1)  {}
 
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
