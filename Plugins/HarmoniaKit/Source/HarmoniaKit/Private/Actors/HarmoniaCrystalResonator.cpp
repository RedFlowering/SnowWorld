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

	// Root component
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// Base mesh
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(SceneRoot);
	BaseMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// Crystal mesh
	CrystalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrystalMesh"));
	CrystalMesh->SetupAttachment(SceneRoot);
	CrystalMesh->SetCollisionProfileName(TEXT("NoCollision"));

	// Point light
	ResonanceLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ResonanceLight"));
	ResonanceLight->SetupAttachment(CrystalMesh);
	ResonanceLight->SetIntensity(InactiveLightIntensity);
	ResonanceLight->SetAttenuationRadius(1000.0f);
	ResonanceLight->SetCastShadows(false);

	// Niagara effect
	ResonanceEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ResonanceEffect"));
	ResonanceEffect->SetupAttachment(CrystalMesh);
	ResonanceEffect->SetAutoActivate(false);

	// Interaction range sphere
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(SceneRoot);
	InteractionSphere->SetSphereRadius(InteractionRange);
	InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	// Audio component
	ResonanceAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("ResonanceAudio"));
	ResonanceAudio->SetupAttachment(CrystalMesh);
	ResonanceAudio->SetAutoActivate(false);

	// Default value settings
	CheckpointID = NAME_None;
	CheckpointName = FText::FromString(TEXT("Crystal Resonator"));
	CheckpointDescription = FText::FromString(TEXT("A mystical crystal that resonates with ancient energies."));
	ResonanceFrequency = EHarmoniaResonanceFrequency::Azure;

	// Initial state setup
	InitializeFrequencyColors();
}

void AHarmoniaCrystalResonator::BeginPlay()
{
	Super::BeginPlay();

	// Register to subsystem
	RegisterToSubsystem();

	// Auto-activate on start
	if (bStartActivated && HasAuthority())
	{
		SetCheckpointState(EHarmoniaCheckpointState::Activated);
	}

	// Initial visual setup
	UpdateVisuals();
}

void AHarmoniaCrystalResonator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Resonance pulse effect
	if (CurrentState == EHarmoniaCheckpointState::Resonating)
	{
		UpdateResonancePulse(DeltaTime);
	}

	// Resonate with nearby crystals
	if (CurrentState != EHarmoniaCheckpointState::Inactive)
	{
		UpdateNearbyResonance();
	}
}

void AHarmoniaCrystalResonator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from subsystem
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
		// Already in the desired state
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

	// Get PlayerController from Context.Interactor
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

	// Deactivated state transition
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

	// Activated state - start resonance
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

	// Apply resonance frequency color
	FLinearColor FrequencyColor = GetFrequencyColor();
	ResonanceLight->SetLightColor(FrequencyColor);

	// Emission intensity based on state
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

	// Particle effect
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

	// Audio
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

	// Point light pulse
	float PulseValue = FMath::Sin(ResonancePulseTimer);
	PulseValue = (PulseValue + 1.0f) * 0.5f; // Normalize to 0~1 range

	float CurrentIntensity = FMath::Lerp(
		ResonatingLightIntensity * 0.8f,
		ResonatingLightIntensity * 1.2f,
		PulseValue
	);

	ResonanceLight->SetIntensity(CurrentIntensity);
}

void AHarmoniaCrystalResonator::UpdateNearbyResonance_Implementation()
{
	// Can be implemented in Blueprint
	// Find nearby activated crystals and display connection effect
}

void AHarmoniaCrystalResonator::ApplyFrequencyEffects_Implementation()
{
	// Can be implemented in Blueprint
	// Passive effects per resonance frequency
}

void AHarmoniaCrystalResonator::PlayActivationEffects_Implementation()
{
	// Activation sound
	if (ActivationSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ActivationSound, GetActorLocation());
	}

	// Can implement additional effects in Blueprint
}

void AHarmoniaCrystalResonator::PlayResonanceStartEffects_Implementation()
{
	// Resonance start sound
	if (ResonanceStartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ResonanceStartSound, GetActorLocation());
	}

	// Can implement additional effects in Blueprint
}

void AHarmoniaCrystalResonator::PlayResonanceCompleteEffects_Implementation()
{
	// Resonance end sound
	if (ResonanceCompleteSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ResonanceCompleteSound, GetActorLocation());
	}

	// Can implement additional effects in Blueprint
}

void AHarmoniaCrystalResonator::InitializeFrequencyColors()
{
	// Default color settings per resonance frequency
	FrequencyColors.Empty();
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Azure, FLinearColor(0.0f, 0.5f, 1.0f)); // Azure blue
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Crimson, FLinearColor(1.0f, 0.0f, 0.2f)); // 붉�???
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Verdant, FLinearColor(0.0f, 1.0f, 0.3f)); // Green
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Aurum, FLinearColor(1.0f, 0.8f, 0.0f)); // 금색
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Violet, FLinearColor(0.8f, 0.0f, 1.0f)); // 보라??
	FrequencyColors.Add(EHarmoniaResonanceFrequency::Luminous, FLinearColor(1.0f, 1.0f, 1.0f)); // White
}

#if WITH_EDITOR
void AHarmoniaCrystalResonator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.GetPropertyName();

	// Auto-generate ID when CheckpointName changes
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaCrystalResonator, CheckpointName))
	{
		if (CheckpointID.IsNone() && !CheckpointName.IsEmpty())
		{
			FString NameString = CheckpointName.ToString();
			NameString.ReplaceInline(TEXT(" "), TEXT("_"));
			CheckpointID = FName(*NameString);
		}
	}

	// Update sphere size when interaction range changes
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaCrystalResonator, InteractionRange))
	{
		if (InteractionSphere)
		{
			InteractionSphere->SetSphereRadius(InteractionRange);
		}
	}

	// Update visuals when resonance frequency changes
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AHarmoniaCrystalResonator, ResonanceFrequency))
	{
		UpdateVisuals();
	}
}
#endif
