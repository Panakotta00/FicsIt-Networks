#include "FINStateEEPROMLua.h"

FFGDynamicStruct AFINStateEEPROMLua_Legacy::ConvertToItemState(TSubclassOf<UFGItemDescriptor> itemDescriptor) const {
	FFINStateEEPROMLua state;
	state.Code = Code;
	return FFGDynamicStruct(state);
}
