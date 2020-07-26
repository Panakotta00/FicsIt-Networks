#pragma once

#include "FINValueReader.h"
#include "FINParameterList.generated.h"

/**
 * This structure allows you to store a parameter list
 * which might be staticly or dynamicly allocated.
 * It is used as parameters of messages like network signals and network messages
 * which can be serialized and also converted to a network data type.
 */
USTRUCT(BlueprintType)
struct FFINParameterList {
	GENERATED_BODY()
public:
	virtual ~FFINParameterList() {}
	
	/**
	* Writes all parameters in order to the parameter reader.
	* Allowing to convert the signal arguments to the values each processor supports.
	*/
	virtual int operator>>(FFINValueReader& reader) const { return 0; };

	/**
	* De/Serialize the parameter list to an archive
	*
	* @param[in]	Ar	the archive which stores the signal information
	*/
	bool Serialize(FArchive& Ar) { return false; };
};

inline bool operator<<(FArchive& Ar, FFINParameterList& ParamList) {
	return ParamList.Serialize(Ar);
}
