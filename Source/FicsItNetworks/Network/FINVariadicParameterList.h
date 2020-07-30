#pragma once

#include "FINAnyNetworkValue.h"
#include "FINParameterList.h"
#include "FINVariadicParameterList.generated.h"

/**
 * This struct allows you to store an array of any network values
 * as a parameter list.
 */
USTRUCT()
struct FFINVariadicParameterList : public FFINParameterList {
	GENERATED_BODY()
protected:
	TArray<FFINAnyNetworkValue> Args;

public:
	FFINVariadicParameterList();

	bool Serialize(FArchive& Ar);
	
	// Begin FFINParameterList
	virtual int operator>>(FFINValueReader& reader) const override;
	// End FFINParameterList

	/**
	 * Adds the given network value to the end of the list.
	 *
	 * @param[in]	Val		the network value you want to add
	 * @return	a reference to this
	 */
	FFINVariadicParameterList& Add(const FFINAnyNetworkValue& Val);

	/**
	 * Returns the network value at the given index.
	 *
	 * @param[in]	Index	the index of the given network value
	 * @return	the network value at the given index
	 */
	const FFINAnyNetworkValue& Get(int Index) const;

	/**
	 * Returns the number of values stored in this parameter list
	 *
	 * @return	the number of values stored
	 */
	int Num() const;
};

template<>
struct TStructOpsTypeTraits<FFINVariadicParameterList> : TStructOpsTypeTraitsBase2<FFINVariadicParameterList>
{
	enum
	{
		WithSerializer = true,
    };
};

inline bool operator<<(FArchive& Ar, FFINVariadicParameterList& List) {
	return List.Serialize(Ar);
}
