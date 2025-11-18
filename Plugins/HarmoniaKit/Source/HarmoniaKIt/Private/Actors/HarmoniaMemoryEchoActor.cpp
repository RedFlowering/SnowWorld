// Copyright Epic Games, Inc. All Rights Reserved.

#include "Actors/HarmoniaMemoryEchoActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Data/HarmoniaDeathPenaltyConfigAsset.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

AHarmoniaMemoryEchoActor::AHarmoniaMemoryEchoActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true; // Important for players to see their memory echo

	// Root component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);

	// Interaction sphere
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(150.0f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Resonance sphere
	ResonanceSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ResonanceSphere"));
	ResonanceSphere->SetupAttachment(RootComponent);
	ResonanceSphere->SetSphereRadius(2000.0f); // 20 meters
	ResonanceSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ResonanceSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	ResonanceSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Visual effect
	EffectComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("EffectComponent"));
	EffectComponent->SetupAttachment(RootComponent);
	EffectComponent->bAutoActivate = true;

	// Ambient sound
	AmbientSound = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientSound"));
	AmbientSound->SetupAttachment(RootComponent);
	AmbientSound->bAutoActivate = true;

	// Initialize
	TimeSinceCreation = 0.0f;
	TotalDecayPercentage = 0.0f;
	LastDecayTime = 0.0f;
}

void AHarmoniaMemoryEchoActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// Setup interaction callbacks
		InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AHarmoniaMemoryEchoActor::OnInteractionBeginOverlap);
		InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AHarmoniaMemoryEchoActor::OnInteractionEndOverlap);

		// Start time decay timer if configured
		if (DeathPenaltyConfig && DeathPenaltyConfig->Config.TimeDecayConfig.bEnableTimeDecay)
		{
			const float DecayInterval = DeathPenaltyConfig->Config.TimeDecayConfig.DecayInterval;
			GetWorldTimerManager().SetTimer(
				DecayTimerHandle,
				[this]()
				{
					if (DeathPenaltyConfig)
					{
						const float DecayPercent = DeathPenaltyConfig->Config.TimeDecayConfig.DecayPercentage;
						ApplyDecayToCurrencies(DecayPercent);
					}
				},
				DecayInterval,
				true,
				DecayInterval
			);
		}

		// Start memory resonance timer if configured
		if (DeathPenaltyConfig && DeathPenaltyConfig->Config.MemoryResonanceConfig.bEnableMemoryResonance)
		{
			GetWorldTimerManager().SetTimer(
				ResonanceTimerHandle,
				this,
				&AHarmoniaMemoryEchoActor::UpdateMemoryResonance,
				1.0f, // Update every second
				true,
				0.5f // Initial delay
			);
		}
	}

	UpdateVisualEffects();
	UpdateAudio();
}

void AHarmoniaMemoryEchoActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		TimeSinceCreation += DeltaTime;
	}

	// Update visual effects on all clients
	if (GetWorld()->GetTimeSeconds() - LastDecayTime > 1.0f)
	{
		UpdateVisualEffects();
		LastDecayTime = GetWorld()->GetTimeSeconds();
	}
}

void AHarmoniaMemoryEchoActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHarmoniaMemoryEchoActor, StoredCurrencies);
	DOREPLIFETIME(AHarmoniaMemoryEchoActor, OwningPlayerController);
	DOREPLIFETIME(AHarmoniaMemoryEchoActor, OwningPlayerStateId);
	DOREPLIFETIME(AHarmoniaMemoryEchoActor, TimeSinceCreation);
	DOREPLIFETIME(AHarmoniaMemoryEchoActor, TotalDecayPercentage);
}

void AHarmoniaMemoryEchoActor::Initialize(const TArray<FHarmoniaCurrencyAmount>& Currencies, APlayerController* OwnerController, UHarmoniaDeathPenaltyConfigAsset* Config)
{
	if (!HasAuthority())
	{
		return;
	}

	StoredCurrencies = Currencies;
	OwningPlayerController = OwnerController;
	DeathPenaltyConfig = Config;

	if (OwnerController && OwnerController->PlayerState)
	{
		OwningPlayerStateId = OwnerController->PlayerState->GetUniqueId().ToString();
	}

	// Update resonance sphere radius from config
	if (Config && Config->Config.MemoryResonanceConfig.bEnableMemoryResonance)
	{
		ResonanceSphere->SetSphereRadius(Config->Config.MemoryResonanceConfig.ResonanceRadius);
	}

	UpdateVisualEffects();
	UpdateAudio();
}

int32 AHarmoniaMemoryEchoActor::GetCurrencyAmount(EHarmoniaCurrencyType CurrencyType) const
{
	for (const FHarmoniaCurrencyAmount& Currency : StoredCurrencies)
	{
		if (Currency.CurrencyType == CurrencyType)
		{
			return Currency.Amount;
		}
	}
	return 0;
}

bool AHarmoniaMemoryEchoActor::BelongsToPlayer(APlayerController* PlayerController) const
{
	if (!PlayerController || !PlayerController->PlayerState)
	{
		return false;
	}

	// Check by player state ID
	const FString PlayerStateId = PlayerController->PlayerState->GetUniqueId().ToString();
	return OwningPlayerStateId == PlayerStateId;
}

bool AHarmoniaMemoryEchoActor::CanPlayerInteract(APlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return false;
	}

	// Owner can always interact
	if (BelongsToPlayer(PlayerController))
	{
		return true;
	}

	// Check if other players can steal
	if (DeathPenaltyConfig && DeathPenaltyConfig->Config.bAllowOtherPlayerRecovery)
	{
		return true;
	}

	return false;
}

TArray<FHarmoniaCurrencyAmount> AHarmoniaMemoryEchoActor::RecoverCurrencies(APlayerController* PlayerController)
{
	TArray<FHarmoniaCurrencyAmount> RecoveredCurrencies;

	if (!HasAuthority() || !PlayerController)
	{
		return RecoveredCurrencies;
	}

	const bool bIsOwner = BelongsToPlayer(PlayerController);

	if (!bIsOwner && (!DeathPenaltyConfig || !DeathPenaltyConfig->Config.bAllowOtherPlayerRecovery))
	{
		return RecoveredCurrencies;
	}

	// Calculate recovery percentage
	float RecoveryPercentage = 1.0f;

	if (!bIsOwner && DeathPenaltyConfig)
	{
		// Other player steals a percentage
		RecoveryPercentage = DeathPenaltyConfig->Config.OtherPlayerRecoveryPercentage;
	}
	else if (bIsOwner && IsFastRecovery())
	{
		// Owner gets fast recovery bonus
		RecoveryPercentage = GetFastRecoveryMultiplier();
	}

	// Recover currencies
	for (FHarmoniaCurrencyAmount& Currency : StoredCurrencies)
	{
		if (Currency.Amount > 0)
		{
			const int32 RecoveredAmount = FMath::FloorToInt32(Currency.Amount * RecoveryPercentage);

			if (RecoveredAmount > 0)
			{
				RecoveredCurrencies.Add(FHarmoniaCurrencyAmount(Currency.CurrencyType, RecoveredAmount));

				if (bIsOwner)
				{
					// Owner recovers all
					Currency.Amount = 0;
				}
				else
				{
					// Other player steals percentage
					Currency.Amount -= RecoveredAmount;
				}
			}
		}
	}

	// Check if all currencies exhausted
	bool bHasAnyCurrency = false;
	for (const FHarmoniaCurrencyAmount& Currency : StoredCurrencies)
	{
		if (Currency.Amount > 0)
		{
			bHasAnyCurrency = true;
			break;
		}
	}

	if (!bHasAnyCurrency)
	{
		HandleCurrenciesExhausted();
	}
	else
	{
		UpdateVisualEffects();
	}

	return RecoveredCurrencies;
}

bool AHarmoniaMemoryEchoActor::IsFastRecovery() const
{
	if (!DeathPenaltyConfig)
	{
		return false;
	}

	return TimeSinceCreation <= DeathPenaltyConfig->Config.TimeDecayConfig.FastRecoveryWindow;
}

float AHarmoniaMemoryEchoActor::GetFastRecoveryMultiplier() const
{
	if (!IsFastRecovery() || !DeathPenaltyConfig)
	{
		return 1.0f;
	}

	return 1.0f + DeathPenaltyConfig->Config.TimeDecayConfig.FastRecoveryBonus;
}

void AHarmoniaMemoryEchoActor::ApplyTimeDecay(float DeltaTime)
{
	if (!HasAuthority() || !DeathPenaltyConfig)
	{
		return;
	}

	const FHarmoniaTimeDecayConfig& DecayConfig = DeathPenaltyConfig->Config.TimeDecayConfig;

	if (!DecayConfig.bEnableTimeDecay)
	{
		return;
	}

	// Check if decay has started
	if (TimeSinceCreation < DecayConfig.DecayStartTime)
	{
		return;
	}

	// Decay is handled by timer, this is just for manual calls
}

void AHarmoniaMemoryEchoActor::ApplyDecayToCurrencies(float DecayPercentage)
{
	if (!HasAuthority())
	{
		return;
	}

	TotalDecayPercentage += DecayPercentage;
	TotalDecayPercentage = FMath::Clamp(TotalDecayPercentage, 0.0f, 1.0f);

	bool bHasAnyCurrency = false;

	for (FHarmoniaCurrencyAmount& Currency : StoredCurrencies)
	{
		if (Currency.Amount > 0)
		{
			const int32 DecayAmount = FMath::CeilToInt32(Currency.Amount * DecayPercentage);
			Currency.Amount = FMath::Max(0, Currency.Amount - DecayAmount);

			if (Currency.Amount > 0)
			{
				bHasAnyCurrency = true;
			}
		}
	}

	if (!bHasAnyCurrency)
	{
		HandleCurrenciesExhausted();
	}
	else
	{
		OnRep_StoredCurrencies();
		UpdateVisualEffects();
	}
}

void AHarmoniaMemoryEchoActor::UpdateMemoryResonance()
{
	if (!HasAuthority() || !DeathPenaltyConfig)
	{
		return;
	}

	if (!DeathPenaltyConfig->Config.MemoryResonanceConfig.bEnableMemoryResonance)
	{
		return;
	}

	TArray<AActor*> CurrentEnemies = GetEnemiesInResonanceRange();
	TArray<TWeakObjectPtr<AActor>> NewBuffedEnemies;

	// Apply buffs to new enemies
	for (AActor* Enemy : CurrentEnemies)
	{
		if (Enemy)
		{
			bool bAlreadyBuffed = false;
			for (const TWeakObjectPtr<AActor>& BuffedEnemy : BuffedEnemies)
			{
				if (BuffedEnemy.IsValid() && BuffedEnemy.Get() == Enemy)
				{
					bAlreadyBuffed = true;
					break;
				}
			}

			if (!bAlreadyBuffed)
			{
				ApplyResonanceBuffToEnemy(Enemy);
			}

			NewBuffedEnemies.Add(Enemy);
		}
	}

	// Remove buffs from enemies that left range
	for (const TWeakObjectPtr<AActor>& BuffedEnemy : BuffedEnemies)
	{
		if (BuffedEnemy.IsValid())
		{
			bool bStillInRange = false;
			for (AActor* CurrentEnemy : CurrentEnemies)
			{
				if (CurrentEnemy == BuffedEnemy.Get())
				{
					bStillInRange = true;
					break;
				}
			}

			if (!bStillInRange)
			{
				RemoveResonanceBuffFromEnemy(BuffedEnemy.Get());
			}
		}
	}

	BuffedEnemies = NewBuffedEnemies;
}

TArray<AActor*> AHarmoniaMemoryEchoActor::GetEnemiesInResonanceRange() const
{
	TArray<AActor*> Enemies;

	if (!ResonanceSphere)
	{
		return Enemies;
	}

	TArray<AActor*> OverlappingActors;
	ResonanceSphere->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		// Check if it's a monster/enemy
		// You can add more specific checks here based on your game's enemy detection
		if (Actor && Actor->IsA(APawn::StaticClass()) && !Actor->IsA(APlayerController::StaticClass()))
		{
			Enemies.Add(Actor);
		}
	}

	return Enemies;
}

void AHarmoniaMemoryEchoActor::ApplyResonanceBuffToEnemy(AActor* Enemy)
{
	if (!Enemy || !DeathPenaltyConfig)
	{
		return;
	}

	// This should apply a gameplay effect to the enemy
	// For now, this is a placeholder - you would integrate with GAS here
	// Example: Apply a damage buff and health buff using gameplay effects

	UE_LOG(LogTemp, Log, TEXT("Applied memory resonance buff to enemy: %s"), *Enemy->GetName());
}

void AHarmoniaMemoryEchoActor::RemoveResonanceBuffFromEnemy(AActor* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	// This should remove the gameplay effect from the enemy
	// Placeholder for GAS integration

	UE_LOG(LogTemp, Log, TEXT("Removed memory resonance buff from enemy: %s"), *Enemy->GetName());
}

void AHarmoniaMemoryEchoActor::UpdateVisualEffects()
{
	if (!EffectComponent)
	{
		return;
	}

	// Scale effect based on remaining currencies and decay
	const float IntensityScale = FMath::Clamp(1.0f - TotalDecayPercentage, 0.1f, 1.0f);

	// You can modify particle parameters here
	// Example: EffectComponent->SetFloatParameter(FName("Intensity"), IntensityScale);
}

void AHarmoniaMemoryEchoActor::UpdateAudio()
{
	if (!AmbientSound)
	{
		return;
	}

	// Adjust volume based on decay
	const float VolumeScale = FMath::Clamp(1.0f - TotalDecayPercentage, 0.0f, 1.0f);
	AmbientSound->SetVolumeMultiplier(VolumeScale);
}

void AHarmoniaMemoryEchoActor::HandleCurrenciesExhausted()
{
	if (!HasAuthority())
	{
		return;
	}

	// Clear timers
	GetWorldTimerManager().ClearTimer(DecayTimerHandle);
	GetWorldTimerManager().ClearTimer(ResonanceTimerHandle);

	// Remove all resonance buffs
	for (const TWeakObjectPtr<AActor>& BuffedEnemy : BuffedEnemies)
	{
		if (BuffedEnemy.IsValid())
		{
			RemoveResonanceBuffFromEnemy(BuffedEnemy.Get());
		}
	}
	BuffedEnemies.Empty();

	// Destroy actor after a short delay for effect
	FTimerHandle DestroyTimerHandle;
	GetWorldTimerManager().SetTimer(
		DestroyTimerHandle,
		[this]()
		{
			Destroy();
		},
		2.0f,
		false
	);
}

void AHarmoniaMemoryEchoActor::OnInteractionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Interaction logic can be handled by the player controller or component
	// This is just for awareness
}

void AHarmoniaMemoryEchoActor::OnInteractionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Cleanup if needed
}

void AHarmoniaMemoryEchoActor::OnRep_StoredCurrencies()
{
	UpdateVisualEffects();
	UpdateAudio();
}

void AHarmoniaMemoryEchoActor::OnRep_TotalDecayPercentage()
{
	UpdateVisualEffects();
	UpdateAudio();
}
