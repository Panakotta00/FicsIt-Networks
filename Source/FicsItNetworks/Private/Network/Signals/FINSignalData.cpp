#include "Network/Signals/FINSignalData.h"
#include "Reflection/FINSignal.h"

bool FFINSignalData::Serialize(FStructuredArchive::FSlot Slot) {
	FStructuredArchive::FRecord Record = Slot.EnterRecord();
	Record.EnterField(SA_FIELD_NAME(TEXT("Signal"))) << Signal;
	Record.EnterField(SA_FIELD_NAME(TEXT("Data"))) << Data; 
	return true;
}
