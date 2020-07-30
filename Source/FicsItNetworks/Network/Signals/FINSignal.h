#pragma once

#include "CoreMinimal.h"
#include "Network/FINValueReader.h"
#include "FINSignal.generated.h"

/**
 * Base class of all signals transferred through the network.
 * Containing the name of the signal.
 * Base classes will then implement support for signal parameters.
 */
USTRUCT(BlueprintType)
struct FICSITNETWORKS_API FFINSignal {
	GENERATED_BODY()
	
private:
	UPROPERTY()
	FString Name;

public:
	FFINSignal() : Name("NoSignal") {}
	FFINSignal(FString Name);
	virtual ~FFINSignal() {}

	bool Serialize(FArchive& Ar);
	
	/**
	 * Writes all additional signal data (excluding the Name) to the given
	 * parameter reader.
	 *
	 * @param[in]	reader	the reader reading teh data.
	 * @return	returns the count of written values
	 */
	virtual int operator>>(FFINValueReader& reader) const { return 0; };

	FString GetName() const;
};

template<>
struct TStructOpsTypeTraits<FFINSignal> : TStructOpsTypeTraitsBase2<FFINSignal>
{
	enum
	{
        WithSerializer = true,
    };
};

inline bool operator<<(FArchive& Ar, FFINSignal& Signal) {
	return Signal.Serialize(Ar);
}

struct FFINNoSignal : public FFINSignal {
public:
	FFINNoSignal() : FFINSignal("None") {}

	virtual int operator>>(FFINValueReader& reader) const override { return 0; }
};
