#include "stdafx.h"

#include <util/JsonConfig.h>

#include "Config.h"

json config = SML::Utility::JsonConfig::load(MOD_NAME, {
	{"SignalQueueSize", 32}
});