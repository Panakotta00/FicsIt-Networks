#pragma once
#include "FINDynamicStructHolder.h"
#include "FINParameterList.h"
#include "FINValueReader.h"
#include "FINStructParameterList.generated.h"

/**
 * Contains a row of properties stored in a struct which are parseable to
 * a network parameter list.
 */
USTRUCT()
struct FFINStructParameterList : public FFINParameterList {
	GENERATED_BODY()
public:
	FFINDynamicStructHolder Struct;
	
	FFINStructParameterList() {}
	FFINStructParameterList(const FFINDynamicStructHolder& Struct);
	FFINStructParameterList(UScriptStruct* Struct, void* Data);

	// Begin FFINParameterList
	virtual int operator>>(FFINValueReader& reader) const override;
	// End FFINParameterList
	
	bool Serialize(FArchive& Ar);

	static int WriteToReader(UStruct* Struct, void* Data, FFINValueReader& reader);
};

template<>
struct TStructOpsTypeTraits<FFINStructParameterList> : TStructOpsTypeTraitsBase2<FFINStructParameterList>
{
	enum
	{
		WithSerializer = true,
    };
};
