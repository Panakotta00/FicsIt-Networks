#include "FicsItNetworksComputer.h"

#include "FGCharacterPlayer.h"
#include "FGDismantleInterface.h"
#include "FGGameMode.h"
#include "FINComputerRCO.h"
#include "FINComputerSubsystem.h"
#include "Patching/NativeHookManager.h"
#include "UObject/CoreRedirects.h"

#define LOCTEXT_NAMESPACE "FFicsItNetworksComputerModule"

DEFINE_LOG_CATEGORY(LogFicsItNetworksComputer);

FDateTime FFicsItNetworksComputerModule::GameStart;

void FFicsItNetworksComputerModule::StartupModule() {
	TArray<FCoreRedirect> redirects;

	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINFSAlways"), TEXT("/Script/FicsItNetworksComputer.FINFSAlways")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerSubsystem"), TEXT("/Script/FicsItNetworksComputer.FINComputerSubsystem")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUSignPrefabWidget"), TEXT("/Script/FicsItNetworksComputer.FINGPUSignPrefabWidget")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerRCO"), TEXT("/Script/FicsItNetworksComputer.FINComputerRCO")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerFloppyDesc"), TEXT("/Script/FicsItNetworksComputer.FINComputerFloppyDesc")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerEEPROMDesc"), TEXT("/Script/FicsItNetworksComputer.FINComputerEEPROMDesc")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerCaseWidget"), TEXT("/Script/FicsItNetworksComputer.FINComputerCaseWidget")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINCaseEEPROMUpdateDelegate"), TEXT("/Script/FicsItNetworksComputer.FINCaseEEPROMUpdateDelegate")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINCaseFloppyUpdateDelegate"), TEXT("/Script/FicsItNetworksComputer.FINCaseFloppyUpdateDelegate")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerCase"), TEXT("/Script/FicsItNetworksComputer.FINComputerCase")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINScreenInterface"), TEXT("/Script/FicsItNetworksComputer.FINScreenInterface")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUInterface"), TEXT("/Script/FicsItNetworksComputer.FINGPUInterface")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerProcessor"), TEXT("/Script/FicsItNetworksComputer.FINComputerProcessor")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerModule"), TEXT("/Script/FicsItNetworksComputer.FINComputerModule")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerMemory"), TEXT("/Script/FicsItNetworksComputer.FINComputerMemory")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINDriveHolderDriveUpdate"), TEXT("/Script/FicsItNetworksComputer.FINDriveHolderDriveUpdate")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINDriveHolderLockedUpdateDelegate"), TEXT("/Script/FicsItNetworksComputer.FINDriveHolderLockedUpdateDelegate")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerDriveHolder"), TEXT("/Script/FicsItNetworksComputer.FINComputerDriveHolder")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerDriveDesc"), TEXT("/Script/FicsItNetworksComputer.FINComputerDriveDesc")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINPciDeviceInterface"), TEXT("/Script/FicsItNetworksComputer.FINPciDeviceInterface")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINInternetCardHttpRequestFuture"), TEXT("/Script/FicsItNetworksComputer.FINInternetCardHttpRequestFuture")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINInternetCard"), TEXT("/Script/FicsItNetworksComputer.FINInternetCard")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.ScreenWidgetUpdate"), TEXT("/Script/FicsItNetworksComputer.ScreenWidgetUpdate")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.ScreenGPUUpdate"), TEXT("/Script/FicsItNetworksComputer.ScreenGPUUpdate")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerScreen"), TEXT("/Script/FicsItNetworksComputer.FINComputerScreen")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerNetworkCard"), TEXT("/Script/FicsItNetworksComputer.FINComputerNetworkCard")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2WidgetStyle"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2WidgetStyle")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2WidgetStyleContainer"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2WidgetStyleContainer")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2DrawCallType"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2DrawCallType")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2DrawContext"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2DrawContext")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2DrawCall"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2DrawCall")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2DC_PushTransform"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2DC_PushTransform")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2DC_PushLayout"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2DC_PushLayout")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2DC_PopGeometry"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2DC_PopGeometry")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2DC_PushClipRect"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2DC_PushClipRect")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2DC_PushClipPolygon"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2DC_PushClipPolygon")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2DC_PopClip"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2DC_PopClip")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2DC_Lines"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2DC_Lines")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2DC_Text"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2DC_Text")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2DC_Spline"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2DC_Spline")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2DC_Bezier"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2DC_Bezier")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2DC_Box"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2DC_Box")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2CursorEvent"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2CursorEvent")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2WheelEvent"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2WheelEvent")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2KeyEvent"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2KeyEvent")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT2KeyCharEvent"), TEXT("/Script/FicsItNetworksComputer.FINGPUT2KeyCharEvent")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerGPUT2"), TEXT("/Script/FicsItNetworksComputer.FINComputerGPUT2")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.ScreenCursorEventHandler"), TEXT("/Script/FicsItNetworksComputer.ScreenCursorEventHandler")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.ScreenKeyEventHandler"), TEXT("/Script/FicsItNetworksComputer.ScreenKeyEventHandler")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.ScreenKeyCharEventHandler"), TEXT("/Script/FicsItNetworksComputer.ScreenKeyCharEventHandler")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT1BufferPixel"), TEXT("/Script/FicsItNetworksComputer.FINGPUT1BufferPixel")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT1TextBlendingMethod"), TEXT("/Script/FicsItNetworksComputer.FINGPUT1TextBlendingMethod")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT1ColorBlendingMethod"), TEXT("/Script/FicsItNetworksComputer.FINGPUT1ColorBlendingMethod")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUT1Buffer"), TEXT("/Script/FicsItNetworksComputer.FINGPUT1Buffer")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerGPUT1"), TEXT("/Script/FicsItNetworksComputer.FINComputerGPUT1")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINComputerGPU"), TEXT("/Script/FicsItNetworksComputer.FINComputerGPU")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINScreenWidget"), TEXT("/Script/FicsItNetworksComputer.FINScreenWidget")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINGPUWidgetSign"), TEXT("/Script/FicsItNetworksComputer.FINGPUWidgetSign")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINKernelState"), TEXT("/Script/FicsItNetworksComputer.FINKernelState")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINKernelCrash"), TEXT("/Script/FicsItNetworksComputer.FINKernelCrash")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINKernelListener"), TEXT("/Script/FicsItNetworksComputer.FINKernelListener")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINKernelSystem"), TEXT("/Script/FicsItNetworksComputer.FINKernelSystem")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINKernelProcessor"), TEXT("/Script/FicsItNetworksComputer.FINKernelProcessor")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINStateEEPROM"), TEXT("/Script/FicsItNetworksComputer.FINStateEEPROM")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINEEPROMUpdateDelegate"), TEXT("/Script/FicsItNetworksComputer.FINEEPROMUpdateDelegate")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINKernelNetworkController"), TEXT("/Script/FicsItNetworksComputer.FINKernelNetworkController")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINFileSystemState"), TEXT("/Script/FicsItNetworksComputer.FINFileSystemState")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FileSystemNodeIndex"), TEXT("/Script/FicsItNetworksComputer.FileSystemNodeIndex")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FileSystemNode"), TEXT("/Script/FicsItNetworksComputer.FileSystemNode")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FileSystemSerializationInfo"), TEXT("/Script/FicsItNetworksComputer.FileSystemSerializationInfo")});
	redirects.Add(FCoreRedirect{ECoreRedirectFlags::Type_AllMask, TEXT("/Script/FicsItNetworks.FINKernelAudioController"), TEXT("/Script/FicsItNetworksComputer.FINKernelAudioController")});

	FCoreRedirects::AddRedirectList(redirects, "FicsItNetworksCircuit");

	CodersFileSystem::Tests::TestPath();

	GameStart = FDateTime::Now();

	FCoreDelegates::OnPostEngineInit.AddStatic([]() {
#if !WITH_EDITOR
		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGGameMode::PostLogin, (void*)GetDefault<AFGGameMode>(), [](AFGGameMode* gm, APlayerController* pc) {
			if (gm->HasAuthority() && !gm->IsMainMenuGameMode()) {
				gm->RegisterRemoteCallObjectClass(UFINComputerRCO::StaticClass());
			}
		});

		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGCharacterPlayer::BeginPlay, (void*)GetDefault<AFGCharacterPlayer>(), [](AActor* self) {
			AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(self);
			if (character) {
				AFINComputerSubsystem* SubSys = AFINComputerSubsystem::GetComputerSubsystem(self->GetWorld());
				if (SubSys) SubSys->AttachWidgetInteractionToPlayer(character);
			}
		});

		SUBSCRIBE_METHOD_VIRTUAL_AFTER(AFGCharacterPlayer::EndPlay, (void*)GetDefault<AFGCharacterPlayer>(), [](AActor* self, EEndPlayReason::Type Reason) {
			AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(self);
			if (character) {
				AFINComputerSubsystem* SubSys = AFINComputerSubsystem::GetComputerSubsystem(self->GetWorld());
				if (SubSys) SubSys->DetachWidgetInteractionToPlayer(character);
			}
		});
#endif
	});
}

void FFicsItNetworksComputerModule::ShutdownModule() {}

UFINComputerGameWorldModule::UFINComputerGameWorldModule() {
	ModSubsystems.Add(AFINComputerSubsystem::StaticClass());
}

UFINComputerGameInstanceModule::UFINComputerGameInstanceModule() {
	//RemoteCallObjects.Add(UFINComputerRCO::StaticClass());
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FFicsItNetworksComputerModule, FicsItNetworksComputer)