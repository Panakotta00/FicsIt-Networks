#include "FINClass.h"

UFINSignal* UFINClass::FindFINSignal(const FString& Name) {
	for (UFINSignal* Signal : GetSignals()) {
		if (Signal->GetInternalName() == Name) return Signal;
	}
	return nullptr;
}
