#include "FINComponentUtility.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

void UFINComponentUtility::ClipboardCopy(FString str) {
	FWindowsPlatformApplicationMisc::ClipboardCopy(*str);
}
