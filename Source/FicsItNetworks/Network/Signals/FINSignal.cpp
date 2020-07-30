#include "FINSignal.h"

FFINSignal::FFINSignal(FString Name) : Name(Name) {}

bool FFINSignal::Serialize(FArchive& Ar) {
	Ar << Name;
	return true;
}

FString FFINSignal::GetName() const {
	return Name;
}
