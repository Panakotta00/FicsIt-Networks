#pragma once

#include "CoreMinimal.h"
#include "FINSignalData.h"
#include "FIRSignal.h"
#include "FINEventFilter.generated.h"

USTRUCT()
struct FFINEventFilter {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<UObject*> Senders;

	UPROPERTY(SaveGame)
	TArray<FString> Events;

	UPROPERTY(SaveGame)
	TMap<FString, FFIRAnyValue> ValueFilters;

	bool Matches(UObject* Sender, const FFINSignalData& Signal) const;
};

UENUM()
enum EFINEventFilterExpressionOperator {
	FIN_EventFilter_None,
	FIN_EventFilter_AND,
	FIN_EventFilter_OR,
	FIN_EventFilter_NOT,
};

USTRUCT()
struct FFINEventFilterExpression {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FFIRInstancedStruct Operand1;

	UPROPERTY(SaveGame)
	TEnumAsByte<EFINEventFilterExpressionOperator> Operator = FIN_EventFilter_None;

	UPROPERTY(SaveGame)
	FFIRInstancedStruct Operand2;

	FFINEventFilterExpression() = default;
	FFINEventFilterExpression(const FFINEventFilter& Filter) : Operand1(Filter) {}
	FFINEventFilterExpression(const FFINEventFilterExpression& Operand1, EFINEventFilterExpressionOperator Operator) : Operand1(Operand1), Operator(Operator) {}
	FFINEventFilterExpression(const FFINEventFilterExpression& Operand1, EFINEventFilterExpressionOperator Operator, const FFINEventFilterExpression& Operand2) : Operand1(Operand1), Operator(Operator), Operand2(Operand2) {}

	static FFINEventFilterExpression And(const FFINEventFilterExpression& Operand1, const FFINEventFilterExpression& Operand2) {
		return FFINEventFilterExpression(Operand1, FIN_EventFilter_AND, Operand2);
	}

	static FFINEventFilterExpression Or(const FFINEventFilterExpression& Operand1, const FFINEventFilterExpression& Operand2) {
		return FFINEventFilterExpression(Operand1, FIN_EventFilter_OR, Operand2);
	}

	static FFINEventFilterExpression Not(const FFINEventFilterExpression& Operand1) {
		return FFINEventFilterExpression(Operand1, FIN_EventFilter_NOT);
	}

	bool Matches(UObject* Sender, const FFINSignalData& Signal) const;
};
