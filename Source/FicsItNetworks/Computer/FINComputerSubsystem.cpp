#include "FINComputerSubsystem.h"

#include "FINSubsystemHolder.h"

AFINComputerSubsystem::AFINComputerSubsystem() {
	Input = CreateDefaultSubobject<UInputComponent>("Input");
	Input->BindAction("PrimaryFire", EInputEvent::IE_Pressed, this, &AFINComputerSubsystem::OnPrimaryFirePressed).bConsumeInput = false;
	Input->BindAction("PrimaryFire", EInputEvent::IE_Released, this, &AFINComputerSubsystem::OnPrimaryFireReleased).bConsumeInput = false;
	Input->BindAction("SecondaryFire", EInputEvent::IE_Pressed, this, &AFINComputerSubsystem::OnSecondaryFirePressed).bConsumeInput = false;
	Input->BindAction("SecondaryFire", EInputEvent::IE_Released, this, &AFINComputerSubsystem::OnSecondaryFireReleased).bConsumeInput = false;

	SetActorTickEnabled(true);
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFINComputerSubsystem::BeginPlay() {
	Super::BeginPlay();
}

void AFINComputerSubsystem::Tick(float dt) {
	Super::Tick(dt);
	this->GetWorld()->GetFirstPlayerController()->PushInputComponent(Input);
}

bool AFINComputerSubsystem::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerSubsystem::OnPrimaryFirePressed() {
	AController* controller = GetWorld()->GetFirstPlayerController();
	if (controller) {
		AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(controller->GetCharacter());
		if (character) {
			UWidgetInteractionComponent* ScreenInteraction = Cast<UWidgetInteractionComponent>(character->GetComponentByClass(UWidgetInteractionComponent::StaticClass()));
			if (ScreenInteraction) ScreenInteraction->PressPointerKey(EKeys::LeftMouseButton);
		}
	}
}

void AFINComputerSubsystem::OnPrimaryFireReleased() {
	AController* controller = GetWorld()->GetFirstPlayerController();
	if (controller) {
		AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(controller->GetCharacter());
		if (character) {
			UWidgetInteractionComponent* ScreenInteraction = Cast<UWidgetInteractionComponent>(character->GetComponentByClass(UWidgetInteractionComponent::StaticClass()));
			if (ScreenInteraction) ScreenInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
		}
	}
}

void AFINComputerSubsystem::OnSecondaryFirePressed() {
	AController* controller = GetWorld()->GetFirstPlayerController();
	if (controller) {
		AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(controller->GetCharacter());
		if (character) {
			UWidgetInteractionComponent* ScreenInteraction = Cast<UWidgetInteractionComponent>(character->GetComponentByClass(UWidgetInteractionComponent::StaticClass()));
			if (ScreenInteraction) ScreenInteraction->PressPointerKey(EKeys::RightMouseButton);
		}
	}
}

void AFINComputerSubsystem::OnSecondaryFireReleased() {
	AController* controller = GetWorld()->GetFirstPlayerController();
	if (controller) {
		AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(controller->GetCharacter());
		if (character) {
			UWidgetInteractionComponent* ScreenInteraction = Cast<UWidgetInteractionComponent>(character->GetComponentByClass(UWidgetInteractionComponent::StaticClass()));
			if (ScreenInteraction) ScreenInteraction->ReleasePointerKey(EKeys::RightMouseButton);
		}
	}
}

AFINComputerSubsystem* AFINComputerSubsystem::GetComputerSubsystem(UObject* WorldContext) {
	return GetSubsystemHolder<UFINSubsystemHolder>(WorldContext)->ComputerSubsystem;
}

UWidgetInteractionComponent* AFINComputerSubsystem::AttachWidgetInteractionToPlayer(AFGCharacterPlayer* character) {
	if (ScreenInteraction) ScreenInteraction->UnregisterComponent();
	ScreenInteraction = NewObject<UWidgetInteractionComponent>(character);
	ScreenInteraction->InteractionSource = EWidgetInteractionSource::World;
	UCameraComponent* cam = Cast<UCameraComponent>(character->GetComponentByClass(UCameraComponent::StaticClass()));
	ScreenInteraction->InteractionDistance = 10000.0;
	ScreenInteraction->VirtualUserIndex = 1;
	ScreenInteraction->RegisterComponent();
	ScreenInteraction->AttachToComponent(cam, FAttachmentTransformRules::KeepRelativeTransform);
	return ScreenInteraction;
}