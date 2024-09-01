#pragma once

#include "CoreMinimal.h"
#include "Commandlets/GatherTextCommandletBase.h"
#include "FINGatherTextFromReflectionCommandlet.generated.h"

class UFIRBase;
class UFIRStruct;
class UFIRClass;
class UFIRProperty;
class UFIRFunction;
class UFIRSignal;

UCLASS()
class FICSITNETWORKSED_API UFINGatherTextFromReflectionCommandlet : public UGatherTextCommandletBase {
	GENERATED_BODY()
private:
	FString Namespace = TEXT("FicsItNetworks-StaticReflection");
	
public:
	UFINGatherTextFromReflectionCommandlet();
	
	//~ Begin UCommandlet Interface
	virtual int32 Main(const FString& Params) override;
	//~ End UCommandlet Interface

	//~ Begin UGatherTextCommandletBase  Interface
	virtual bool ShouldRunInPreview(const TArray<FString>& Switches, const TMap<FString, FString>& ParamVals) const override;
	//~ End UGatherTextCommandletBase  Interface

	void GatherBase(UFIRBase* Base);
	void GatherStruct(UFIRStruct* Struct);
	void GatherClass(UFIRClass* Class);
	void GatherProperty(UFIRProperty* Property);
	void GatherFunction(UFIRFunction* Function);
	void GatherSignal(UFIRSignal* Signal);
};
