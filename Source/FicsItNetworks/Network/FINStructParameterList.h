#pragma once
#include "FINDynamicStructHolder.h"
#include "FINParameterList.h"

class FFINStructParameterList : public FFINParameterList {
public:
	FFINDynamicStructHolder Struct;
	
	FFINStructParameterList() {}
	FFINStructParameterList(const FFINDynamicStructHolder& Struct);

	// Begin FFINParameterList
	virtual int operator>>(FFINParameterReader& reader) const override;
	virtual bool Serialize(FArchive& Ar) override;
	// End FFINParameterList
};
