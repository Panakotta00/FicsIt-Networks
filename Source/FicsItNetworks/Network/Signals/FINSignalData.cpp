#include "FINSignalData.h"

#include "Reflection/FINSignal.h"

bool FFINSignalData::Serialize(FArchive& Ar) {
	Ar << Signal;
	Ar << Data;
	return true;
}