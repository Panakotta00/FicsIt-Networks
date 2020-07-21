#pragma once

#include "FINParameterReader.h"

/**
 * This structure allows you to store a parameter list
 * which might be staticly or dynamicly allocated.
 * It is used as parameters of messages like network signals and network messages
 * which can be serialized and also converted to a network data type.
 */
class FFINParameterList {
public:
	virtual ~FFINParameterList() {}
	
	/**
	* Writes all parameters in order to the parameter reader.
	* Allowing to convert the signal arguments to the values each processor supports.
	*/
	virtual int operator>>(FFINParameterReader& reader) const = 0;

	/**
	* De/Serialize the parameter list to an archive
	*
	* @param[in]	Ar	the archive which stores the signal information
	*/
	virtual bool Serialize(FArchive& Ar) = 0;
};

inline bool operator<<(FArchive& Ar, FFINParameterList& ParamList) {
	return ParamList.Serialize(Ar);
}
