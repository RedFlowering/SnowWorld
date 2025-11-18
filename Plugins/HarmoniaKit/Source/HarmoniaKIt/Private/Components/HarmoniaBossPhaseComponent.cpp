// Copyright 2024 Snow Game Studio.

#include "Components/HarmoniaBossPhaseComponent.h"
#include "Monsters/HarmoniaBossCharacter.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

UHarmoniaBossPhaseComponent::UHarmoniaBossPhaseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHarmoniaBossPhaseComponent::BeginPlay()
{
	Super::BeginPlay();

	BossOwner = Cast<AHarmoniaBossCharacter>(GetOwner());
}

//~=============================================================================
// Phase Management
//~=============================================================================

void UHarmoniaBossPhaseComponent::OnPhaseTransition(int32 NewPhase)
{
	if (!BossOwner)
	{
		return;
	}

	if (NewPhase == CurrentPhaseIndex)
	{
		return;
	}

	StartPhaseTransition(NewPhase);
}

const FHarmoniaBossPhaseConfig* UHarmoniaBossPhaseComponent::GetPhaseConfig(int32 PhaseIndex) const
{
	return PhaseConfigs.Find(PhaseIndex);
}

const FHarmoniaBossPhaseConfig* UHarmoniaBossPhaseComponent::GetCurrentPhaseConfig() const
{
	return GetPhaseConfig(CurrentPhaseIndex);
}

//~=============================================================================
// Phase Transition Implementation
//~=============================================================================

void UHarmoniaBossPhaseComponent::StartPhaseTransition(int32 NewPhase)
{
	if (bIsTransitioning)
	{
		return;
	}

	bIsTransitioning = true;

	// Remove old phase config
	if (const FHarmoniaBossPhaseConfig* OldConfig = GetPhaseConfig(CurrentPhaseIndex))
	{
		RemovePhaseConfig(*OldConfig);
	}

	// Update phase
	CurrentPhaseIndex = NewPhase;

	// Get new phase config
	const FHarmoniaBossPhaseConfig* NewConfig = GetPhaseConfig(NewPhase);
	if (!NewConfig)
	{
		CompletePhaseTransition();
		return;
	}

	// Play transition montage
	if (NewConfig->TransitionMontage && BossOwner)
	{
		if (UAnimInstance* AnimInstance = BossOwner->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(NewConfig->TransitionMontage);
		}
	}

	// Spawn transition VFX
	if (NewConfig->TransitionEffect && BossOwner)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			NewConfig->TransitionEffect,
			BossOwner->GetActorLocation(),
			BossOwner->GetActorRotation()
		);
	}

	// Spawn transition Niagara VFX
	if (NewConfig->TransitionNiagaraEffect && BossOwner)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			NewConfig->TransitionNiagaraEffect,
			BossOwner->GetActorLocation(),
			BossOwner->GetActorRotation()
		);
	}

	// Apply invulnerability if configured
	if (NewConfig->bInvulnerableDuringTransition)
	{
		if (ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(BossOwner->GetAbilitySystemComponent()))
		{
			// Add invulnerability tag
			FGameplayTagContainer InvulnerabilityTags;
			// Note: You may need to define this tag in your project
			// InvulnerabilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Invulnerable")));
			ASC->AddLooseGameplayTags(InvulnerabilityTags);
		}
	}

	// Set timer for transition completion
	if (NewConfig->TransitionDuration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			TransitionTimerHandle,
			this,
			&UHarmoniaBossPhaseComponent::OnTransitionTimerComplete,
			NewConfig->TransitionDuration,
			false
		);
	}
	else
	{
		CompletePhaseTransition();
	}
}

void UHarmoniaBossPhaseComponent::CompletePhaseTransition()
{
	// Apply new phase config
	if (const FHarmoniaBossPhaseConfig* NewConfig = GetPhaseConfig(CurrentPhaseIndex))
	{
		ApplyPhaseConfig(*NewConfig);

		// Remove invulnerability
		if (NewConfig->bInvulnerableDuringTransition && BossOwner)
		{
			if (ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(BossOwner->GetAbilitySystemComponent()))
			{
				FGameplayTagContainer InvulnerabilityTags;
				// InvulnerabilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Status.Invulnerable")));
				ASC->RemoveLooseGameplayTags(InvulnerabilityTags);
			}
		}
	}

	bIsTransitioning = false;

	// Broadcast completion
	OnPhaseTransitionComplete.Broadcast(CurrentPhaseIndex);
}

void UHarmoniaBossPhaseComponent::ApplyPhaseConfig(const FHarmoniaBossPhaseConfig& Config)
{
	if (!BossOwner)
	{
		return;
	}

	ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(BossOwner->GetAbilitySystemComponent());
	if (!ASC)
	{
		return;
	}

	// Remove specified abilities
	RemovePhaseAbilities(Config.AbilitiesToRemove);

	// Grant phase abilities
	GrantPhaseAbilities(Config.PhaseAbilities);

	// Apply gameplay tags
	if (Config.PhaseTags.Num() > 0)
	{
		ASC->AddLooseGameplayTags(Config.PhaseTags);
	}

	// Remove specified tags
	if (Config.TagsToRemove.Num() > 0)
	{
		ASC->RemoveLooseGameplayTags(Config.TagsToRemove);
	}

	// Apply stat multipliers (would need to implement via gameplay effects in a real implementation)
	// For now, this is a placeholder for where you'd apply damage/defense/speed multipliers
}

void UHarmoniaBossPhaseComponent::RemovePhaseConfig(const FHarmoniaBossPhaseConfig& Config)
{
	if (!BossOwner)
	{
		return;
	}

	ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(BossOwner->GetAbilitySystemComponent());
	if (!ASC)
	{
		return;
	}

	// Remove phase abilities
	RemovePhaseAbilities(Config.PhaseAbilities);

	// Remove phase tags
	if (Config.PhaseTags.Num() > 0)
	{
		ASC->RemoveLooseGameplayTags(Config.PhaseTags);
	}
}

void UHarmoniaBossPhaseComponent::GrantPhaseAbilities(const TArray<TSubclassOf<ULyraGameplayAbility>>& Abilities)
{
	if (!BossOwner)
	{
		return;
	}

	ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(BossOwner->GetAbilitySystemComponent());
	if (!ASC)
	{
		return;
	}

	for (const TSubclassOf<ULyraGameplayAbility>& AbilityClass : Abilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, BossOwner);
		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AbilitySpec);

		GrantedAbilityHandles.Add(Handle);
	}
}

void UHarmoniaBossPhaseComponent::RemovePhaseAbilities(const TArray<TSubclassOf<ULyraGameplayAbility>>& Abilities)
{
	if (!BossOwner)
	{
		return;
	}

	ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(BossOwner->GetAbilitySystemComponent());
	if (!ASC)
	{
		return;
	}

	for (const TSubclassOf<ULyraGameplayAbility>& AbilityClass : Abilities)
	{
		if (!AbilityClass)
		{
			continue;
		}

		// Find and remove abilities of this class
		ASC->ClearAbility(ASC->FindAbilitySpecFromClass(AbilityClass));
	}
}

void UHarmoniaBossPhaseComponent::OnTransitionTimerComplete()
{
	CompletePhaseTransition();
}
