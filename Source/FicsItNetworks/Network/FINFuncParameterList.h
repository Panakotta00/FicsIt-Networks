#pragma once
#include "FINParameterList.h"
#include "FINFuncParameterList.generated.h"

/**
 * A network parameter list based of a function
 * and the parameter struct.
 */
USTRUCT()
struct FFINFuncParameterList : public FFINParameterList {
	GENERATED_BODY()
public:
	UFunction* Func = nullptr;
	void* Data = nullptr;

	FFINFuncParameterList() {}
	FFINFuncParameterList(UFunction* Func);
	FFINFuncParameterList(UFunction* Func, void* Data);
	FFINFuncParameterList(const FFINFuncParameterList& Other);
	~FFINFuncParameterList();
	FFINFuncParameterList& operator=(const FFINFuncParameterList& Other);

	// Begin FFINParameterList
	virtual int operator>>(FFINValueReader& reader) const override;
	// End FFINParameterList

	bool Serialize(FArchive& Ar);
};

template<>
struct TStructOpsTypeTraits<FFINFuncParameterList> : public TStructOpsTypeTraitsBase2<FFINFuncParameterList>
{
	enum
	{
		WithSerializer = true,
    };
};

inline bool operator<<(FArchive& Ar, FFINFuncParameterList& ParamList) {
	return ParamList.Serialize(Ar);
}
