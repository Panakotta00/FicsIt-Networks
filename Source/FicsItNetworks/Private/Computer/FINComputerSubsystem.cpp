#include "Computer/FINComputerSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"
#include "FGCharacterPlayer.h"

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

	SetReplicates(true);
	bAlwaysRelevant = true;
}

void AFINComputerSubsystem::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
}

void AFINComputerSubsystem::BeginPlay() {
	Super::BeginPlay();

	TArray<AActor*> FoundCharacters;
	UGameplayStatics::GetAllActorsOfClass(this, AFGCharacterPlayer::StaticClass(), FoundCharacters);
	for (AActor* Character : FoundCharacters) AttachWidgetInteractionToPlayer(Cast<AFGCharacterPlayer>(Character));
}

void AFINComputerSubsystem::Tick(float dt) {
	Super::Tick(dt);
	this->GetWorld()->GetFirstPlayerController()->PushInputComponent(Input);
}

bool AFINComputerSubsystem::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerSubsystem::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	Version = FINLatestVersion;
}

void AFINComputerSubsystem::OnPrimaryFirePressed() {
	AController* controller = GetWorld()->GetFirstPlayerController();
	if (controller) {
		AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(controller->GetCharacter());
		if (character) {
			UWidgetInteractionComponent** Comp = ScreenInteraction.Find(character);
			if (Comp && IsValid(*Comp)) (*Comp)->PressPointerKey(EKeys::LeftMouseButton);
		}
	}
}

void AFINComputerSubsystem::OnPrimaryFireReleased() {
	AController* controller = GetWorld()->GetFirstPlayerController();
	if (controller) {
		AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(controller->GetCharacter());
		if (character) {
			UWidgetInteractionComponent** Comp = ScreenInteraction.Find(character);
			if (Comp && IsValid(*Comp)) (*Comp)->ReleasePointerKey(EKeys::LeftMouseButton);
		}
	}
}

void AFINComputerSubsystem::OnSecondaryFirePressed() {
	AController* controller = GetWorld()->GetFirstPlayerController();
	if (controller) {
		AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(controller->GetCharacter());
		if (character) {
			UWidgetInteractionComponent** Comp = ScreenInteraction.Find(character);
			if (Comp && IsValid(*Comp)) (*Comp)->PressPointerKey(EKeys::RightMouseButton);
		}
	}
}

void AFINComputerSubsystem::OnSecondaryFireReleased() {
	AController* controller = GetWorld()->GetFirstPlayerController();
	if (controller) {
		AFGCharacterPlayer* character = Cast<AFGCharacterPlayer>(controller->GetCharacter());
		if (character) {
			UWidgetInteractionComponent** Comp = ScreenInteraction.Find(character);
			if (Comp && IsValid(*Comp)) (*Comp)->ReleasePointerKey(EKeys::RightMouseButton);
		}
	}
}

AFINComputerSubsystem* AFINComputerSubsystem::GetComputerSubsystem(UObject* WorldContext) {
#if WITH_EDITOR
	return nullptr;
#endif
	UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFINComputerSubsystem>();
}

void AFINComputerSubsystem::AttachWidgetInteractionToPlayer(AFGCharacterPlayer* character) {
	if (!IsValid(character) || !character->GetController() || !character->GetController()->IsLocalPlayerController()) return;
	DetachWidgetInteractionToPlayer(character);
	UWidgetInteractionComponent* Comp = NewObject<UWidgetInteractionComponent>(character);
	Comp->InteractionSource = EWidgetInteractionSource::World;
	UCameraComponent* cam = Cast<UCameraComponent>(character->GetComponentByClass(UCameraComponent::StaticClass()));
	Comp->InteractionDistance = 10000.0;
	Comp->VirtualUserIndex = VirtualUserNum++;
	Comp->RegisterComponent();
	Comp->AttachToComponent(cam, FAttachmentTransformRules::KeepRelativeTransform);
	ScreenInteraction.Add(character, Comp);
}

void AFINComputerSubsystem::DetachWidgetInteractionToPlayer(AFGCharacterPlayer* character) {
	if (!IsValid(character)) return;
	UWidgetInteractionComponent** Comp = ScreenInteraction.Find(character);
	if (Comp) {
		(*Comp)->UnregisterComponent();
		ScreenInteraction.Remove(character);
	}
}
