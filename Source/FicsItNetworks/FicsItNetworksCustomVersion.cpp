#include "FicsItNetworksCustomVersion.h"
#include "CustomVersion.h"

const FGuid FFINCustomVersion::GUID = FGuid(0xc7abe2f0, 0xf82242c2, 0x8ea32b9d, 0xedb72ce9);

//Stuff to register custom version for UE4 tracking
FCustomVersionRegistration GRegisterFactoryGameCustomVersion{ FFINCustomVersion::GUID, 1, TEXT("Dev-Framework") };
