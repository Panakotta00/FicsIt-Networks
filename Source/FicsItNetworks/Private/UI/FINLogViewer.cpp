#include "UI/FINLogViewer.h"

#include "FicsItKernel/Logging.h"
#include "Reflection/FINReflection.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableViewBase.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

#define LOCTEXT_NAMESPACE "LogViewer"

const FName FIN_LogViewer_Row_Name_VerbosityIcon("VerbosityIcon");
const FName FIN_LogViewer_Row_Name_Time("Time");
const FName FIN_LogViewer_Row_Name_Verbosity("Verbosity");
const FName FIN_LogViewer_Row_Name_Content("Content");

const FName FFINLogViewerStyle::TypeName(TEXT("LogViewerStyle"));

void FFINLogViewerStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const {
	RowStyle.GetResources(OutBrushes);
	TextBoxStyle.GetResources(OutBrushes);
	
	OutBrushes.Add(&IconDebug);
	OutBrushes.Add(&IconInfo);
	OutBrushes.Add(&IconWarning);
	OutBrushes.Add(&IconError);
	OutBrushes.Add(&IconFatal);
}

const FFINLogViewerStyle& FFINLogViewerStyle::GetDefault() {
	static FFINLogViewerStyle Style;
	return Style;
}

TSharedRef<SWidget> UFINLogViewer::RebuildWidget() {
	return SNew(SFINLogViewer)
	.Style(&Style)
	.OnNavigateReflection_Lambda([this](UFINBase* ReflectionItem) {
		OnNavigateReflection.Broadcast(ReflectionItem);
	});
}

UFINLogViewer::UFINLogViewer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	Style = FFINStyle::Get().GetWidgetStyle<FFINLogViewerStyle>("LogViewer");
}

void UFINLogViewer::SetLog(UFINLog* InLog) {
	if (Log) Log->OnLogEntriesUpdated.RemoveDynamic(this, &UFINLogViewer::UpdateLogEntries);
	Log = InLog;
	if (Log) Log->OnLogEntriesUpdated.AddDynamic(this, &UFINLogViewer::UpdateLogEntries);
	UpdateLogEntries();
}

void UFINLogViewer::UpdateLogEntries() {
	TSharedPtr<SFINLogViewer> LogViewer = StaticCastSharedPtr<SFINLogViewer>(MyWidget.Pin());
	if (LogViewer) {
		LogViewer->UpdateEntries(Log);
	}
}

void SFINLogViewer::Construct(const FArguments& InArgs) {
	Style = InArgs._Style;
	OnNavigateReflectionDelegate = InArgs._OnNavigateReflection;
	
	ChildSlot[
		SAssignNew(ListView, SListView<TSharedRef<FFINLogEntry>>)
		.ListItemsSource(&Entries)
		.ItemHeight(Style->IconSize.Y + Style->IconBoxPadding.GetDesiredSize2f().Y)
		.OnGenerateRow_Raw(this, &SFINLogViewer::OnGenerateRow)
		.EnableAnimatedScrolling(true)
		.HeaderRow(
			SNew(SHeaderRow)

			+SHeaderRow::Column(FIN_LogViewer_Row_Name_VerbosityIcon)
			.DefaultLabel(FText::GetEmpty())
			.FixedWidth(Style->IconSize.X + Style->IconBoxPadding.GetDesiredSize2f().X)

			+SHeaderRow::Column(FIN_LogViewer_Row_Name_Time)
			.DefaultLabel(LOCTEXT("ColumnTime", "Time"))
			.ManualWidth(150.0f)

			+SHeaderRow::Column(FIN_LogViewer_Row_Name_Verbosity)
			.DefaultLabel(LOCTEXT("ColumnVerbosity", "Verbosity"))
			.ManualWidth(100.0f)

			+SHeaderRow::Column(FIN_LogViewer_Row_Name_Content)
			.DefaultLabel(LOCTEXT("ColumnContent", "Message"))
			.FillWidth(1.0f)
		)
	];
}

FReply SFINLogViewer::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::C && InKeyEvent.GetModifierKeys().IsControlDown()) {
		FString Text;
		for (const TSharedRef<FFINLogEntry>& Entry : ListView->GetSelectedItems()) {
			if (!Text.IsEmpty()) Text += TEXT("\n");
			Text += Entry->ToClipboardText();
		}
		FWindowsPlatformApplicationMisc::ClipboardCopy(*Text);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SFINLogViewer::UpdateEntries(UFINLog* Log) {
	bool bBottom = ListView->GetScrollDistanceRemaining().IsNearlyZero() || !ListView->IsScrollbarNeeded();
	
	Entries.Empty();
	if (Log) {
		for (const FFINLogEntry& Entry : Log->GetLogEntries()) {
			Entries.Add(MakeShared<FFINLogEntry>(Entry));
		}
	}
#if WITH_EDITOR
	Entries.Add(MakeShared<FFINLogEntry>(FDateTime::Now(), FIN_Log_Verbosity_Debug, "Sample Debug?"));
	Entries.Add(MakeShared<FFINLogEntry>(FDateTime::Now(), FIN_Log_Verbosity_Info, "Sample Info."));
	Entries.Add(MakeShared<FFINLogEntry>(FDateTime::Now(), FIN_Log_Verbosity_Warning, "Sample Warning!"));
	Entries.Add(MakeShared<FFINLogEntry>(FDateTime::Now(), FIN_Log_Verbosity_Error, "Sample Error!!"));
	Entries.Add(MakeShared<FFINLogEntry>(FDateTime::Now(), FIN_Log_Verbosity_Fatal, "Sample Fatal!!!"));
#endif
	
	ListView->RebuildList();
	
	if (bBottom) ListView->ScrollToBottom();
}

TSharedRef<ITableRow> SFINLogViewer::OnGenerateRow(TSharedRef<FFINLogEntry> Entry, const TSharedRef<STableViewBase>& InListView) {
	TSharedRef<SFINLogViewerRow> Row = SNew(SFINLogViewerRow, InListView, Style, Entry)
		.Style(&Style->RowStyle);
	Row->OnNavigateReflectionDelegate.BindLambda([this](UFINBase* Base) {
		OnNavigateReflectionDelegate.ExecuteIfBound(Base);
	});
	return Row;
}

void SFINLogViewerRow::Construct(const FTableRowArgs& InArgs, const TSharedRef<STableViewBase>& OwnerTable, const FFINLogViewerStyle* InStyle, const TSharedRef<FFINLogEntry>& LogEntry) {
	Style = InStyle;
	Entry = LogEntry;
	FSuperRowType::Construct(InArgs, OwnerTable);
}

TSharedRef<SWidget> SFINLogViewerRow::GenerateWidgetForColumn(const FName& ColumnName) {
	if (ColumnName == FIN_LogViewer_Row_Name_VerbosityIcon) {
		return SNew(SBox)
		.Padding(Style->IconBoxPadding)
		.WidthOverride(Style->IconSize.X)
		.HeightOverride(Style->IconSize.Y)
		.VAlign(VAlign_Top)[
			SNew(SImage)
			.Image(GetVerbosityIcon())
		];
	} else if (ColumnName == FIN_LogViewer_Row_Name_Time) {
		return SNew(SBox)
		.VAlign(VAlign_Top)[
			SNew(SEditableTextBox)
			.Style(&Style->TextBoxStyle)
			.RenderOpacity(1.0)
			.Text(FText::FromString(GetLogEntry()->Timestamp.ToString()))
			.Font(GetFontInfo())
			.ForegroundColor(GetTextColor())
			.Visibility(this, &SFINLogViewerRow::GetTextVisibility)
			.IsReadOnly(true)
		];
	} else if (ColumnName == FIN_LogViewer_Row_Name_Verbosity) {
		return SNew(SBox)
		.VAlign(VAlign_Top)[
			SNew(SEditableTextBox)
			.Style(&Style->TextBoxStyle)
			.Text(GetLogEntry()->GetVerbosityAsText())
			.Font(GetFontInfo())
			.ForegroundColor(GetTextColor())
			.Visibility(this, &SFINLogViewerRow::GetTextVisibility)
			.IsReadOnly(true)
		];
	} else if (ColumnName == FIN_LogViewer_Row_Name_Content) {
		return SNew(SBox)
		.VAlign(VAlign_Top)[
			SNew(SRichTextBlock)
			.TextStyle(&Style->TextBlockStyle)
			.Text(FText::FromString(GetLogEntry()->Content))
			.AutoWrapText(true)
			.Visibility(this, &SFINLogViewerRow::GetTextVisibility)
			.Parser(MakeShared<FFINLogTextParser>())
			.Decorators({
				MakeShared<FFINReflectionReferenceDecorator>(TEXT("Class"), FSlateHyperlinkRun::FOnClick::CreateLambda([this](const FSlateHyperlinkRun::FMetadata& Metadata) {
					const FString* ClassName = Metadata.Find(TEXT("Class"));
					if (ClassName) {
						UFINClass* Class = FFINReflection::Get()->FindClass(*ClassName);
						if (Class) OnNavigateReflectionDelegate.ExecuteIfBound(Class);
					}
				})),
				MakeShared<FFINReflectionReferenceDecorator>(TEXT("Object"), FSlateHyperlinkRun::FOnClick::CreateLambda([this](const FSlateHyperlinkRun::FMetadata& Metadata) {
					const FString* ClassName = Metadata.Find(TEXT("Object"));
					if (ClassName) {
						UFINClass* Class = FFINReflection::Get()->FindClass(*ClassName);
						if (Class) OnNavigateReflectionDelegate.ExecuteIfBound(Class);
					}
				})),
				MakeShared<FFINReflectionReferenceDecorator>(TEXT("Trace"), FSlateHyperlinkRun::FOnClick::CreateLambda([this](const FSlateHyperlinkRun::FMetadata& Metadata) {
					const FString* ClassName = Metadata.Find(TEXT("Trace"));
					if (ClassName) {
						UFINClass* Class = FFINReflection::Get()->FindClass(*ClassName);
						if (Class) OnNavigateReflectionDelegate.ExecuteIfBound(Class);
					}
				})),
				MakeShared<FFINReflectionReferenceDecorator>(TEXT("Struct"), FSlateHyperlinkRun::FOnClick::CreateLambda([this](const FSlateHyperlinkRun::FMetadata& Metadata) {
					const FString* StructName = Metadata.Find(TEXT("Struct"));
					if (StructName) {
						UFINStruct* Struct = FFINReflection::Get()->FindClass(*StructName);
						if (Struct) OnNavigateReflectionDelegate.ExecuteIfBound(Struct);
					}
				}))
			})
		];
	} else {
		return SNew(SBorder);
	}
}

const FSlateBrush* SFINLogViewerRow::GetVerbosityIcon() const {
	switch (GetLogEntry()->Verbosity) {
	case FIN_Log_Verbosity_Debug: return &Style->IconDebug;
	case FIN_Log_Verbosity_Info: return &Style->IconInfo;
	case FIN_Log_Verbosity_Warning: return &Style->IconWarning;
	case FIN_Log_Verbosity_Error: return &Style->IconError;
	case FIN_Log_Verbosity_Fatal: return &Style->IconFatal;
	default: return &Style->IconInfo;
	}
}

const FSlateFontInfo& SFINLogViewerRow::GetFontInfo() const {
	switch (GetLogEntry()->Verbosity) {
	case FIN_Log_Verbosity_Debug: return Style->DebugText;
	case FIN_Log_Verbosity_Info: return Style->InfoText;
	case FIN_Log_Verbosity_Warning: return Style->WarningText;
	case FIN_Log_Verbosity_Error: return Style->ErrorText;
	case FIN_Log_Verbosity_Fatal: return Style->FatalText;
	default: return Style->InfoText;
	}
}

const FSlateColor& SFINLogViewerRow::GetTextColor() const {
	switch (GetLogEntry()->Verbosity) {
	case FIN_Log_Verbosity_Debug: return Style->ColorDebug;
	case FIN_Log_Verbosity_Info: return Style->ColorInfo;
	case FIN_Log_Verbosity_Warning: return Style->ColorWarning;
	case FIN_Log_Verbosity_Error: return Style->ColorError;
	case FIN_Log_Verbosity_Fatal: return Style->ColorFatal;
	default: return Style->ColorInfo;
	}
}

EVisibility SFINLogViewerRow::GetTextVisibility() const {
	if (IsSelected()) {
		return EVisibility::Visible;
	} else {
		return EVisibility::HitTestInvisible;
	}
}

FFINLogTextParser::FFINLogTextParser() : ClassPattern(TEXT("(\\w+)<([\\w<>]*)>")) {}

void FFINLogTextParser::Process(TArray<FTextLineParseResults>& Results, const FString& Input, FString& Output) {
	TArray<FTextRange> LineRanges;
	FTextRange::CalculateLineRangesFromString(Input, LineRanges);
	
	for(int32 i = 0; i < LineRanges.Num(); ++i) {
		const FTextRange& LineRange = LineRanges[i];
		FTextLineParseResults LineParseResults(LineRange);
		
		int BeginParseRange = LineRange.BeginIndex;

		FRegexMatcher Matcher(ClassPattern, Input.Mid(LineRange.BeginIndex, LineRange.Len()));
		while (Matcher.FindNext()) {
			FTextRange MatchRange(Matcher.GetMatchBeginning(), Matcher.GetMatchEnding());

			FTextRange Prefix(BeginParseRange, MatchRange.BeginIndex);
			if (!Prefix.IsEmpty()) {
				FTextRunParseResults NormalResult(FString(), Prefix, Prefix);
				LineParseResults.Runs.Add(NormalResult);
			}

			FString Type = Matcher.GetCaptureGroup(1);
			FTextRunParseResults ClassResult(Type, MatchRange, MatchRange);
			FTextRange TypeRange(Matcher.GetCaptureGroupBeginning(2), Matcher.GetCaptureGroupEnding(2));
			ClassResult.MetaData.Add(Type, TypeRange);
			LineParseResults.Runs.Add(ClassResult);
			BeginParseRange = MatchRange.EndIndex;
		}

		FTextRange Postfix(BeginParseRange, LineRange.EndIndex);
		if (!Postfix.IsEmpty()) {
			FTextRunParseResults RunParseResults(FString(), Postfix, Postfix);
			LineParseResults.Runs.Add(RunParseResults);
		}

		Results.Add(LineParseResults);
	}

	Output = Input;
}

FFINReflectionReferenceDecorator::FFINReflectionReferenceDecorator(FString InId,
	const FSlateHyperlinkRun::FOnClick& InNavigateDelegate,
	const FSlateHyperlinkRun::FOnGetTooltipText& InToolTipTextDelegate,
	const FSlateHyperlinkRun::FOnGenerateTooltip& InToolTipDelegate) : Id(InId), NavigateDelegate(InNavigateDelegate), ToolTipTextDelegate(InToolTipTextDelegate), ToolTipDelegate(ToolTipDelegate) {
	
}

bool FFINReflectionReferenceDecorator::Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const {
	return RunParseResult.Name == Id;
}

TSharedRef<ISlateRun> FFINReflectionReferenceDecorator::Create(const TSharedRef<FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef<FString>& InOutModelText, const ISlateStyle* Style) {
	FString StyleName = TEXT("Hyperlink");
	FString TextStyleName = TEXT("");

	FTextRange ModelRange;
	ModelRange.BeginIndex = InOutModelText->Len();
	*InOutModelText += OriginalText.Mid(RunParseResult.ContentRange.BeginIndex, RunParseResult.ContentRange.Len());
	ModelRange.EndIndex = InOutModelText->Len();

	if (!Style->HasWidgetStyle<FHyperlinkStyle>(FName(*StyleName)))	{
		Style = &FCoreStyle::Get();
	}

	FRunInfo RunInfo( RunParseResult.Name );
	for(const TPair<FString, FTextRange>& Pair : RunParseResult.MetaData) {
		RunInfo.MetaData.Add(Pair.Key, OriginalText.Mid( Pair.Value.BeginIndex, Pair.Value.Len()));
	}

	FHyperlinkStyle HyperlinkStyle = Style->GetWidgetStyle<FHyperlinkStyle>(FName(*StyleName));
	if (!TextStyleName.IsEmpty() && Style->HasWidgetStyle<FTextBlockStyle>(FName(*TextStyleName))) {
		HyperlinkStyle.SetTextStyle(Style->GetWidgetStyle<FTextBlockStyle>(FName(*TextStyleName)));
	}
	
	return FSlateHyperlinkRun::Create(RunInfo, InOutModelText, HyperlinkStyle, NavigateDelegate, ToolTipDelegate, ToolTipTextDelegate, ModelRange);
}
