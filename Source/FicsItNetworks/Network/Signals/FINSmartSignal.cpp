#include "FINSmartSignal.h"

UScriptStruct* FFINSmartSignal::GetStruct() const {
	return StaticStruct();
}

FFINSmartSignal::FFINSmartSignal() {}

bool FFINSmartSignal::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);

	Ar << Args;
	
	return true;
}

int FFINSmartSignal::operator>>(FFINValueReader& reader) const {
	for (const FFINAnyNetworkValue& Val : Args) {
		Val >> reader;
	}
	return Args.Num();
}
