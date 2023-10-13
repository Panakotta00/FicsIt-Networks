#include "UI/FINRichtTextUtils.h"

void FFINPureTextWriter::Write(const TArray<FRichTextLine>& InLines, FString& Output) {
	for (int32 LineIndex = 0; LineIndex < InLines.Num(); ++LineIndex) {
		const FRichTextLine& Line = InLines[LineIndex];

		if(LineIndex > 0) {
			Output += LINE_TERMINATOR;
		}

		for (const FRichTextRun& Run : Line.Runs) {
			Output.Append(Run.Text);
		}
	}
}
