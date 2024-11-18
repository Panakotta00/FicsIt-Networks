#include "FicsItReflection.h"
#include "FINEventFilter.h"
#include "Reflection/Source/FIRSourceStaticMacros.h"

BeginStruct(FFINEventFilterExpression, "EventFilter", "Event Filter", "This struct contains filter settings so you can evaluate if a sent signal matches the filter or not.")
BeginFuncVA(matches, "Matches", "Returns true if the given signal data matches this event filter.") {
	InVal(0, RString, name, "Name", "The (internal) name of the signal.")
	InVal(1, RObject<UObject>, sender, "Sender", "The sender of the signal")
	OutVal(2, RBool, matches, "Matches", "True if the given signal matches the filter")
	Body()
	if (sender.IsValid()) {
		UFIRSignal* signal = FFicsItReflectionModule::Get().FindClass(sender->GetClass())->FindFIRSignal(name);
		if (IsValid(signal)) {
			FFINSignalData data(signal, TArray<FIRAny>(&Params[3], Params.Num()-3));
			matches = self->Matches(sender.Get(), data);
			return;
		}
	}
	matches = false;
} EndFunc()
BeginOp(FIR_Operator_Mul, 0, "And", "Creates a new Event Filter with an AND expression from two Event Filters") {
	InVal(0, RStruct<FFINEventFilterExpression>, operand, "Operand", "The other Operand.")
	OutVal(1, RStruct<FFINEventFilterExpression>, result, "Result", "The combined Expression.")
	Body()
	result = static_cast<FIRStruct>(FFINEventFilterExpression::And(*self, operand));
} EndFunc()
BeginOp(FIR_Operator_BitAND, 0, "And", "Creates a new Event Filter with an AND expression from two Event Filters") {
	InVal(0, RStruct<FFINEventFilterExpression>, operand, "Operand", "The other Operand.")
	OutVal(1, RStruct<FFINEventFilterExpression>, result, "Result", "The combined Expression.")
	Body()
	result = static_cast<FIRStruct>(FFINEventFilterExpression::And(*self, operand));
} EndFunc()
BeginOp(FIR_Operator_Add, 0, "Or", "Creates a new Event Filter with an OR expression from two Event Filters") {
	InVal(0, RStruct<FFINEventFilterExpression>, operand, "Operand", "The other Operand.")
	OutVal(1, RStruct<FFINEventFilterExpression>, result, "Result", "The combined Expression.")
	Body()
	result = static_cast<FIRStruct>(FFINEventFilterExpression::Or(*self, operand));
} EndFunc()
BeginOp(FIR_Operator_BitOR, 0, "Or", "Creates a new Event Filter with an OR expression from two Event Filters") {
	InVal(0, RStruct<FFINEventFilterExpression>, operand, "Operand", "The other Operand.")
	OutVal(1, RStruct<FFINEventFilterExpression>, result, "Result", "The combined Expression.")
	Body()
	result = static_cast<FIRStruct>(FFINEventFilterExpression::Or(*self, operand));
} EndFunc()
BeginOp(FIR_Operator_Neg, 0, "Not", "Creates a new Event Filter with an NOT expression from this Event Filter") {
	OutVal(1, RStruct<FFINEventFilterExpression>, result, "Result", "The output Expression.")
	Body()
	result = static_cast<FIRStruct>(FFINEventFilterExpression::Not(*self));
} EndFunc()
BeginOp(FIR_Operator_BitNOT, 0, "Not", "Creates a new Event Filter with an NOT expression from this Event Filter") {
	OutVal(1, RStruct<FFINEventFilterExpression>, result, "Result", "The output Expression.")
	Body()
	result = static_cast<FIRStruct>(FFINEventFilterExpression::Not(*self));
} EndFunc()
EndStruct()
