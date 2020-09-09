#pragma once

#include "CoreMinimal.h"

#include "Network/FINNetworkValues.h"

#include "FINScriptNode.generated.h"

class UFINScriptNode;

UENUM()
enum EFINScriptPinType {
	FIVS_PIN_INPUT			= 0b0001,
	FIVS_PIN_OUTPUT			= 0b0010,
	FIVS_PIN_DATA			= 0b0100,
	FIVS_PIN_EXEC			= 0b1000,
	FIVS_PIN_DATA_INPUT		= FIVS_PIN_DATA | FIVS_PIN_INPUT,
	FIVS_PIN_DATA_OUTPUT	= FIVS_PIN_DATA | FIVS_PIN_OUTPUT,
	FIVS_PIN_EXEC_INPUT		= FIVS_PIN_EXEC | FIVS_PIN_INPUT,
	FIVS_PIN_EXEC_OUTPUT	= FIVS_PIN_EXEC | FIVS_PIN_OUTPUT,
};

USTRUCT()
struct FFINScriptPin {
	GENERATED_BODY()

private:
	TArray<FFINScriptPin*> ConnectedPins;

public:
	UPROPERTY()
	UFINScriptNode* ParentNode = nullptr;

	UPROPERTY()
	TEnumAsByte<EFINNetworkValueType> DataType;

	UPROPERTY()
	TEnumAsByte<EFINScriptPinType> PinType;

	UPROPERTY()
	FString Name;
	
	FFINScriptPin() = default;
	FFINScriptPin(EFINNetworkValueType DataType, EFINScriptPinType PinType, FString Name) : DataType(DataType), PinType(PinType), Name(Name) {}
	~FFINScriptPin();
	
	/**
	 * Creates a connection between this and the given pin
	 */
	void AddConnection(FFINScriptPin* Pin);

	/**
	 * Removes a connection between this and the given pin
	 */
	void RemoveConnection(FFINScriptPin* Pin);

	/**
	 * Returns all connected pins
	 */
	const TArray<FFINScriptPin*>& GetConnections() const;
};

/**
* Notifies if the pin list of the node has changed.
* Param1: type of change (0 = pin added, 1 = pin removed)
* Param2: the changed pin
*/
DECLARE_MULTICAST_DELEGATE_TwoParams(FFINScriptGraphPinChanged, int, int);

UCLASS()
class UFINScriptNode : public UObject {
	GENERATED_BODY()
private:
	TArray<TSharedPtr<FFINScriptPin>> Pins;

public:
	UPROPERTY()
	FVector2D Pos;

	FString Name;

	FFINScriptGraphPinChanged OnPinChanged;

	/**
	 * Adds the given pin to the node
	 *
	 * @param[in]	Pin		the pin you want to add
	 * @return	the index of the pin in the pin array
	 */
	int AddPin(const TSharedPtr<FFINScriptPin>& Pin);

	/**
	 * Removes the pin at the given index from the node
	 *
	 * @param[in]	index	the index of the pin
	 */
	void RemovePin(int index);

	/**
	 * Returns the list of pins of this node
	 */
	const TArray<TSharedPtr<FFINScriptPin>> GetPins() const;
};
