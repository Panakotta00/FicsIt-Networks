#pragma once

#include "CoreMinimal.h"
#include "Commandlets/GatherTextCommandletBase.h"
#include "FINGatherTextFromReflectionCommandlet.generated.h"

class UFINBase;
class UFINStruct;
class UFINClass;
class UFINProperty;
class UFINFunction;
class UFINSignal;

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

	void GatherBase(UFINBase* Base);
	void GatherStruct(UFINStruct* Struct);
	void GatherClass(UFINClass* Class);
	void GatherProperty(UFINProperty* Property);
	void GatherFunction(UFINFunction* Function);
	void GatherSignal(UFINSignal* Signal);
};
