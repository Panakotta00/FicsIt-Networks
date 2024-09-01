#include "UI/FINLogViewer.h"

#include "FINConfigurationStruct.h"
#include "Configuration/Properties/ConfigPropertyBool.h"
#include "Engine/GameInstance.h"
#include "FicsItLogLibrary/Public/FILLogContainer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Text/RichTextLayoutMarshaller.h"
#include "Framework/Text/RichTextMarkupProcessing.h"
#include "UI/FINRichtTextUtils.h"
#include "UI/FINTextDecorators.h"
#include "Utils/FINUtils.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
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
	TextLogStyle.GetResources(OutBrushes);
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
	return SNew(SFINLogViewer, this)
	.Style(&Style)
	.OnNavigateReflection_Lambda([this](UFIRBase* ReflectionItem) {
		OnNavigateReflection.Broadcast(ReflectionItem);
	})
	.OnNavigateEEPROM_Lambda([this](int64 LineNumber) {
		OnNavigateEEPROM.Broadcast(LineNumber);
	});
}

UFINLogViewer::UFINLogViewer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	Style = FFINStyle::Get().GetWidgetStyle<FFINLogViewerStyle>("LogViewer");
}

void UFINLogViewer::SetLog(UFILLogContainer* InLog) {
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

void SFINLogViewer::Construct(const FArguments& InArgs, UObject* InWorldContext) {
	WorldContext = InWorldContext;
	Style = InArgs._Style;
	NavigateReflectionDelegate = InArgs._OnNavigateReflection;
	NavigateEEPROMDelegate = InArgs._OnNavigateEEPROM;

#if !WITH_EDITOR
	FFINConfigurationStruct Config = FFINConfigurationStruct::GetActiveConfig(WorldContext);
	bTextOutputEnabled = Config.LogViewer.TextLog;
	TextTimestampEnabled = Config.LogViewer.TextLogTimestamp;
	TextVerbosityEnabled = Config.LogViewer.TextLogVerbosity;
	TextMultilineAlignEnabled = Config.LogViewer.TextLogMultilineAlign;
#endif

	ChildSlot[
		SNew(SOverlay)
		+SOverlay::Slot()[
			SNew(SWidgetSwitcher)
			.WidgetIndex_Lambda([this]() {
				return bTextOutputEnabled ? 1 : 0;
			})
			+SWidgetSwitcher::Slot()[
				SAssignNew(ListView, SListView<TSharedRef<FFILEntry>>)
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
			]
			+SWidgetSwitcher::Slot().Padding(0, Style->IconSize.X + Style->IconBoxPadding.GetDesiredSize2f().X, 0, 0)[
				SAssignNew(TextLog, SMultiLineEditableTextBox)
				.Style(&Style->TextLogStyle)
				.IsReadOnly(true)
				// TODO: Add marshaller for color & references/links
			]
		]
		+SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(3, 4, 3, 3)[
			SNew(SButton)
			.ButtonStyle(&Style->TextLogButtonStyle)
			.ContentPadding(0)
			.OnClicked_Lambda([this]() {
				bTextOutputEnabled = !bTextOutputEnabled;

				FConfigId ConfigId{"FicsItNetworks", ""};
				if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
					UConfigManager* ConfigManager = WorldContext->GetWorld()->GetGameInstance()->GetSubsystem<UConfigManager>();
					UConfigPropertyBool* Prop = Cast<UConfigPropertyBool>(Cast<UConfigPropertySection>(ConfigManager->GetConfigurationRootSection(ConfigId)->SectionProperties[TEXT("LogViewer")])->SectionProperties[TEXT("TextLog")]);
					Prop->Value = bTextOutputEnabled;
					Prop->MarkDirty();
				}
				
				return FReply::Handled();
			})
			.Content()[
				SNew(SBox)
				.HeightOverride(Style->IconSize.X + Style->IconBoxPadding.GetDesiredSize2f().X - 6)
				.WidthOverride(Style->IconSize.X + Style->IconBoxPadding.GetDesiredSize2f().X - 6)
			]
		]
	];
}

FReply SFINLogViewer::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::C && InKeyEvent.GetModifierKeys().IsControlDown()) {
		FString Text;
		for (const TSharedRef<FFILEntry>& Entry : ListView->GetSelectedItems()) {
			if (!Text.IsEmpty()) Text += TEXT("\n");
			Text += Entry->ToClipboardText();
		}
		FWindowsPlatformApplicationMisc::ClipboardCopy(*Text);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SFINLogViewer::UpdateEntries(UFILLogContainer* Log) {
	bool bBottom = ListView->GetScrollDistanceRemaining().IsNearlyZero() || !ListView->IsScrollbarNeeded();
	
	Entries.Empty();
	FString Text;
	if (Log) {
		bool bTextTimestampEnabled = TextTimestampEnabled.Get();
		bool bTextVerbosityEnabled = TextVerbosityEnabled.Get();
		bool bTextMultilineAlignEnabled = TextMultilineAlignEnabled.Get();

		int LineCount = 0;
		for (const FFILEntry& Entry : Log->GetLogEntries()) {
			LineCount += 1;
			Entries.Add(MakeShared<FFILEntry>(Entry));

			if (!Text.IsEmpty()) Text.AppendChar(L'\n');
			FString Line;
			if (bTextTimestampEnabled) Line.Append(Entry.Timestamp.ToString()).AppendChar(L' ');
			if (bTextVerbosityEnabled) Line.Append(FString::Printf(TEXT("[%s] "), *Entry.GetVerbosityAsText().ToString()));
			Text.Append(Line);
			
			if (bTextMultilineAlignEnabled) {
				FString Spacer = TEXT("\n") + FString::ChrN(Line.Len(), L' ');
				TArray<FTextRange> LineRanges;
				FTextRange::CalculateLineRangesFromString(Entry.Content, LineRanges);
				bool bFirstDone = false;
				for (const FTextRange Range : LineRanges) {
					if (bFirstDone) {
						Text.Append(Spacer);
						LineCount += 1;
					}
					bFirstDone = true;
					Text.Append(UFINUtils::TextRange(Entry.Content, Range));
				}
			} else {
				Text.Append(Entry.Content);
			}
		}
	}
#if WITH_EDITOR
	Entries.Add(MakeShared<FFILEntry>(FDateTime::Now(), FIN_Log_Verbosity_Debug, "Sample Debug?"));
	Entries.Add(MakeShared<FFILEntry>(FDateTime::Now(), FIN_Log_Verbosity_Info, "Sample Info."));
	Entries.Add(MakeShared<FFILEntry>(FDateTime::Now(), FIN_Log_Verbosity_Warning, "Sample Warning!"));
	Entries.Add(MakeShared<FFILEntry>(FDateTime::Now(), FIN_Log_Verbosity_Error, "Sample Error!!"));
	Entries.Add(MakeShared<FFILEntry>(FDateTime::Now(), FIN_Log_Verbosity_Fatal, "Sample Fatal!!!"));
#endif
	
	ListView->RebuildList();
	if (bBottom) ListView->ScrollToBottom();

	TextLog->SetText(FText::FromString(Text));
}

TSharedRef<ITableRow> SFINLogViewer::OnGenerateRow(TSharedRef<FFILEntry> Entry, const TSharedRef<STableViewBase>& InListView) {
	TSharedRef<SFINLogViewerRow> Row = SNew(SFINLogViewerRow, InListView, Style, Entry, NavigateReflectionDelegate, NavigateEEPROMDelegate)
		.Style(&Style->RowStyle);
	return Row;
}

void SFINLogViewerRow::Construct(const FTableRowArgs& InArgs, const TSharedRef<STableViewBase>& OwnerTable, const FFINLogViewerStyle* InStyle, const TSharedRef<FFILEntry>& LogEntry, const SFINLogViewer::FOnNavigateReflection& InNavigateReflectionDelegate, const SFINLogViewer::FOnNavigateEEPROM& InNavigateEEPROMDelegate) {
	Style = InStyle;
	Entry = LogEntry;
	NavigateReflectionDelegate = InNavigateReflectionDelegate;
	NavigateEEPROMDelegate = InNavigateEEPROMDelegate;
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
		TArray<TSharedRef<ITextDecorator>> Decorators = {
			MakeShared<FFINReflectionReferenceDecorator>(NavigateReflectionDelegate),
			MakeShared<FFINEEPROMReferenceDecorator>(NavigateEEPROMDelegate)
		};

		TSharedRef<IRichTextMarkupParser> Parser = MakeShared<FFINLogTextParser>();
		
		TSharedRef<FRichTextLayoutMarshaller> Marshaller = FRichTextLayoutMarshaller::Create(Parser, MakeShared<FFINPureTextWriter>(), Decorators, &FCoreStyle::Get());
		
		return SNew(SBox)
		.VAlign(VAlign_Top)[
			SNew(SMultiLineEditableTextBox)
			.Style(&Style->TextBoxStyle)
			.Text(FText::FromString(GetLogEntry()->Content))
			.Font(GetFontInfo())
			.ForegroundColor(GetTextColor())
			.AutoWrapText(true)
			.IsReadOnly(true)
			.Visibility(this, &SFINLogViewerRow::GetTextVisibility)
			.Marshaller(Marshaller)
		];
	} else {
		return SNew(SBorder);
	}
}

const FSlateBrush* SFINLogViewerRow::GetVerbosityIcon() const {
	switch (GetLogEntry()->Verbosity) {
	case FIL_Verbosity_Debug: return &Style->IconDebug;
	case FIL_Verbosity_Info: return &Style->IconInfo;
	case FIL_Verbosity_Warning: return &Style->IconWarning;
	case FIL_Verbosity_Error: return &Style->IconError;
	case FIL_Verbosity_Fatal: return &Style->IconFatal;
	default: return &Style->IconInfo;
	}
}

const FSlateFontInfo& SFINLogViewerRow::GetFontInfo() const {
	switch (GetLogEntry()->Verbosity) {
	case FIL_Verbosity_Debug: return Style->DebugText;
	case FIL_Verbosity_Info: return Style->InfoText;
	case FIL_Verbosity_Warning: return Style->WarningText;
	case FIL_Verbosity_Error: return Style->ErrorText;
	case FIL_Verbosity_Fatal: return Style->FatalText;
	default: return Style->InfoText;
	}
}

const FSlateColor& SFINLogViewerRow::GetTextColor() const {
	switch (GetLogEntry()->Verbosity) {
	case FIL_Verbosity_Debug: return Style->ColorDebug;
	case FIL_Verbosity_Info: return Style->ColorInfo;
	case FIL_Verbosity_Warning: return Style->ColorWarning;
	case FIL_Verbosity_Error: return Style->ColorError;
	case FIL_Verbosity_Fatal: return Style->ColorFatal;
	default: return Style->ColorInfo;
	}
}

EVisibility SFINLogViewerRow::GetTextVisibility() const {
	if (IsSelected() || FSlateApplication::Get().GetModifierKeys().IsControlDown()) {
		return EVisibility::Visible;
	} else {
		return EVisibility::HitTestInvisible;
	}
}

const FRegexPattern FFINLogTextParser::Pattern(TEXT("((\\w+)<([\\w<>]*)>)|(EEPROM:(\\d+))"));

void FFINLogTextParser::Process(TArray<FTextLineParseResults>& Results, const FString& Input, FString& Output) {
	TArray<FTextRange> LineRanges;
	FTextRange::CalculateLineRangesFromString(Input, LineRanges);
	
	for(int32 i = 0; i < LineRanges.Num(); ++i) {
		const FTextRange& LineRange = LineRanges[i];
		FTextLineParseResults LineParseResults(LineRange);
		
		int BeginParseRange = LineRange.BeginIndex;

		FRegexMatcher Matcher(Pattern, Input.Mid(LineRange.BeginIndex, LineRange.Len()));
		while (Matcher.FindNext()) {
			FTextRange MatchRange(Matcher.GetMatchBeginning(), Matcher.GetMatchEnding());
			MatchRange.Offset(LineRange.BeginIndex);

			FTextRange Prefix(BeginParseRange, MatchRange.BeginIndex);
			if (!Prefix.IsEmpty()) {
				FTextRunParseResults NormalResult(FString(), Prefix, Prefix);
				LineParseResults.Runs.Add(NormalResult);
			}

			FString TypeVariant = Matcher.GetCaptureGroup(2);
			if (!TypeVariant.IsEmpty()) {
				FTextRange VariantRange(Matcher.GetCaptureGroupBeginning(2), Matcher.GetCaptureGroupEnding(2));
				VariantRange.Offset(LineRange.BeginIndex);
				FTextRange TypeRange(Matcher.GetCaptureGroupBeginning(3), Matcher.GetCaptureGroupEnding(3));
				TypeRange.Offset(LineRange.BeginIndex);
				FTextRunParseResults ClassResult(FFINReflectionReferenceDecorator::Id, MatchRange, MatchRange);
				ClassResult.MetaData.Add(FFINReflectionReferenceDecorator::MetaDataVariantKey, VariantRange);
				ClassResult.MetaData.Add(FFINReflectionReferenceDecorator::MetaDataTypeKey, TypeRange);
				LineParseResults.Runs.Add(ClassResult);
			}

			FString EEPROMLocation = Matcher.GetCaptureGroup(4);
			if (!EEPROMLocation.IsEmpty()) {
				FTextRange LineNumberRange(Matcher.GetCaptureGroupBeginning(5), Matcher.GetCaptureGroupEnding(5));
				LineNumberRange.Offset(LineRange.BeginIndex);
				FTextRunParseResults EEPROMResult(FFINEEPROMReferenceDecorator::Id, MatchRange, MatchRange);
				EEPROMResult.MetaData.Add(FFINEEPROMReferenceDecorator::MetaDataLineNumberKey, LineNumberRange);
				LineParseResults.Runs.Add(EEPROMResult);
			}
			
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
