#include "FINVariadicParameterList.h"

FFINVariadicParameterList::FFINVariadicParameterList() {}

bool FFINVariadicParameterList::Serialize(FArchive& Ar) {
	Ar << Args;
	return true;
}

int FFINVariadicParameterList::operator>>(FFINValueReader& reader) const {
	for (const FFINAnyNetworkValue& Val : Args) {
		Val >> reader;
	}
	return Args.Num();
}

FFINVariadicParameterList& FFINVariadicParameterList::Add(const FFINAnyNetworkValue& Val) {
	Args.Add(Val);
	return *this;
}

const FFINAnyNetworkValue& FFINVariadicParameterList::Get(int Index) const {
	return Args[Index];
}

int FFINVariadicParameterList::Num() const {
	return Args.Num();
}
