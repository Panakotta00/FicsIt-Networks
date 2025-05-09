#pragma once

#include "CoreMinimal.h"
#include "FIRExtendedValueType.h"
#include "FIRProperty.h"
#include "FIVSPin.generated.h"

class UFIRStruct;

UENUM()
enum EFIVSPinType {
	FIVS_PIN_NONE			= 0b0,
	FIVS_PIN_INPUT			= 0b0001,
	FIVS_PIN_OUTPUT			= 0b0010,
	FIVS_PIN_DATA			= 0b0100,
	FIVS_PIN_EXEC			= 0b1000,
	FIVS_PIN_DATA_INPUT		= FIVS_PIN_DATA | FIVS_PIN_INPUT,
	FIVS_PIN_DATA_OUTPUT	= FIVS_PIN_DATA | FIVS_PIN_OUTPUT,
	FIVS_PIN_EXEC_INPUT		= FIVS_PIN_EXEC | FIVS_PIN_INPUT,
	FIVS_PIN_EXEC_OUTPUT	= FIVS_PIN_EXEC | FIVS_PIN_OUTPUT,
};
ENUM_CLASS_FLAGS(EFIVSPinType)

USTRUCT()
struct FFIVSPinDataType : public FFIRExtendedValueType {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame)
	bool bReference = false;

public:
	FFIVSPinDataType() = default;
	FFIVSPinDataType(const FFIVSPinDataType&) = default;
	FFIVSPinDataType(EFIRValueType Type) : FFIRExtendedValueType(Type) {}
	FFIVSPinDataType(const FFIRExtendedValueType& Other) : FFIRExtendedValueType(Other) {}
	FFIVSPinDataType(EFIRValueType InType, UFIRStruct* InRefType) : FFIRExtendedValueType(InType, InRefType) {}

	bool IsReference() const { return bReference; }

	bool Equals(const FFIVSPinDataType& Other) {
		if (Other.bReference && !bReference) return false;
		return Super::Equals(Other);
	}

	bool IsA(const FFIVSPinDataType& Other) const {
		if (Other.bReference && !bReference) return false;
		return Super::IsA(Other);
	}

	FFIVSPinDataType AsRef() const {
		FFIVSPinDataType NewType(*this);
		NewType.bReference = true;
		return NewType;
	}

	FFIVSPinDataType AsVal() const {
		FFIVSPinDataType NewType(*this);
		NewType.bReference = false;
		return NewType;
	}
};

USTRUCT()
struct FFIVSFullPinType {
	GENERATED_BODY()

	EFIVSPinType PinType;
	FFIVSPinDataType DataType;

	FFIVSFullPinType() = default;
	FFIVSFullPinType(EFIVSPinType PinType) : PinType(PinType) {}
	FFIVSFullPinType(EFIVSPinType PinType, FFIVSPinDataType DataType) : PinType(PinType), DataType(DataType) {}
	FFIVSFullPinType(UFIRProperty* Property) {
		EFIRPropertyFlags Flags = Property->GetPropertyFlags();
		if (Flags & FIR_Prop_Param) {
			if (Flags & FIR_Prop_OutParam) {
				PinType = FIVS_PIN_DATA_OUTPUT;
			} else {
				PinType = FIVS_PIN_DATA_INPUT;
			}
		} else if (Flags & FIR_Prop_Attrib) {
			PinType = FIVS_PIN_DATA;
		}
		DataType = FFIVSPinDataType(Property);
	}

	bool CanConnect(const FFIVSFullPinType& Other) const;
};

UCLASS(Abstract)
class UFIVSPin : public UObject {
	GENERATED_BODY()
protected:
	UPROPERTY()
	TArray<UFIVSPin*> ConnectedPins;

	UPROPERTY()
	FFIRAnyValue Literal;

public:
	UPROPERTY()
	FGuid PinId = FGuid::NewGuid();
	UPROPERTY()
	UFIVSNode* ParentNode = nullptr;
	UPROPERTY()
	FString Name = TEXT("Unnamed");
	UPROPERTY()
	FText DisplayName = FText::FromString("Unnamed");

	/**
	 * Returns the literal value of the given pin.
	 * The literal value will be used if pin has no connection to any other pins.
	 */
	FFIRAnyValue GetLiteral() {
		if (Literal.GetType() != GetPinDataType().GetType()) {
			SetLiteral(FFIRAnyValue::DefaultValue(GetPinDataType().GetType()));
		}
		return Literal;
	}

	/**
	 * Allows to set/change the literal of the pin.
	 * For more info on literals, see GetLiteral()
	 */
	void SetLiteral(FFIRAnyValue InLiteral) {
		if (InLiteral.GetType() == GetPinDataType().GetType()) Literal = InLiteral;
	}

	/**
	 * Returns the pin type use to determine if this pin is a input, output, data or exec pin.
	 */
	virtual EFIVSPinType GetPinType();

	/**
	 * Returns the pin data type use to check if two pins can be connected.
	 */
	virtual FFIVSPinDataType GetPinDataType();

	/**
	 * Returns all connected pins
	 */
	virtual const TArray<UFIVSPin*>& GetConnections();

	/**
	 * Returns the internal name of the pin. Mainly used for referencing the pin.
	 */
	virtual FString GetName() { return Name; }

	/**
	 * Returns the display name of the pin.
	 */
	virtual FText GetDisplayName() { return DisplayName; }

	/**
	 * Checks if the the pin can be connected to the given pin
	 */
	virtual bool CanConnect(UFIVSPin* Pin);

	/**
	 * Creates a connection between this and the given pin
	 */
	void AddConnection(UFIVSPin* Pin);

	/**
	 * Removes a connection between this and the given pin
	 */
	void RemoveConnection(UFIVSPin* Pin);

	void GetAllConnected(TArray<UFIVSPin*>& Searches);
	TArray<UFIVSPin*> GetAllConnected() {
		TArray<UFIVSPin*> Connected;
		GetAllConnected(Connected);
		return Connected;
	}

	/**
	 * Trys to find the data-source for the network of pins this pin is connected to,
	 * or in the case of exec pins, trys to find the next Exec-Pin.
	 * */
	UFIVSPin* FindConnected();

	/**
	 * Removes all connections of this pin
	 */
	void RemoveAllConnections();
};

UCLASS()
class UFIVSGenericPin : public UFIVSPin {
	GENERATED_BODY()
public:
	EFIVSPinType PinType = FIVS_PIN_NONE;
	FFIVSPinDataType PinDataType = FFIRExtendedValueType(FIR_NIL);

	// Begin UFINScriptPin
	virtual EFIVSPinType GetPinType() override;
	virtual FFIVSPinDataType GetPinDataType() override;
	// End UFINScriptPin

	static UFIVSGenericPin* Create(FFIVSPinDataType DataType, EFIVSPinType PinType, const FString& Name, const FText& DisplayName);
};

UCLASS()
class UFIVSWildcardPin : public UFIVSPin {
	GENERATED_BODY()
public:
	// Begin UFINScriptPin
	virtual EFIVSPinType GetPinType() override;
	virtual FFIVSPinDataType GetPinDataType() override;
	virtual bool CanConnect(UFIVSPin* Pin) override;
	// End UFINScriptPin
};
