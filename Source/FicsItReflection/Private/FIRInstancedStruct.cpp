#include "FIRInstancedStruct.h"

#include "Engine/World.h"

FFIRInstancedStruct::FFIRInstancedStruct() {}

FFIRInstancedStruct::FFIRInstancedStruct(UScriptStruct* Struct) : Struct(Struct) {
	Data = FMemory::Malloc(Struct->GetStructureSize());
	Struct->InitializeStruct(Data);
}

FFIRInstancedStruct::FFIRInstancedStruct(UScriptStruct* Struct, void* Data) : Data(Data), Struct(Struct) {}

FFIRInstancedStruct::FFIRInstancedStruct(const FFIRInstancedStruct& Other) {
	*this = Other;
}

FFIRInstancedStruct::~FFIRInstancedStruct() {
	if (Data) {
		Struct->DestroyStruct(Data);
		FMemory::Free(Data);
		Data = nullptr;
	}
}

FFIRInstancedStruct& FFIRInstancedStruct::operator=(const FFIRInstancedStruct& Other) {
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

FFIRInstancedStruct FFIRInstancedStruct::Copy(UScriptStruct* Struct, const void* Data) {
	FFIRInstancedStruct holder(Struct);
	if (Data) Struct->CopyScriptStruct(holder.Data, Data);
	return holder;
}

bool FFIRInstancedStruct::Serialize(FStructuredArchive::FSlot Slot) {
	UScriptStruct* OldStruct = Struct;
	
	FStructuredArchive::FRecord Record = Slot.EnterRecord();
	Record.EnterField(SA_FIELD_NAME(TEXT("Type"))) << Struct;

	if (Slot.GetUnderlyingArchive().IsLoading()) {
		if (Data) {
			if (OldStruct) OldStruct->DestroyStruct(Data);
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
		auto field = Record.EnterField(SA_FIELD_NAME(TEXT("End")));
		Struct->SerializeItem(field, Data, nullptr);
	}
	return true;
}

/*bool FFIRInstancedStruct::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) {
	bOutSuccess = Serialize(Ar);
	return bOutSuccess;
}*/

void FFIRInstancedStruct::AddStructReferencedObjects(FReferenceCollector& Collector) const {
	UScriptStruct* ThisStruct = Struct;
	if (Struct) Collector.AddReferencedObject(ThisStruct);
	if (Struct && Data) {
		if (Struct->GetCppStructOps()->HasAddStructReferencedObjects()) Struct->GetCppStructOps()->AddStructReferencedObjects()(Data, Collector);
	}
}

UScriptStruct* FFIRInstancedStruct::GetStruct() const {
	return Struct;
}

void* FFIRInstancedStruct::GetData() const {
	return Data;
}
