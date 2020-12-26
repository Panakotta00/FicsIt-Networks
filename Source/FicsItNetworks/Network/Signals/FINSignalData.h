#pragma once

#include "Network/FINAnyNetworkValue.h"
#include "FINSignalData.generated.h"

class UFINSignal;

USTRUCT()
struct FFINSignalData {
	GENERATED_BODY()

	UPROPERTY()
	UFINSignal* Signal = nullptr;

	UPROPERTY()
	TArray<FFINAnyNetworkValue> Data;

	FFINSignalData() = default;
	FFINSignalData(UFINSignal* Signal, const FINArray& Data) : Signal(Signal), Data(Data) {}

	bool Serialize(FArchive& Ar);
};

inline FArchive& operator<<(FArchive& Ar, FFINSignalData& Signal) {
	Signal.Serialize(Ar);
	return Ar;
}

template<>
struct TStructOpsTypeTraits<FFINSignalData> : TStructOpsTypeTraitsBase2<FFINSignalData> {
	enum {
		WithSerializer = true,
	};
};