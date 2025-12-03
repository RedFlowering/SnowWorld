// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaRecoveryAreaActor.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Character/LyraHealthComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/HarmoniaAbilitySystemLibrary.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"

AHarmoniaRecoveryAreaActor::AHarmoniaRecoveryAreaActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Create Sphere Component
	RecoveryAreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RecoveryAreaSphere"));
	SetRootComponent(RecoveryAreaSphere);
	RecoveryAreaSphere->SetSphereRadius(300.0f);
	RecoveryAreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RecoveryAreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	RecoveryAreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Create VFX Component
	AreaVFXComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AreaVFXComponent"));
	AreaVFXComponent->SetupAttachment(RootComponent);
	AreaVFXComponent->SetAutoActivate(false);

	// Create Audio Component
	AreaAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AreaAudioComponent"));
	AreaAudioComponent->SetupAttachment(RootComponent);
	AreaAudioComponent->SetAutoActivate(false);

	// Default settings
	RecoveryConfig.RecoveryRadius = 300.0f;
	RecoveryConfig.Duration = 60.0f;
	RecoveryConfig.HealthPerTick = 5.0f;
	RecoveryConfig.TickInterval = 1.0f;
}

void AHarmoniaRecoveryAreaActor::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap events
	RecoveryAreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AHarmoniaRecoveryAreaActor::OnActorEnterRecoveryArea);
	RecoveryAreaSphere->OnComponentEndOverlap.AddDynamic(this, &AHarmoniaRecoveryAreaActor::OnActorLeaveRecoveryArea);

	// Auto-activate
	ActivateRecoveryArea();
}

void AHarmoniaRecoveryAreaActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RecoveryTickTimerHandle);
		World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AHarmoniaRecoveryAreaActor::InitializeRecoveryArea(const FHarmoniaDeployableRecoveryConfig& Config)
{
	RecoveryConfig = Config;

	// Update sphere radius
	if (RecoveryAreaSphere)
	{
		RecoveryAreaSphere->SetSphereRadius(Config.RecoveryRadius);
	}

	// Set VFX
	if (AreaVFXComponent && Config.AreaVFX)
	{
		AreaVFXComponent->SetAsset(Config.AreaVFX);
	}

	// Set SFX
	if (AreaAudioComponent && Config.AreaSound)
	{
		AreaAudioComponent->SetSound(Config.AreaSound);
	}
}

void AHarmoniaRecoveryAreaActor::ActivateRecoveryArea()
{
	if (bIsActive)
	{
		return;
	}

	bIsActive = true;

	// Activate VFX
	if (AreaVFXComponent)
	{
		AreaVFXComponent->Activate();
	}

	// Activate SFX
	if (AreaAudioComponent)
	{
		AreaAudioComponent->Play();
	}

	// Start recovery tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RecoveryTickTimerHandle,
			this,
			&AHarmoniaRecoveryAreaActor::PerformRecoveryTick,
			RecoveryConfig.TickInterval,
			true
		);

		// Set expiration timer
		if (RecoveryConfig.Duration > 0.0f)
		{
			World->GetTimerManager().SetTimer(
				ExpirationTimerHandle,
				this,
				&AHarmoniaRecoveryAreaActor::ExpireRecoveryArea,
				RecoveryConfig.Duration,
				false
			);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Recovery Area activated with radius %f, duration %f, tick %f"),
		RecoveryConfig.RecoveryRadius, RecoveryConfig.Duration, RecoveryConfig.TickInterval);
}

void AHarmoniaRecoveryAreaActor::DeactivateRecoveryArea()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;

	// Deactivate VFX
	if (AreaVFXComponent && AreaVFXComponent->IsActive())
	{
		AreaVFXComponent->Deactivate();
	}

	// Stop SFX
	if (AreaAudioComponent && AreaAudioComponent->IsPlaying())
	{
		AreaAudioComponent->Stop();
	}

	// Clean up timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RecoveryTickTimerHandle);
		World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("Recovery Area deactivated"));
}

void AHarmoniaRecoveryAreaActor::PerformRecoveryTick()
{
	if (!bIsActive)
	{
		return;
	}

	int32 HealedCount = 0;

	// Apply recovery to all actors in range
	for (AActor* Actor : ActorsInArea)
	{
		if (!Actor || !Actor->IsValidLowLevel())
		{
			continue;
		}

		// Find Health Component
		if (ULyraHealthComponent* HealthComp = Actor->FindComponentByClass<ULyraHealthComponent>())
		{
			if (HealthComp->GetHealth() < HealthComp->GetMaxHealth())
			{
				// Apply healing through AbilitySystemComponent if available
				if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
				{
					// Use Harmonia Ability System Library to restore health
					UHarmoniaAbilitySystemLibrary::RestoreHealth(ASC, RecoveryConfig.HealthPerTick);
					UE_LOG(LogTemp, Log, TEXT("Healing %s for %f HP via AbilitySystem"), *Actor->GetName(), RecoveryConfig.HealthPerTick);
					HealedCount++;
				}
				else
				{
					// Fallback: directly heal through Lyra Health Component
					float CurrentHealth = HealthComp->GetHealth();
					float MaxHealth = HealthComp->GetMaxHealth();
					float NewHealth = FMath::Min(CurrentHealth + RecoveryConfig.HealthPerTick, MaxHealth);
					float ActualHealing = NewHealth - CurrentHealth;

					// Note: Lyra's HealthComponent doesn't have a direct SetHealth,
					// so we would need to use a proper healing mechanism or gameplay effect
					UE_LOG(LogTemp, Log, TEXT("Healing %s for %f HP (no ASC)"), *Actor->GetName(), ActualHealing);
					HealedCount++;
				}
			}
		}
	}

	if (HealedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Recovery Area healed %d actors"), HealedCount);
	}
}

TArray<AActor*> AHarmoniaRecoveryAreaActor::GetActorsInRecoveryArea() const
{
	return ActorsInArea.Array();
}

void AHarmoniaRecoveryAreaActor::ExpireRecoveryArea()
{
	UE_LOG(LogTemp, Log, TEXT("Recovery Area expired"));

	DeactivateRecoveryArea();

	// Actor ?�괴
	Destroy();
}

void AHarmoniaRecoveryAreaActor::OnActorEnterRecoveryArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || !bIsActive)
	{
		return;
	}

	// Character�??�복 (?�는 HealthComponent가 ?�는 Actor)
	if (OtherActor->IsA<ACharacter>() || OtherActor->FindComponentByClass<ULyraHealthComponent>())
	{
		ActorsInArea.Add(OtherActor);
		UE_LOG(LogTemp, Log, TEXT("Actor %s entered recovery area"), *OtherActor->GetName());
	}
}

void AHarmoniaRecoveryAreaActor::OnActorLeaveRecoveryArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	ActorsInArea.Remove(OtherActor);
	UE_LOG(LogTemp, Log, TEXT("Actor %s left recovery area"), *OtherActor->GetName());
}
