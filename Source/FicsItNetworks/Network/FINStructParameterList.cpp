#include "FINStructParameterList.h"

FFINStructParameterList::FFINStructParameterList(const FFINDynamicStructHolder& Struct) : Struct(Struct) {}
FFINStructParameterList::FFINStructParameterList(UScriptStruct* Struct, void* Data) : Struct(Struct, Data) {}

int FFINStructParameterList::operator>>(FFINValueReader& reader) const {
	void* Data = Struct.GetData();
	return WriteToReader(Struct.GetStruct(), Data, reader);
}

bool FFINStructParameterList::Serialize(FArchive& Ar) {
	Ar << Struct;
	return true;
}

int FFINStructParameterList::WriteToReader(UStruct* Struct, void* Data, FFINValueReader& reader) {
	int count = 0;
	for (auto p = TFieldIterator<UProperty>(Struct); p; ++p) {
		++count;
		if (UStrProperty* strp = Cast<UStrProperty>(*p)) reader << TCHAR_TO_UTF8(*strp->GetPropertyValue_InContainer(Data));
		else if (UIntProperty* intp = Cast<UIntProperty>(*p)) reader << (FINInt)intp->GetPropertyValue_InContainer(Data);
		else if (UInt64Property* int64p = Cast<UInt64Property>(*p)) reader << (FINInt)int64p->GetPropertyValue_InContainer(Data);
		else if (UFloatProperty* floatp = Cast<UFloatProperty>(*p)) reader << floatp->GetPropertyValue_InContainer(Data);
		else if (UBoolProperty* boolp = Cast<UBoolProperty>(*p)) reader << boolp->GetPropertyValue_InContainer(Data);
		else if (UObjectProperty* objp = Cast<UObjectProperty>(*p)) reader << objp->GetPropertyValue_InContainer(Data);
		//else if (auto vp = Cast<UArrayProperty>(dp)) reader << vp->GetPropertyValue_InContainer(data);
		// TODO: Add Array support
		else --count;
	}
	return count;
}
