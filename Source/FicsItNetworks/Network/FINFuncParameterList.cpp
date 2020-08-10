#include "FINFuncParameterList.h"

#include "FINStructParameterList.h"

FFINFuncParameterList::FFINFuncParameterList(UFunction* Func) : Func(Func) {
	Data = (uint8*)FMemory::Malloc(Func->PropertiesSize);
	FMemory::Memzero(((uint8*)Data) + Func->ParmsSize, Func->PropertiesSize - Func->ParmsSize);
	for (UProperty* LocalProp = Func->FirstPropertyToInit; LocalProp != NULL; LocalProp = (UProperty*)LocalProp->Next) {
		LocalProp->InitializeValue_InContainer(Data);
	}
}

FFINFuncParameterList::FFINFuncParameterList(UFunction* Func, void* Data) : Func(Func), Data(Data) {}
FFINFuncParameterList::FFINFuncParameterList(const FFINFuncParameterList& Other) {
	*this = Other;
}

FFINFuncParameterList::~FFINFuncParameterList() {
	if (Data) {
		for (UProperty* P = Func->DestructorLink; P; P = P->DestructorLinkNext) {
			if (!P->IsInContainer(Func->ParmsSize)) {
				P->DestroyValue_InContainer(Data);
			}
		}
		FMemory::Free(Data);
		Data = nullptr;
	}
}

FFINFuncParameterList& FFINFuncParameterList::operator=(const FFINFuncParameterList& Other) {
	if (Data) {
		for (UProperty* P = Func->DestructorLink; P; P = P->DestructorLinkNext) {
			if (!P->IsInContainer(Func->ParmsSize)) {
				P->DestroyValue_InContainer(Data);
			}
		}
		if (Other.Data) {
			Data = FMemory::Realloc(Data, Other.Func->PropertiesSize);
		} else {
			FMemory::Free(Data);
			Data = nullptr;
		}
	} else if (Other.Data) {
		Data = FMemory::Malloc(Other.Func->PropertiesSize);
	}
	Func = Other.Func;
	if (Data) {
		FMemory::Memzero(((uint8*)Data) + Func->ParmsSize, Func->PropertiesSize - Func->ParmsSize);
		for (UProperty* LocalProp = Func->FirstPropertyToInit; LocalProp != NULL; LocalProp = (UProperty*)LocalProp->Next) {
			LocalProp->InitializeValue_InContainer(Data);
		}
		for (TFieldIterator<UProperty> It(Func); It; ++It) {
			It->CopyCompleteValue_InContainer(Data, Other.Data);
		}
	}
	
	return *this;
}

int FFINFuncParameterList::operator>>(FFINValueReader& reader) const {
	return FFINStructParameterList::WriteToReader(Func, Data, reader);
}

bool FFINFuncParameterList::Serialize(FArchive& Ar) {
	UFunction* OldFunc = Func;
	Ar << Func;
	if (Ar.IsLoading()) {
		if (Data) {
			for (UProperty* P = Func->DestructorLink; P; P = P->DestructorLinkNext) {
				if (!P->IsInContainer(Func->ParmsSize)) {
					P->DestroyValue_InContainer(Data);
				}
			}
			if (Func) {
				Data = FMemory::Realloc(Data, Func->PropertiesSize);
			} else {
				FMemory::Free(Data);
				Data = nullptr;
			}
		} else if (Func) {
			Data = FMemory::Malloc(Func->PropertiesSize);
		}
		if (Func) {
			for (UProperty* LocalProp = Func->FirstPropertyToInit; LocalProp != NULL; LocalProp = (UProperty*)LocalProp->Next) {
				LocalProp->InitializeValue_InContainer(Data);
			}
		}
	}
	if (Func) {
		for (auto p = TFieldIterator<UProperty>(Func); p; ++p) {
			if (auto vp = Cast<UStrProperty>(*p)) Ar << *vp->GetPropertyValuePtr_InContainer(Data);
			else if (auto vp = Cast<UIntProperty>(*p)) Ar << *vp->GetPropertyValuePtr_InContainer(Data);
			else if (auto vp = Cast<UInt64Property>(*p)) Ar << *vp->GetPropertyValuePtr_InContainer(Data);
			else if (auto vp = Cast<UFloatProperty>(*p)) Ar << *vp->GetPropertyValuePtr_InContainer(Data);
			else if (auto vp = Cast<UBoolProperty>(*p)) {
				bool b = vp->GetPropertyValue_InContainer(Data);
				Ar << b;
				vp->SetPropertyValue_InContainer(Data, b);
			} else if (auto vp = Cast<UObjectProperty>(*p)) Ar << *vp->GetPropertyValuePtr_InContainer(Data);
			//else if (auto vp = Cast<UArrayProperty>(dp)) reader << vp->GetPropertyValue_InContainer(data);
			// TODO: Add Array support
		}
		//Func->SerializeBin(Ar, Data);
	}
	return true;
}
