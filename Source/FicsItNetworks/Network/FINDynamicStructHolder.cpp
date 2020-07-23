#include "FINDynamicStructHolder.h"

FFINDynamicStructHolder::FFINDynamicStructHolder() {}

FFINDynamicStructHolder::FFINDynamicStructHolder(UScriptStruct* Struct) : Struct(Struct) {
	Data = FMemory::Malloc(Struct->GetStructureSize());
	Struct->InitializeStruct(Data);
}

FFINDynamicStructHolder::FFINDynamicStructHolder(UScriptStruct* Struct, void* Data) : Data(Data), Struct(Struct) {}

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
		} else {
			FMemory::Free(Data);
			Data = nullptr;
		}
	} else {
		if (Other.Data) {
			Data = FMemory::Malloc(Other.Struct->GetStructureSize());
		}
	}
	Struct = Other.Struct;
	if (Data) {
		Struct->InitializeStruct(Data);
		Struct->CopyScriptStruct(Data, Other.Data);
	}
	
	return *this;
}

FFINDynamicStructHolder FFINDynamicStructHolder::Copy(UScriptStruct* Struct, void* Data) {
	FFINDynamicStructHolder holder(Struct);
	Struct->CopyScriptStruct(holder.Data, Data);
	return holder;
}

bool FFINDynamicStructHolder::Serialize(FArchive& Ar) {
	UScriptStruct* OldStruct = Struct;
	Ar << Struct;
	if (Ar.IsLoading()) {
		if (Data) {
			OldStruct->DestroyStruct(Data);
			if (Struct) {
				Data = FMemory::Realloc(Data, Struct->GetStructureSize());
			} else {
				FMemory::Free(Data);
				Data = nullptr;
			}
		} else if (Struct) {
			Data = FMemory::Malloc(Struct->GetStructureSize());
		}
		if (Struct) Struct->InitializeStruct(Data);
	}
	if (Struct) {
		Struct->GetCppStructOps()->Serialize(Ar, Data);
	}
	return true;
}

UScriptStruct* FFINDynamicStructHolder::GetStruct() const {
	return Struct;
}

void* FFINDynamicStructHolder::GetData() const {
	return Data;
}
