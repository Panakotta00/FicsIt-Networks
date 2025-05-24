#pragma once

#include "CoreMinimal.h"
#include "FIVSNode.h"
#include "FIVSCompileError.generated.h"

USTRUCT()
struct FFIVSCompileError {
	GENERATED_BODY()

	UPROPERTY()
	UFIVSNode* Node = nullptr;

	UPROPERTY()
	UFIVSPin* Pin = nullptr;

	UPROPERTY()
	FText Message;

	FFIVSCompileError() = default;
	FFIVSCompileError(UFIVSNode* InNode, FText InMessage) : Node(InNode), Message(InMessage) {}
	FFIVSCompileError(UFIVSPin* InPin, FText InMessage) : Node(InPin->ParentNode), Pin(InPin), Message(InMessage) {}
};
