#pragma once

#include "FINSignal.h"
#include "Network/FINStructParameterList.h"
#include "FINStructSignal.generated.h"

/**
 * A signal container using a network parameter list
 * to store the signals parameters.
 */
USTRUCT(BlueprintType)
struct FFINStructSignal : public FFINSignal {
	GENERATED_BODY()

private:
	UPROPERTY()
	FFINDynamicStructHolder Data;
	
public:
	FFINStructSignal();
	FFINStructSignal(const FString& Name, const TFINDynamicStruct<FFINParameterList>& Data);
	
	bool Serialize(FArchive& Ar);
	
	// Begin FFINSignal
	virtual int operator>>(FFINValueReader& reader) const override;
	// End FFINSignal
};

inline bool operator<<(FArchive& Ar, FFINStructSignal& Sig) {
	return Sig.Serialize(Ar);
}

template<>
struct TStructOpsTypeTraits<FFINStructSignal> : TStructOpsTypeTraitsBase2<FFINStructSignal>
{
	enum
	{
		WithSerializer = true,
    };
};
