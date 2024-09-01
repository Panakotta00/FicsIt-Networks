#pragma once

#include "CoreMinimal.h"
#include "FicsItKernel/Logging.h"
#include "FINStyle.h"
#include "FINTextDecorators.h"
#include "SlateCore.h"
#include "Components/Widget.h"
#include "Framework/Text/IRichTextMarkupParser.h"
#include "Framework/Text/IRichTextMarkupWriter.h"
#include "Framework/Text/SlateHyperlinkRun.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Views/STableRow.h"
#include "FINLogViewer.generated.h"

USTRUCT(BlueprintType)
struct FFINLogViewerStyle : public FSlateWidgetStyle {
	GENERATED_USTRUCT_BODY()
	
	FFINLogViewerStyle() :
		TextLogStyle(FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox")),
		TextLogButtonStyle(FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button")),
		RowStyle(FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row")),
		TextBoxStyle(FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox")),
		TextBlockStyle(FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")) {}

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	static const FFINLogViewerStyle& GetDefault();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FEditableTextBoxStyle TextLogStyle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FButtonStyle TextLogButtonStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTableRowStyle RowStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FEditableTextBoxStyle TextBoxStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FTextBlockStyle TextBlockStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FMargin IconBoxPadding;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FVector2D IconSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateFontInfo DebugText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateFontInfo InfoText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateFontInfo WarningText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateFontInfo ErrorText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateFontInfo FatalText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush IconDebug;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush IconInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush IconWarning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush IconError;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateBrush IconFatal;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor ColorDebug;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor ColorInfo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor ColorWarning;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor ColorError;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FSlateColor ColorFatal;
};

UCLASS(BlueprintType, hidecategories=Object, MinimalAPI)
class UFINLogViewerWidgetStyle : public USlateWidgetStyleContainerBase {
	GENERATED_BODY()
	
public:
	UPROPERTY(Category=Appearance, EditAnywhere, BlueprintReadWrite, meta=(ShowOnlyInnerProperties))
	FFINLogViewerStyle LogViewerStyle;

	virtual const FSlateWidgetStyle* const GetStyle() const override {
		return &LogViewerStyle;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFINNavigateReflection, UFIRBase*, ReflectionItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFINNavigateEEPROM, int64, LineNumber);

UCLASS()
class UFINLogViewer : public UWidget {
	GENERATED_BODY()

	// Begin UWidget
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End UWidget

	UFINLogViewer(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	void SetLog(UFINLog* InLog);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UFINLog* GetLog() { return Log; }

	UFUNCTION(BlueprintCallable)
	void UpdateLogEntries();

public:
	UPROPERTY(BlueprintAssignable)
	FFINNavigateReflection OnNavigateReflection;
	UPROPERTY(BlueprintAssignable)
	FFINNavigateEEPROM OnNavigateEEPROM;
	
	UPROPERTY(EditAnywhere)
	FFINLogViewerStyle Style;

	UPROPERTY()
	UFINLog* Log = nullptr;
};

class SFINLogViewer : public SCompoundWidget {
public:
	typedef FFINReflectionReferenceDecorator::FOnNavigate FOnNavigateReflection;
	typedef FFINEEPROMReferenceDecorator::FOnNavigate FOnNavigateEEPROM;
	
	
	SLATE_BEGIN_ARGS(SFINLogViewer) :
		_Style(&FFINStyle::Get().GetWidgetStyle<FFINLogViewerStyle>("LogViewer")) {}
	SLATE_STYLE_ARGUMENT(FFINLogViewerStyle, Style)
	SLATE_ATTRIBUTE(bool, TextTimestampEnabled)
	SLATE_ATTRIBUTE(bool, TextVerbosityEnabled)
	SLATE_ATTRIBUTE(bool, TextMultilineAlignEnabled)
	SLATE_EVENT(FOnNavigateReflection, OnNavigateReflection)
	SLATE_EVENT(FOnNavigateEEPROM, OnNavigateEEPROM)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UObject* WorldContext);

	// Begin SWidget
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	// End SWidget

	void UpdateEntries(UFINLog* InLog);
	
private:
	TSharedRef<ITableRow> OnGenerateRow(TSharedRef<FFINLogEntry> Entry, const TSharedRef<class STableViewBase>& ListView);

public:
	FOnNavigateReflection NavigateReflectionDelegate;
	FOnNavigateEEPROM NavigateEEPROMDelegate;
	
private:
	const FFINLogViewerStyle* Style = nullptr;

	UObject* WorldContext = nullptr;
	TArray<TSharedRef<FFINLogEntry>> Entries;

	bool bTextOutputEnabled = false;
	TAttribute<bool> TextTimestampEnabled = true;
	TAttribute<bool> TextVerbosityEnabled = true;
	TAttribute<bool> TextMultilineAlignEnabled = true;
	
	TSharedPtr<SListView<TSharedRef<FFINLogEntry>>> ListView;
	TSharedPtr<SWidgetSwitcher> WidgetSwitcher;
	TSharedPtr<SMultiLineEditableTextBox> TextLog;
};

class SFINLogViewerRow : public SMultiColumnTableRow<TSharedRef<FFINLogEntry>> {
public:
	void Construct(const FTableRowArgs& InArgs, const TSharedRef<STableViewBase>& OwnerTable, const FFINLogViewerStyle* Style, const TSharedRef<FFINLogEntry>& LogEntry, const SFINLogViewer::FOnNavigateReflection& NavigateReflectionDelegate, const SFINLogViewer::FOnNavigateEEPROM& NavigateEEPROMDelegate);

	TSharedPtr<FFINLogEntry> GetLogEntry() const { return Entry; }
	
protected:
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

	const FSlateBrush* GetVerbosityIcon() const;
	const FSlateFontInfo& GetFontInfo() const;
	const FSlateColor& GetTextColor() const;
	EVisibility GetTextVisibility() const;

public:
	SFINLogViewer::FOnNavigateReflection NavigateReflectionDelegate;
	SFINLogViewer::FOnNavigateEEPROM NavigateEEPROMDelegate;

private:
	const FFINLogViewerStyle* Style = nullptr;
	TSharedPtr<FFINLogEntry> Entry;
};

class FICSITNETWORKS_API FFINLogTextParser : public IRichTextMarkupParser {
public:
	virtual void Process(TArray<FTextLineParseResults>& Results, const FString& Input, FString& Output) override;

private:
	static const FRegexPattern Pattern;
};
