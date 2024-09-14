#pragma once

#include "FIVSRuntimeContext.h"
#include "FicsItKernel/Processor/Processor.h"
#include "Script/FIVSScriptContext.h"
#include "Script/FIVSScriptNode.h"
#include "Reflection/FINReflection.h"
#include "FIVSProcessor.generated.h"

class AFINStateEEPROMText;

UCLASS()
class UFIVSProcessor : public UFINKernelProcessor, public IFIVSScriptContext_Interface {
	GENERATED_BODY()

	UPROPERTY()
	AFINStateEEPROMText* EEPROM = nullptr;

	uint32 GraphHash = 0;
	TOptional<FFIVSScript> TickScript;

	TSharedPtr<FFIVSRuntimeContext> RuntimeContext;

public:
	// Begin UFINKernelProcessor
	virtual void Tick(float InDeltaTime) override;
	virtual void Reset() override;
	virtual void Stop(bool) override;
	virtual void SetEEPROM(AFINStateEEPROM* InEEPROM) override;
	// End UFINKernelProcessor

	// Begin IFIVSScriptContext_Interface
	virtual void GetRelevantObjects_Implementation(TArray<FFINNetworkTrace>& OutObjects) override;
	virtual void GetRelevantClasses_Implementation(TArray<UFINClass*>& OutClasses) override;
	virtual void GetRelevantStructs_Implementation(TArray<UFINStruct*>& OutStructs) override;
	// End IFVSScriptContext_Interface
};
