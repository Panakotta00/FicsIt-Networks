#include "FINItemStateEEPROMLua.h"

#include "FicsItNetworksLuaModule.h"

FFGDynamicStruct AFINStateEEPROMLua_Legacy::ConvertToItemState(TSubclassOf<UFGItemDescriptor> itemDescriptor) const {
	FFINItemStateEEPROMLua state;
	state.Label = Label;
	state.Code = Code;
	return FFGDynamicStruct(state);
}
