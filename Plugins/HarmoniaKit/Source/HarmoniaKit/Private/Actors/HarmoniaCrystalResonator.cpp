// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaCrystalResonator.h"
#include "System/HarmoniaCheckpointSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"

AHarmoniaCrystalResonator::AHarmoniaCrystalResonator()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Î£®Ìä∏ Ïª¥Ìè¨?åÌä∏
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// Î∞õÏπ®?Ä Î©îÏãú
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(SceneRoot);
	BaseMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// ?¨Î¶¨?§ÌÉà Î©îÏãú
	CrystalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrystalMesh"));
	CrystalMesh->SetupAttachment(SceneRoot);
	CrystalMesh->SetCollisionProfileName(TEXT("NoCollision"));

	// ?¨Ïù∏???ºÏù¥??
	ResonanceLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ResonanceLight"));
	ResonanceLight->SetupAttachment(CrystalMesh);
	ResonanceLight->SetIntensity(InactiveLightIntensity);
	ResonanceLight->SetAttenuationRadius(1000.0f);
	ResonanceLight->SetCastShadows(false);

	// ?òÏù¥?ÑÍ????¥Ìéô??
	ResonanceEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ResonanceEffect"));
	ResonanceEffect->SetupAttachment(CrystalMesh);
	ResonanceEffect->SetAutoActivate(false);

	// ?ÅÌò∏?ëÏö© Î≤îÏúÑ
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(SceneRoot);
	InteractionSphere->SetSphereRadius(InteractionRange);
	InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	// ?§Îîî??Ïª¥Ìè¨?åÌä∏
	ResonanceAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("ResonanceAudio"));
	ResonanceAudio->SetupAttachment(CrystalMesh);
	ResonanceAudio->SetAutoActivate(false);

	// Í∏∞Î≥∏ Í∞??§Ï†ï
	CheckpointID = NAME_None;
	CheckpointName = FText::FromString(TEXT("Crystal Resonator"));
	CheckpointDescription = FText::FromString(TEXT("A mystical crystal that resonates with ancient energies."));
	ResonanceFrequency = EHarmoniaResonanceFrequency::Azure;

	// Ï¥àÍ∏∞ ?âÏÉÅ ?§Ï†ï
	InitializeFrequencyColors();
}

void AHarmoniaCrystalResonator::BeginPlay()
{
	Super::BeginPlay();

	// ?úÎ∏å?úÏä§?úÏóê ?±Î°ù
	RegisterToSubsystem();

	// ?úÏûë ???êÎèô ?úÏÑ±??
	if (bStartActivated && HasAuthority())
	{
		SetCheckpointState(EHarmoniaCheckpointState::Activated);
	}

	// Ï¥àÍ∏∞ ÎπÑÏ£º???§Ï†ï
	UpdateVisuals();
}

void AHarmoniaCrystalResonator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Í≥µÎ™Ö ?ÑÏä§ ?®Í≥º
	if (CurrentState == EHarmoniaCheckpointState::Resonating)
	{
		UpdateResonancePulse(DeltaTime);
	}

	// Ï£ºÎ? ?¨Î¶¨?§ÌÉàÍ≥?Í≥µÎ™Ö
	if (CurrentState != EHarmoniaCheckpointState::Inactive)
	{
		UpdateNearbyResonance();
	}
}

void AHarmoniaCrystalResonator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// ?úÎ∏å?úÏä§?úÏóê???±Î°ù ?¥Ï†ú
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UHarmoniaCheckpointSubsystem* CheckpointSubsystem = GameInstance->GetSubsystem<UHarmoniaCheckpointSubsystem>())
		{
			CheckpointSubsystem->UnregisterCheckpoint(CheckpointID);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AHarmoniaCrystalResonator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHarmoniaCrystalResonator, CurrentState);
}

// ============================================================================
// Public Functions
// ============================================================================

bool AHarmoniaCrystalResonator::Activate(APlayerController* Player)
{
	if (!Player || !HasAuthority())
	{
		return false;
	}

	if (CurrentState != EHarmoniaCheckpointState::Inactive)
	{
		// ?¥Î? ?úÏÑ±?îÎê®
		return true;
	}

	SetCheckpointState(EHarmoniaCheckpointState::Activated);
	PlayActivationEffects();

	UE_LOG(LogTemp, Log, TEXT("HarmoniaCrystalResonator: Checkpoint %s activated by player"), *CheckpointID.ToString());
	return true;
}

bool AHarmoniaCrystalResonator::StartResonance(APlayerController* Player)
{
	if (!Player || !HasAuthority())
	{
		return false;
	}

	if (CurrentState == EHarmoniaCheckpointState::Inactive)
	{
		return false;
	}

	if (!ResonatingPlayers.Contains(Player))
	{
		ResonatingPlayers.Add(Player);
	}

	SetCheckpointState(EHarmoniaCheckpointState::Resonating);
	PlayResonanceStartEffects();

	UE_LOG(LogTemp, Log, TEXT("HarmoniaCrystalResonator: Player started resonance at checkpoint %s"), *CheckpointID.ToString());
	return true;
}

void AHarmoniaCrystalResonator::EndResonance(APlayerController* Player)
{
	if (!Player || !HasAuthority())
	{
		return;
	}

	ResonatingPlayers.Remove(Player);

	if (ResonatingPlayers.Num() == 0)
	{
		SetCheckpointState(EHarmoniaCheckpointState::Activated);
		PlayResonanceCompleteEffects();
	}

	UE_LOG(LogTemp, Log, TEXT("HarmoniaCrystalResonator: Player ended resonance at checkpoint %s"), *CheckpointID.ToString());
}

void AHarmoniaCrystalResonator::SetCheckpointState(EHarmoniaCheckpointState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;
	UpdateVisuals();
}

FHarmoniaCheckpointData AHarmoniaCrystalResonator::GetCheckpointData() const
{
	FHarmoniaCheckpointData Data;
	Data.CheckpointID = CheckpointID;
	Data.CheckpointName = CheckpointName;
	Data.CheckpointDescription = CheckpointDescription;
	Data.ResonanceFrequency = ResonanceFrequency;
	Data.Location = GetActorLocation();
	Data.Rotation = GetActorRotation();
	Data.bActivated = IsActivated();
	Data.LastResonanceTime = LastResonanceTime;
	Data.UpgradeLevels = UpgradeLevels;

	return Data;
}

void AHarmoniaCrystalResonator::ApplyCheckpointData(const FHarmoniaCheckpointData& Data)
{
	CheckpointID = Data.CheckpointID;
	CheckpointName = Data.CheckpointName;
	CheckpointDescription = Data.CheckpointDescription;
	ResonanceFrequency = Data.ResonanceFrequency;
	LastResonanceTime = Data.LastResonanceTime;
	UpgradeLevels = Data.UpgradeLevels;

	if (Data.bActivated)
	{
		SetCheckpointState(EHarmoniaCheckpointState::Activated);
	}
	else
	{
		SetCheckpointState(EHarmoniaCheckpointState::Inactive);
	}

	UpdateVisuals();
}

bool AHarmoniaCrystalResonator::IsPlayerResonating(APlayerController* Player) const
{
	return ResonatingPlayers.Contains(Player);
}

FLinearColor AHarmoniaCrystalResonator::GetFrequencyColor() const
{
	const FLinearColor* Color = FrequencyColors.Find(ResonanceFrequency);
	return Color ? *Color : FLinearColor::White;
}

// ============================================================================
// Interaction Interface
// ============================================================================

void AHarmoniaCrystalResonator::OnInteract_Implementation(const FHarmoniaInteractionContext& Context, FHarmoniaInteractionResult& OutResult)
{
	OutResult.bSuccess = false;

	// Context.Interactor?êÏÑú PlayerController Í∞Ä?∏Ïò§Í∏?
	APlayerController* PlayerController = nullptr;
	if (APawn* InteractorPawn = Cast<APawn>(Context.Interactor))
	{
		PlayerController = Cast<APlayerController>(InteractorPawn->GetController());
	}
	else
	{
		PlayerController = Cast<APlayerController>(Context.Interactor);
	}

	if (!PlayerController)
	{
		OutResult.Message = TEXT("Invalid player");
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UHarmoniaCheckpointSubsystem* CheckpointSubsystem = GameInstance->GetSubsystem<UHarmoniaCheckpointSubsystem>();
	if (!CheckpointSubsystem)
	{
		return;
	}

	// ÎπÑÌôú?±Ìôî ?ÅÌÉúÎ©??úÏÑ±??
	if (CurrentState == EHarmoniaCheckpointState::Inactive)
	{
		bool bActivated = CheckpointSubsystem->ActivateCheckpoint(CheckpointID, PlayerController);
		if (bActivated)
		{
			OutResult.bSuccess = true;
			OutResult.Message = FString::Printf(TEXT("Activated %s"), *CheckpointName.ToString());
		}
		return;
	}

	// ?úÏÑ±???ÅÌÉúÎ©?Í≥µÎ™Ö ?úÏûë
	bool bResonanceStarted = CheckpointSubsystem->StartResonance(CheckpointID, PlayerController);
	if (bResonanceStarted)
	{
		OutResult.bSuccess = true;
		OutResult.Message = FString::Printf(TEXT("Resonating with %s..."), *CheckpointName.ToString());
	}
}

// ============================================================================
// Protected Functions
// ============================================================================

void AHarmoniaCrystalResonator::RegisterToSubsystem()
{
	if (!HasAuthority())
	{
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UHarmoniaCheckpointSubsystem* CheckpointSubsystem = GameInstance->GetSubsystem<UHarmoniaCheckpointSubsystem>();
	if (CheckpointSubsystem)
	{
		CheckpointSubsystem->RegisterCheckpoint(this);
		UE_LOG(LogTemp, Log, TEXT("HarmoniaCrystalResonator: Registered checkpoint %s to subsystem"), *CheckpointID.ToString());
	}
}

void AHarmoniaCrystalResonator::UpdateVisuals_Implementation()
{
	if (!ResonanceLight)
	{
		return;
	}

	// Í≥µÎ™Ö Ï£ºÌåå???âÏÉÅ ?ÅÏö©
	FLinearColor FrequencyColor = GetFrequencyColor();
	ResonanceLight->SetLightColor(FrequencyColor);

	// ?ÅÌÉúÎ≥?Î∞úÍ¥ë ?∏Í∏∞
	float TargetIntensity = InactiveLightIntensity;
	bool bShouldActivateEffect = false;

	switch (CurrentState)
	{
	case EHarmoniaCheckpointState::Inactive:
		TargetIntensity = InactiveLightIntensity;
		bShouldActivateEffect = false;
		break;

	case EHarmoniaCheckpointState::Activated:
		TargetIntensity = ActiveLightIntensity;
		bShouldActivateEffect = true;
		break;

	case EHarmoniaCheckpointState::Resonating:
		TargetIntensity = ResonatingLightIntensity;
		bShouldActivateEffect = true;
		break;
	}

	ResonanceLight->SetIntensity(TargetIntensity);

	// ?åÌã∞???®Í≥º
	if (ResonanceEffect)
	{
		if (bShouldActivateEffect && !ResonanceEffect->IsActive())
		{
			ResonanceEffect->Activate(true);
		}
		else if (!bShouldActivateEffect && ResonanceEffect->IsActive())
		{
			ResonanceEffect->Deactivate();
		}
	}

	// ?§Îîî??
	if (ResonanceAudio)
	{
		if (CurrentState == EHarmoniaCheckpointState::Resonating)
		{
			if (!ResonanceAudio->IsPlaying() && ResonanceLoopSound)
			{
				ResonanceAudio->SetSound(ResonanceLoopSound);
				ResonanceAudio->Play();
			}
		}
		else
		{
			if (ResonanceAudio->IsPlaying())
			{
				ResonanceAudio->Stop();
			}
		}
	}
}

void AHarmoniaCrystalResonator::UpdateResonancePulse(float DeltaTime)
{
	if (!ResonanceLight)
	{
		return;
	}

	ResonancePulseTimer += DeltaTime * ResonancePulseSpeed;

	// ?¨Ïù∏???ÑÏä§
	float PulseValue = FMath::Sin(ResonancePulseTimer);
	PulseValue = (PulseValue + 1.0f) * 0.5f; // 0~1 Î≤îÏúÑÎ°??ïÍ∑ú??

	float CurrentIntensity = FMath::Lerp(
		ResonatingLightIntensity * 0.8f,
		ResonatingLightIntensity * 1.2f,
		PulseValue
	);

	ResonanceLight->SetIntensity(CurrentIntensity);
}

void AHarmoniaCrystalResonator::UpdateNearbyResonance_Implementation()
{
	// Blueprint?êÏÑú Íµ¨ÌòÑ Í∞Ä??
	// Í∞ÄÍπåÏö¥ ?§Î•∏ ?úÏÑ±?îÎêú ?¨Î¶¨?§ÌÉà Ï∞æÏïÑ???∞Í≤∞ ?®Í≥º ?úÏãú
}

void AHarmoniaCrystalResonator::ApplyFrequencyEffects_Implementation()
{
	// Blueprint?êÏÑú Íµ¨ÌòÑ Í∞Ä??
	// Í∞?Í≥µÎ™Ö Ï£ºÌåå?òÎ≥Ñ ?πÏàò ?®Í≥º
}

void AHarmoniaCrystalResonator::PlayActivationEffects_Implementation()
{
	// ?úÏÑ±???¨Ïö¥??
	if (ActivationSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ActivationSound, GetActorLocation());
	}

	// Blueprint?êÏÑú Ï∂îÍ? ?®Í≥º Íµ¨ÌòÑ Í∞Ä??
}

void AHarmoniaCrystalResonator::PlayResonanceStartEffects_Implementation()
{
	// Í≥µÎ™Ö ?úÏûë ?¨Ïö¥??
	if (ResonanceStartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ResonanceStartSound, GetActorLocation());
	}

	// Blueprint?êÏÑú Ï∂îÍ? ?®Í≥º Íµ¨ÌòÑ Í∞Ä??
}

void AHarmoniaCrystalResonator::PlayResonanceCompleteEffects_Implementation()
{
	// Í≥µÎ™Ö ?ÑÎ£å ?¨Ïö¥??
	if (ResonanceCompleteSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ResonanceCompleteSound, GetActorLocation());
	}

	// Blueprint?êÏÑú Ï∂îÍ? ?®Í≥º Íµ¨ÌòÑ Í∞Ä??
}

void AHarmoniaCrystalResonator::InitializeFrequencyColors()
{
	// Í≥µÎ™Ö Ï£ºÌåå?òÎ≥Ñ Í∏∞Î≥∏ ?âÏÉÅ ?§Ï†ï
	FrequencyColors.Empty();
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Azure, FLinearColor(0.0f, 0.5f, 1.0f)); // ?∏Î•∏??
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Crimson, FLinearColor(1.0f, 0.0f, 0.2f)); // Î∂âÏ???
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Verdant, FLinearColor(0.0f, 1.0f, 0.3f)); // ?πÏÉâ
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Aurum, FLinearColor(1.0f, 0.8f, 0.0f)); // Í∏àÏÉâ
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Violet, FLinearColor(0.8f, 0.0f, 1.0f)); // Î≥¥Îùº??
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Luminous, FLinearColor(1.0f, 1.0f, 1.0f)); // ?∞ÏÉâ
}

#if WITH_EDITOR
void AHarmoniaCrystalResonator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	// CheckpointName??Î≥ÄÍ≤ΩÎêòÎ©??êÎèô?ºÎ°ú ID ?ùÏÑ±
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaCrystalResonator, CheckpointName))
	{
		if (CheckpointID.IsNone() && !CheckpointName.IsEmpty())
		{
			FString NameString = CheckpointName.ToString();
			NameString.ReplaceInline(TEXT(" "), TEXT("_"));
			CheckpointID = FName(*NameString);
		}
	}

	// ?ÅÌò∏?ëÏö© Î≤îÏúÑ Î≥ÄÍ≤???Íµ¨Ï≤¥ ?¨Í∏∞ ?ÖÎç∞?¥Ìä∏
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaCrystalResonator, InteractionRange))
	{
		if (InteractionSphere)
		{
			InteractionSphere->SetSphereRadius(InteractionRange);
		}
	}

	// Í≥µÎ™Ö Ï£ºÌåå??Î≥ÄÍ≤???ÎπÑÏ£º???ÖÎç∞?¥Ìä∏
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaCrystalResonator, ResonanceFrequency))
	{
		UpdateVisuals();
	}
}
#endif
