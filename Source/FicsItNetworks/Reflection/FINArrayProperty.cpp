#include "FINArrayProperty.h"

FINAny UFINArrayProperty::GetValue(const FFINExecutionContext& Ctx) const {
	return GetArray(Ctx.GetGeneric());
}

void UFINArrayProperty::SetValue(const FFINExecutionContext& Ctx, const FINAny& Value) const {
	SetArray(Ctx.GetGeneric(), Value.GetArray());
}

FINArray UFINArrayProperty::GetArray(void* Ctx) const {
	if (Property) {
		FINArray Arr;
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		for (int i = 0; i < SArr.Num(); ++i) {
			Arr.Add(InnerType->GetValue(SArr.GetRawPtr(i)));
		}
		return Arr;
	}
	return Super::GetValue(Ctx).GetArray();
}

void UFINArrayProperty::SetArray(void* Ctx, const FINArray& Array) const {
	if (Property) {
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		SArr.EmptyValues();
		for (int i = 0; i < Array.Num(); ++i) {
			if (!InnerType->IsValidValue(Array[i])) continue;
			SArr.AddValue();
			InnerType->SetValue(SArr.GetRawPtr(i), Array[i]);
		}
	} else Super::SetValue(Ctx, Array);
}

void UFINArrayProperty::AddValueToArray(void* Ctx, const FINAny& Value) const {
	if (Property) {
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		SArr.AddValue();
		InnerType->SetValue(SArr.GetRawPtr(SArr.Num()), Value);
	} else {
		FINArray Array = GetArray(Ctx);
		Array.Add(Value);
		SetArray(Ctx, Array);
	}
}

FINAny UFINArrayProperty::GetValueInArray(void* Ctx, int Index) const {
	if (Property) {
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		if (SArr.Num() <= Index) return FINAny();
		return InnerType->GetValue(SArr.GetRawPtr(Index));
	} else {
		FINArray Array = GetArray(Ctx);
		if (Array.Num() <= Index) return FINAny();
		return Array[Index];
	}
}

void UFINArrayProperty::SetValueInArray(void* Ctx, int Index, const FINAny& Value) const {
	if (Property) {
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		if (SArr.Num() <= Index) return;
		InnerType->SetValue(SArr.GetRawPtr(Index), Value);
	} else {
		FINArray Array = GetArray(Ctx);
		if (Array.Num() <= Index) return;
		Array[Index] = Value;
		SetArray(Ctx, Array);
	}
}

void UFINArrayProperty::RemoveValueInArray(void* Ctx, int Index) const {
	if (Property) {
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		SArr.RemoveValues(Index);
	} else {
		FINArray Array = GetArray(Ctx);
		Array.RemoveAt(Index);
		SetArray(Ctx, Array);
	}
}

void UFINArrayProperty::EmptyArray(void* Ctx) const {
	if (Property) {
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		SArr.EmptyValues();
	} else {
		FINArray Array = GetArray(Ctx);
		Array.Empty();
		SetArray(Ctx, Array);
	}
}

int UFINArrayProperty::GetNumOfArray(void* Ctx) const {
	if (Property) {
		FScriptArrayHelper SArr = FScriptArrayHelper(Property, Ctx);
		return SArr.Num();
	} else {
		FINArray Array = GetArray(Ctx);
		return Array.Num();
	}
}
