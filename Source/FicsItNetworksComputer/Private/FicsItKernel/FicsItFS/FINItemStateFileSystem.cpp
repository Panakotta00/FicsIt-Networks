#include "FicsItKernel/FicsItFS/FINItemStateFileSystem.h"

#include "CommandLine.h"
#include "FINComputerSubsystem.h"
#include "FINFileSystemSubsystem.h"
#include "FileSystemSerializationInfo.h"
#include "StructuredLog.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "SViewport.h"
#include "Engine/Engine.h"

FIN_STRUCT_IMPLEMENT_INTERFACE(FFINItemStateFileSystem, FFINLabelContainerInterface)

int AskForDiskOrSave(FString Name) {
	if (FParse::Param(FCommandLine::Get(), TEXT("FINOverwriteFS"))) {
		return FIN_FS_AlwaysOverride;
	}
	if (FParse::Param(FCommandLine::Get(), TEXT("FINKeepFS"))) {
		return FIN_FS_AlwaysKeep;
	}
#if UE_SERVER
	UE_LOGFMT(LogFicsItFileSystem, Fatal,
		"A FileSystem Content difference between save file and host has been found for UUID: {uuid}!"
		"Please add the '-FINOverwriteFS' or '-FINKeepFS' command line options to define what should happen in such case.",
		Name);
#endif

	switch (AFINComputerSubsystem::GetFSAlways()) {
		case FIN_FS_AlwaysOverride:
			return FIFS_OVERRIDE_CHANGES;
		case FIN_FS_AlwaysKeep:
			return FIFS_KEEP_CHANGES;
		default: ;
	}

	int KeepDisk = 0;
	bool bDontAskAgain = false;
	TSharedRef<SWindow> Window = SNew(SWindow)
		.HasCloseButton(false)
		.ActivationPolicy(EWindowActivationPolicy::Always)
		.FocusWhenFirstShown(true)
		.SizingRule(ESizingRule::Autosized)
		.Title(FText::FromString(FString::Printf(TEXT("FileSystem Content difference in FS '%s' found!"), *Name)))
		.Content()[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.FillHeight(1)[
				SNew(SBox)
				.MinDesiredHeight(100)
				.MinDesiredWidth(100)[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("It appears as if the disk filesystem of a drive "
						"or floppy of yours is different from the content that is saved in the save file you are currently trying to load.\n\n"
						"Do you want to keep the content on the disk or overwrite it with the content in the save file?\n\n"
						"FileSystem UUID:'%s'"), *Name)))
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)[
					SNew(SButton)
					.Text(FText::FromString("Keep changes on disk"))
					.OnClicked_Lambda([&KeepDisk, &Window]() {
						KeepDisk = FIFS_KEEP_CHANGES;
						Window->RequestDestroyWindow();
						return FReply::Handled();
					})]
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Overwrite with Save")))
					.OnClicked_Lambda([&KeepDisk, &Window]() {
						KeepDisk = FIFS_OVERRIDE_CHANGES;
						Window->RequestDestroyWindow();
						return FReply::Handled();
					})
				]
				+SHorizontalBox::Slot()
				.FillWidth(1)
				.HAlign(HAlign_Right)[
					SNew(SCheckBox)
					.Content()[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Don't ask again")))
					]
					.IsChecked_Lambda([&]() {
						return bDontAskAgain ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([&](ECheckBoxState State) {
						bDontAskAgain = State == ECheckBoxState::Checked;
					})
				]
			]
		];
	FSlateApplication::Get().AddModalWindow(Window, GEngine->GetGameViewportWidget());

	if (bDontAskAgain) {
		switch (KeepDisk) {
			case FIFS_KEEP_CHANGES:
				AFINComputerSubsystem::SetFSAlways(FIN_FS_AlwaysKeep);
			break;
			case FIFS_OVERRIDE_CHANGES:
				AFINComputerSubsystem::SetFSAlways(FIN_FS_AlwaysOverride);
			break;
			default: ;
		}
	}

	return KeepDisk;
}

bool FFINItemStateFileSystem::Serialize(FStructuredArchive::FSlot Slot) {
	StaticStruct()->SerializeBin(Slot, this);

	if (!Slot.GetUnderlyingArchive().IsSaveGame()) return true;

	int32 OldCapacity = Capacity;
	Capacity = 0;
	TSharedPtr<CodersFileSystem::Device> SerializeDevice = AFINFileSystemSubsystem::GetDevice(ID, false, true);
	Capacity = OldCapacity;

	auto Record = Slot.EnterRecord();
	FStructuredArchive::FSlot RootNode = Record.EnterField(SA_FIELD_NAME(TEXT("RootNode")));

	int KeepDisk = -1;
	CodersFileSystem::SerializePath(SerializeDevice.ToSharedRef(), RootNode.EnterRecord(), "/", ID.ToString(), KeepDisk, &AskForDiskOrSave);

	return true;
}
