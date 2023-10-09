#pragma once

#include "CoreMinimal.h"
#include "FicsItKernel/Logging.h"
#include "FINStyle.h"
#include "SlateCore.h"
#include "Components/Widget.h"
#include "Framework/Text/IRichTextMarkupParser.h"
#include "Framework/Text/SlateHyperlinkRun.h"
#include "Widgets/Views/STableRow.h"
#include "FINLogViewer.generated.h"

USTRUCT(BlueprintType)
struct FFINLogViewerStyle : public FSlateWidgetStyle {
	GENERATED_USTRUCT_BODY()
	
	FFINLogViewerStyle() :
		RowStyle(FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row")),
		TextBoxStyle(FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox")),
		TextBlockStyle(FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText")) {}

	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;

	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };

	static const FFINLogViewerStyle& GetDefault();

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
	UPROPERTY(Category=Appearance, EditAnywhere, BlueprintReadWrite)
	FFINLogViewerStyle LogViewerStyle;

	virtual const FSlateWidgetStyle* const GetStyle() const override {
		return &LogViewerStyle;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFINNavigateReflection, UFINBase*, ReflectionItem);

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
	
	UPROPERTY(EditAnywhere)
	FFINLogViewerStyle Style;

	UPROPERTY()
	UFINLog* Log = nullptr;
};

class SFINLogViewer : public SCompoundWidget {
public:
	DECLARE_DELEGATE_OneParam(FOnNavigateReflection, UFINBase*)
	
	SLATE_BEGIN_ARGS(SFINLogViewer) :
		_Style(&FFINStyle::Get().GetWidgetStyle<FFINLogViewerStyle>("LogViewer")) {}
	SLATE_STYLE_ARGUMENT(FFINLogViewerStyle, Style)
	SLATE_EVENT(SFINLogViewer::FOnNavigateReflection, OnNavigateReflection)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// Begin SWidget
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	// End SWidget

	void UpdateEntries(UFINLog* InLog);
	
private:
	TSharedRef<ITableRow> OnGenerateRow(TSharedRef<FFINLogEntry> Entry, const TSharedRef<class STableViewBase>& ListView);

public:
	FOnNavigateReflection OnNavigateReflectionDelegate;
	
private:
	const FFINLogViewerStyle* Style = nullptr;

	TArray<TSharedRef<FFINLogEntry>> Entries;
	TSharedPtr<SListView<TSharedRef<FFINLogEntry>>> ListView;
};

class SFINLogViewerRow : public SMultiColumnTableRow<TSharedRef<FFINLogEntry>> {
public:
	void Construct(const FTableRowArgs& InArgs, const TSharedRef<STableViewBase>& OwnerTable, const FFINLogViewerStyle* Style, const TSharedRef<FFINLogEntry>& LogEntry);

	TSharedPtr<FFINLogEntry> GetLogEntry() const { return Entry; }
	
protected:
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

	const FSlateBrush* GetVerbosityIcon() const;
	const FSlateFontInfo& GetFontInfo() const;
	const FSlateColor& GetTextColor() const;
	EVisibility GetTextVisibility() const;

public:
	SFINLogViewer::FOnNavigateReflection OnNavigateReflectionDelegate;

private:
	const FFINLogViewerStyle* Style = nullptr;
	TSharedPtr<FFINLogEntry> Entry;
};

class FICSITNETWORKS_API FFINLogTextParser : public IRichTextMarkupParser
{
public:
	FFINLogTextParser();
	
	virtual void Process(TArray<FTextLineParseResults>& Results, const FString& Input, FString& Output) override;

private:
	FRegexPattern ClassPattern;
};

class FICSITNETWORKS_API FFINReflectionReferenceDecorator : public ITextDecorator
{
public:
	FFINReflectionReferenceDecorator(FString InId, const FSlateHyperlinkRun::FOnClick& InNavigateDelegate, const FSlateHyperlinkRun::FOnGetTooltipText& InToolTipTextDelegate = FSlateHyperlinkRun::FOnGetTooltipText(), const FSlateHyperlinkRun::FOnGenerateTooltip& InToolTipDelegate = FSlateHyperlinkRun::FOnGenerateTooltip());
	
	virtual bool Supports( const FTextRunParseResults& RunParseResult, const FString& Text ) const override;
	virtual TSharedRef<ISlateRun> Create(const TSharedRef<FTextLayout>& TextLayout, const FTextRunParseResults& RunParseResult, const FString& OriginalText, const TSharedRef< FString >& InOutModelText, const ISlateStyle* Style) override;

protected:
	FSlateHyperlinkRun::FOnClick NavigateDelegate;

protected:
	FString Id;
	FSlateHyperlinkRun::FOnGetTooltipText ToolTipTextDelegate;
	FSlateHyperlinkRun::FOnGenerateTooltip ToolTipDelegate;
};