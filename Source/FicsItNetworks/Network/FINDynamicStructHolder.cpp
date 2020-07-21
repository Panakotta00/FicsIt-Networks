#include "FINDynamicStructHolder.h"

FFINDynamicStructHolder::FFINDynamicStructHolder() {}

FFINDynamicStructHolder::FFINDynamicStructHolder(UStruct* Struct) : Struct(Struct) {
	Data = FMemory::Malloc(Struct->GetStructureSize());
	Struct->InitializeStruct(Data);
}

FFINDynamicStructHolder::FFINDynamicStructHolder(UStruct* Struct, void* Data) : Data(Data), Struct(Struct) {}

FFINDynamicStructHolder::FFINDynamicStructHolder(const FFINDynamicStructHolder& Other) {
	*this = Other;
}

FFINDynamicStructHolder::~FFINDynamicStructHolder() {
	if (Data) {
		Struct->DestroyStruct(Data);
		FMemory::Free(Data);
		Data = nullptr;
	}
}

FFINDynamicStructHolder& FFINDynamicStructHolder::operator=(const FFINDynamicStructHolder& Other) {
	if (Data) {
		Struct->DestroyStruct(Data);
		if (Other.Data) {
			Data = FMemory::Realloc(Data, Other.Struct->GetStructureSize());
		}
	} else {
		if (Other.Data) {
			Data = FMemory::Malloc(Other.Struct->GetStructureSize());
		}
	}
	Struct = Other.Struct;
	if (Other.Data) {
		Struct->InitializeStruct(Data);
		for (TFieldIterator<UProperty> Prop(Struct); Prop; ++Prop) {
			Prop->CopyCompleteValue_InContainer(Data, Other.Data);
		}
	} else {
		FMemory::Free(Data);
		Data = nullptr;
	}
	
	return *this;
}

bool FFINDynamicStructHolder::Serialize(FArchive& Ar) {
	if (Ar.IsLoading() && Data && Struct) {
		Struct->DestroyStruct(Data);
		FMemory::Free(Data);
		Data = nullptr;
	}
	Ar << Struct;
	if (Ar.IsLoading() && Struct) {
		Data = FMemory::Malloc(Struct->GetStructureSize());
		Struct->InitializeStruct(Data);
	}
	if (Data && Struct) Struct->SerializeBin(Ar, Data);
	return true;
}

UStruct* FFINDynamicStructHolder::GetStruct() const {
	return Struct;
}

void* FFINDynamicStructHolder::GetData() const {
	return Data;
}
