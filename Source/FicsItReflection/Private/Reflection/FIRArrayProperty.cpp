#include "Reflection/FIRArrayProperty.h"

FIRAny UFIRArrayProperty::GetValue(const FFIRExecutionContext& Ctx) const {
	void* Ptr = Ctx.GetGeneric();
    if (Property) {
    	Ptr = Property->ContainerPtrToValuePtr<void>(Ptr);
    }
	return GetArray(Ptr);
}

void UFIRArrayProperty::SetValue(const FFIRExecutionContext& Ctx, const FIRAny& Value) const {
	if (Value.GetType() != FIR_ARRAY) return;
	void* Ptr = Ctx.GetGeneric();
	if (Property) {
		Ptr = Property->ContainerPtrToValuePtr<void>(Ptr);
	}
	SetArray(Ptr, Value.GetArray());
}

FIRArray UFIRArrayProperty::GetArray(void* Ctx) const {
	if (Property) {
		FIRArray Arr;
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		for (int i = 0; i < SArr.Num(); ++i) {
			Arr.Add(InnerType->GetValue(SArr.GetRawPtr(i)));
		}
		return Arr;
	}
	return Super::GetValue(Ctx).GetArray();
}

void UFIRArrayProperty::SetArray(void* Ctx, const FIRArray& Array) const {
	if (Property) {
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		SArr.EmptyValues();
		for (int i = 0; i < Array.Num(); ++i) {
			if (!InnerType->IsValidValue(Array[i])) continue;
			int j = SArr.AddValue();
			InnerType->SetValue(SArr.GetRawPtr(j), Array[i]);
		}
	} else Super::SetValue(Ctx, Array);
}

void UFIRArrayProperty::AddValueToArray(void* Ctx, const FIRAny& Value) const {
	if (Property) {
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		SArr.AddValue();
		InnerType->SetValue(SArr.GetRawPtr(SArr.Num()), Value);
	} else {
		FIRArray Array = GetArray(Ctx);
		Array.Add(Value);
		SetArray(Ctx, Array);
	}
}

FIRAny UFIRArrayProperty::GetValueInArray(void* Ctx, int Index) const {
	if (Property) {
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		if (SArr.Num() <= Index) return FIRAny();
		return InnerType->GetValue(SArr.GetRawPtr(Index));
	} else {
		FIRArray Array = GetArray(Ctx);
		if (Array.Num() <= Index) return FIRAny();
		return Array[Index];
	}
}

void UFIRArrayProperty::SetValueInArray(void* Ctx, int Index, const FIRAny& Value) const {
	if (Property) {
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		if (SArr.Num() <= Index) return;
		InnerType->SetValue(SArr.GetRawPtr(Index), Value);
	} else {
		FIRArray Array = GetArray(Ctx);
		if (Array.Num() <= Index) return;
		Array[Index] = Value;
		SetArray(Ctx, Array);
	}
}

void UFIRArrayProperty::RemoveValueInArray(void* Ctx, int Index) const {
	if (Property) {
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		SArr.RemoveValues(Index);
	} else {
		FIRArray Array = GetArray(Ctx);
		Array.RemoveAt(Index);
		SetArray(Ctx, Array);
	}
}

void UFIRArrayProperty::EmptyArray(void* Ctx) const {
	if (Property) {
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		SArr.EmptyValues();
	} else {
		FIRArray Array = GetArray(Ctx);
		Array.Empty();
		SetArray(Ctx, Array);
	}
}

int UFIRArrayProperty::GetNumOfArray(void* Ctx) const {
	if (Property) {
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		return SArr.Num();
	} else {
		FIRArray Array = GetArray(Ctx);
		return Array.Num();
	}
}
