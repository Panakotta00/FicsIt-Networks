#include "FINEventFilter.h"

bool FFINEventFilter::Matches(UObject* Sender, const FFINSignalData& Signal) const {
	if (!Senders.IsEmpty() && !Senders.Contains(Sender)) return false;

	if (!Events.IsEmpty() && !Events.Contains(Signal.Signal->GetInternalName())) return false;

	if (!ValueFilters.IsEmpty()) {
		TArray<UFIRProperty*> params = Signal.Signal->GetParameters();
		int num = FMath::Min(params.Num(), Signal.Data.Num());
		int numEvaluated = 0;
		for (int i = 0; i < num; ++i) {
			UFIRProperty* param = params[i];
			const FFIRAnyValue* filter = ValueFilters.Find(param->GetInternalName());
			if (!filter) continue;
			numEvaluated += 1;
			const FFIRAnyValue& value = Signal.Data[i];

			if (filter->GetType() != value.GetType()) return false;
			switch (filter->GetType()) {
				case FIR_BOOL:
					if (filter->GetBool() != value.GetBool()) return false;
					break;
				case FIR_INT:
					if (filter->GetInt() != value.GetInt()) return false;
					break;
				case FIR_FLOAT:
					if (filter->GetFloat() != value.GetFloat()) return false;
					break;
				case FIR_STR:
					if (filter->GetString() != value.GetString()) return false;
					break;
				case FIR_OBJ:
					if (filter->GetObj() != value.GetObj()) return false;
					break;
				case FIR_CLASS:
					if (filter->GetClass() != value.GetClass()) return false;
					break;
				case FIR_TRACE:
					if (filter->GetTrace() != value.GetTrace()) return false;
					break;
				case FIR_STRUCT:
					if (filter->GetStruct() != value.GetStruct()) return false;
					break;
				case FIR_ARRAY:
					break;
				default: break;
			}
		}
		if (numEvaluated != ValueFilters.Num()) return false;
	}

	return true;
}

bool FFINEventFilterExpression::Matches(UObject* Sender, const FFINSignalData& Signal) const {
	switch (Operator) {
		case FIN_EventFilter_None:
			if (!IsValid(Operand1.GetStruct())) return true;
			if (Operand1.GetStruct() != FFINEventFilter::StaticStruct()) return false;
			return Operand1.Get<FFINEventFilter>().Matches(Sender, Signal);
		case FIN_EventFilter_AND:
			if (Operand1.GetStruct() != FFINEventFilterExpression::StaticStruct() || Operand2.GetStruct() != FFINEventFilterExpression::StaticStruct()) return false;
			return Operand1.Get<FFINEventFilterExpression>().Matches(Sender, Signal) && Operand2.Get<FFINEventFilterExpression>().Matches(Sender, Signal);
		case FIN_EventFilter_OR:
			if (Operand1.GetStruct() != FFINEventFilterExpression::StaticStruct() || Operand2.GetStruct() != FFINEventFilterExpression::StaticStruct()) return false;
			return Operand1.Get<FFINEventFilterExpression>().Matches(Sender, Signal) || Operand2.Get<FFINEventFilterExpression>().Matches(Sender, Signal);
		case FIN_EventFilter_NOT:
			if (Operand1.GetStruct() != FFINEventFilterExpression::StaticStruct()) return false;
			return !Operand1.Get<FFINEventFilterExpression>().Matches(Sender, Signal);
		default:
			return false;
	}
}
