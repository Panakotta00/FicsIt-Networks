#include "FINFuncParameterList.h"

#include "FINStructParameterList.h"

FFINFuncParameterList::FFINFuncParameterList(UFunction* Func) : Func(Func) {
	Data = (uint8*)FMemory::Malloc(Func->PropertiesSize);
	FMemory::Memzero(((uint8*)Data) + Func->ParmsSize, Func->PropertiesSize - Func->ParmsSize);
	Func->InitializeStruct(Data);
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
		Func->InitializeStruct(Data);
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
	int count = 0;
	for (auto p = TFieldIterator<UProperty>(Func); p; ++p) {
		if (p->PropertyFlags & EPropertyFlags::CPF_Parm) {
			++count;
			if (UStrProperty* strp = Cast<UStrProperty>(*p)) reader << TCHAR_TO_UTF8(*strp->GetPropertyValue_InContainer(Data));
			else if (UIntProperty* intp = Cast<UIntProperty>(*p)) reader << (FINInt)intp->GetPropertyValue_InContainer(Data);
			else if (UInt64Property* int64p = Cast<UInt64Property>(*p)) reader << (FINInt)int64p->GetPropertyValue_InContainer(Data);
			else if (UFloatProperty* floatp = Cast<UFloatProperty>(*p)) reader << floatp->GetPropertyValue_InContainer(Data);
			else if (UBoolProperty* boolp = Cast<UBoolProperty>(*p)) reader << boolp->GetPropertyValue_InContainer(Data);
			else if (UObjectProperty* objp = Cast<UObjectProperty>(*p)) reader << objp->GetObjectPropertyValue_InContainer(Data);
			//else if (auto vp = Cast<UArrayProperty>(dp)) reader << vp->GetPropertyValue_InContainer(data);
			// TODO: Add Array support
			else --count;
		}
	}
	return count;
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
			Func->InitializeStruct(Data);
			for (UProperty* LocalProp = Func->FirstPropertyToInit; LocalProp != NULL; LocalProp = (UProperty*)LocalProp->Next) {
				LocalProp->InitializeValue_InContainer(Data);
			}
		}
	}
	if (Func) {
		for (auto p = TFieldIterator<UProperty>(Func); p; ++p) {
			if (auto strp = Cast<UStrProperty>(*p)) Ar << *strp->GetPropertyValuePtr_InContainer(Data);
			else if (auto intp = Cast<UIntProperty>(*p)) Ar << *intp->GetPropertyValuePtr_InContainer(Data);
			else if (auto int64p = Cast<UInt64Property>(*p)) Ar << *int64p->GetPropertyValuePtr_InContainer(Data);
			else if (auto fp = Cast<UFloatProperty>(*p)) Ar << *fp->GetPropertyValuePtr_InContainer(Data);
			else if (auto bp = Cast<UBoolProperty>(*p)) {
				bool b = bp->GetPropertyValue_InContainer(Data);
				Ar << b;
				bp->SetPropertyValue_InContainer(Data, b);
			} else if (auto op = Cast<UObjectProperty>(*p)) Ar << *op->GetPropertyValuePtr_InContainer(Data);
			//else if (auto vp = Cast<UArrayProperty>(dp)) reader << vp->GetPropertyValue_InContainer(data);
			// TODO: Add Array support
		}
		//Func->SerializeBin(Ar, Data);
	}
	return true;
}
