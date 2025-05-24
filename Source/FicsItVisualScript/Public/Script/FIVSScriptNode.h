#pragma once

#include "FIVSNode.h"
#include "FIVSScriptNode.generated.h"

UCLASS(Abstract)
class UFIVSScriptNode : public UFIVSNode {
	GENERATED_BODY()
private:
	UPROPERTY()
	TArray<UFIVSPin*> Pins;

protected:
	/**
	 * Creates a new pin with the given info, adds it to the pin list and returns the pin.
	 */
	UFIVSPin* CreatePin(EFIVSPinType PinType, const FString& Name, const FText& InDisplayName, FFIVSPinDataType DataType = FFIVSPinDataType(FIR_ANY));
	UFIVSPin* CreateDefaultPin(EFIVSPinType PinType, const FName& Name, const FText& InDisplayName, FFIVSPinDataType DataType = FFIVSPinDataType(FIR_ANY));

	void DeletePin(UFIVSPin* Pin);
	void DeletePins(TArrayView<UFIVSPin*> Pins);

	/**
	 * Deletes the given pin and creates a new one with the new settings.
	 * Will reconnect all pin connections.
	 */
	void RecreatePin(UFIVSPin*& Pin, EFIVSPinType PinType, const FString& Name, const FText& InDisplayName, FFIVSPinDataType DataType = FFIVSPinDataType(FIR_ANY));

public:
	// Begin UFIVSNode
	virtual TArray<UFIVSPin*> GetNodePins() const override { return Pins; }
	// End UFIVSNode
};
