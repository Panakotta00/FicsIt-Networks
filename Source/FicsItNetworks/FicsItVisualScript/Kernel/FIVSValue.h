#pragma once

#include "FicsItNetworks/Network/FINAnyNetworkValue.h"
#include "FIVSValue.generated.h"

UENUM()
enum EFIVS_ValueType {
	FIVS_Value_RValue,
	FIVS_Value_LValue,
};

USTRUCT()
struct FFIVSValue {
	GENERATED_BODY()
private:
	EFIVS_ValueType ValueType;
	FFINAnyNetworkValue* Value;

public:
	explicit FFIVSValue(const FFINAnyNetworkValue& InValue) {
		ValueType = FIVS_Value_RValue;
		Value = new FFINAnyNetworkValue(InValue);
	}
	
	explicit FFIVSValue(FFINAnyNetworkValue* InValue) : Value(InValue) {
		ValueType = FIVS_Value_LValue;
	}
	
	FFIVSValue() : FFIVSValue(FFINAnyNetworkValue()) {}

	FFIVSValue(const FFIVSValue& InValue) : ValueType(InValue.ValueType) {
		if (ValueType == FIVS_Value_RValue) {
			Value = new FFINAnyNetworkValue(*InValue.Value);
		} else {
			Value = InValue.Value;
		}
	}

	FFIVSValue& operator=(const FFIVSValue& InValue) {
		if (InValue.ValueType == FIVS_Value_RValue) {
			if (ValueType == FIVS_Value_RValue) {
				*Value = *InValue.Value;
			} else {
				Value = new FFINAnyNetworkValue(*InValue.Value);
			}
		} else {
			if (ValueType == FIVS_Value_RValue) {
				delete Value;
			}
			Value = InValue.Value;
		}
		ValueType = InValue.ValueType;
		return *this;
	}

	FFINAnyNetworkValue& operator*() const {
		return *Value;
	}

	FFINAnyNetworkValue* operator->() const {
		return Value;
	}

	FFIVSValue& operator=(const FFINAnyNetworkValue& InValue) {
		*Value = InValue;
		return *this;
	}
}; 