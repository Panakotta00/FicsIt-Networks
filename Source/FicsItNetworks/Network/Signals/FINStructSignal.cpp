#include "FINStructSignal.h"

FFINStructSignal::FFINStructSignal() : FFINSignal("NoSignal") {}

FFINStructSignal::FFINStructSignal(const FString& Name, const TFINDynamicStruct<FFINParameterList>& Data) : FFINSignal(Name), Data(Data) {}

bool FFINStructSignal::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);
	Ar << Data;
	return true;
}

int FFINStructSignal::operator>>(FFINValueReader& reader) const {
	return Data.Get<FFINParameterList>() >> reader;
}

UScriptStruct* FFINStructSignal::GetStruct() const {
	return StaticStruct();
}
