#include "FINComponentUtility.h"
#include "HAL/PlatformApplicationMisc.h"

void UFINComponentUtility::ClipboardCopy(FString str) {
	FPlatformApplicationMisc::ClipboardCopy(*str);
}
