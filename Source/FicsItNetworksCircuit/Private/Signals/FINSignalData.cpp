#include "Signals/FINSignalData.h"
#include "Reflection/FIRSignal.h"

bool FFINSignalData::Serialize(FStructuredArchive::FSlot Slot) {
	FStructuredArchive::FRecord Record = Slot.EnterRecord();
	Record.EnterField(TEXT("Signal")) << Signal;
	Record.EnterField(TEXT("Data")) << Data; 
	return true;
}
