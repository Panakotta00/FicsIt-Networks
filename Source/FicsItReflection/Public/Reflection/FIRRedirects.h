#pragma once

#include "CoreMinimal.h"
#include "FIRGlobalRegisterHelper.h"
#include "FicsItReflection.h"

#define FIR_RedirectName(Prefix, From, To) FIRRedirect_ ## Prefix ## _ ## From ## _ ## To
#define FIR_Redirect_All(From, To) \
	FFIRStaticGlobalRegisterFunc FIR_RedirectName(All, From, To) ([](){ \
		FFicsItReflectionModule::Get().AddRedirect(TEXT(#From), TEXT(#To)); \
	});
