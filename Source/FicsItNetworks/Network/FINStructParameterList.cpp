#include "FINStructParameterList.h"

FFINStructParameterList::FFINStructParameterList(const FFINDynamicStructHolder& Struct) : Struct(Struct) {}

int FFINStructParameterList::operator>>(FFINParameterReader& reader) const {
	void* Data = Struct.GetData();
	int count = 0;
	for (auto p = TFieldIterator<UProperty>(Struct.GetStruct()); p; ++p) {
		++count;
		if (UStrProperty* strp = Cast<UStrProperty>(*p)) reader << TCHAR_TO_UTF8(*strp->GetPropertyValue_InContainer(Data));
		else if (UIntProperty* intp = Cast<UIntProperty>(*p)) reader << intp->GetPropertyValue_InContainer(Data);
		else if (UInt64Property* int64p = Cast<UInt64Property>(*p)) reader << (int)intp->GetPropertyValue_InContainer(Data);
		else if (UFloatProperty* floatp = Cast<UFloatProperty>(*p)) reader << floatp->GetPropertyValue_InContainer(Data);
		else if (UBoolProperty* boolp = Cast<UBoolProperty>(*p)) reader << boolp->GetPropertyValue_InContainer(Data);
		else if (UObjectProperty* objp = Cast<UObjectProperty>(*p)) reader << objp->GetPropertyValue_InContainer(Data);
		//else if (auto vp = Cast<UArrayProperty>(dp)) reader << vp->GetPropertyValue_InContainer(data);
		// TODO: Add Array support
		else --count;
	}
	return count;
}

bool FFINStructParameterList::Serialize(FArchive& Ar) {
	Ar << Struct;
	return true;
}
