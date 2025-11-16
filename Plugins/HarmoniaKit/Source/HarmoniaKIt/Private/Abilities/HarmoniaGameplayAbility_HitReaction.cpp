// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_HitReaction.h"
#include "HarmoniaGameplayTags.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

UHarmoniaGameplayAbility_HitReaction::UHarmoniaGameplayAbility_HitReaction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Set default activation policy
	ActivationPolicy = ELyraAbilityActivationPolicy::OnSpawn;
	ActivationGroup = ELyraAbilityActivationGroup::Exclusive_Blocking;

	// Setup default tags
	HitReactionTags.AddTag(HarmoniaGameplayTags::State_HitReaction);
	HitReactionTags.AddTag(HarmoniaGameplayTags::State_HitStunned);

	BlockedTags.AddTag(HarmoniaGameplayTags::State_Combat_Attacking);
	BlockedTags.AddTag(HarmoniaGameplayTags::State_HitReaction);

	// Trigger on hit reaction event
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = HarmoniaGameplayTags::GameplayEvent_HitReaction;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

bool UHarmoniaGameplayAbility_HitReaction::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check if already in hit reaction
	const UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC && ASC->HasMatchingGameplayTag(HarmoniaGameplayTags::State_HitReaction))
	{
		// Allow if interruption is enabled and minimum time has passed
		if (bAllowInterruption)
		{
			const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
			const float TimeSinceStart = CurrentTime - HitReactionStartTime;
			return TimeSinceStart >= MinimumInterruptTime;
		}
		return false;
	}

	// Check if invincible
	if (ASC && ASC->HasMatchingGameplayTag(HarmoniaGameplayTags::State_Invincible))
	{
		return false;
	}

	return true;
}

void UHarmoniaGameplayAbility_HitReaction::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Record start time
	if (UWorld* World = GetWorld())
	{
		HitReactionStartTime = World->GetTimeSeconds();
	}

	// Apply hit reaction tags
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTags(HitReactionTags);
	}

	// Apply invincibility if enabled
	if (bApplyInvincibilityFrames)
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Invincible);

			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					InvincibilityTimerHandle,
					[this, ASC]()
					{
						if (ASC)
						{
							ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Invincible);
						}
					},
					InvincibilityDuration,
					false
				);
			}
		}
	}

	// Perform the hit reaction
	if (TriggerEventData)
	{
		PerformHitReaction(*TriggerEventData);
	}
	else
	{
		// Create default event data
		FGameplayEventData DefaultEventData;
		PerformHitReaction(DefaultEventData);
	}
}

void UHarmoniaGameplayAbility_HitReaction::PerformHitReaction(const FGameplayEventData& EventData)
{
	// Determine hit reaction type from event data or use default
	EHarmoniaHitReactionType ReactionType = DefaultReactionType;

	// Try to get reaction type from event magnitude (encoded as enum value)
	if (EventData.EventMagnitude > 0.0f)
	{
		ReactionType = static_cast<EHarmoniaHitReactionType>(FMath::RoundToInt(EventData.EventMagnitude));
	}

	// Get hit reaction data
	CurrentReactionData = GetHitReactionData(ReactionType);

	// Calculate hit direction
	if (bAutoDetectHitDirection && EventData.Instigator != nullptr)
	{
		CurrentHitDirection = CalculateHitDirection(EventData.Instigator->GetActorLocation());
	}
	else
	{
		CurrentHitDirection = EHarmoniaHitDirection::Front;
	}

	// Get appropriate montage for direction
	UAnimMontage* HitMontage = CurrentReactionData.GetMontageForDirection(CurrentHitDirection);
	if (!HitMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("HitReaction: No montage for direction %d"), static_cast<int32>(CurrentHitDirection));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// Apply movement speed modification
	if (CurrentReactionData.MovementSpeedMultiplier < 1.0f)
	{
		ApplyMovementSpeedModification(CurrentReactionData.MovementSpeedMultiplier);
	}

	// Apply additional tags from data
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTags(CurrentReactionData.AppliedTags);
		ASC->BlockAbilitiesWithTags(CurrentReactionData.BlockedTags);
	}

	// Play the hit montage
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		HitMontage,
		1.0f,
		NAME_None,
		false,
		1.0f
	);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UHarmoniaGameplayAbility_HitReaction::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UHarmoniaGameplayAbility_HitReaction::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &UHarmoniaGameplayAbility_HitReaction::OnMontageInterrupted);
		MontageTask->ReadyForActivation();
	}

	// Set stun duration timer
	if (CurrentReactionData.StunDuration > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				StunTimerHandle,
				[this]()
				{
					// Remove stun tag after duration
					if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
					{
						ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_HitStunned);
					}
				},
				CurrentReactionData.StunDuration,
				false
			);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("HitReaction: Performing %s reaction in direction %d"),
		*CurrentReactionData.DisplayName.ToString(),
		static_cast<int32>(CurrentHitDirection));
}

EHarmoniaHitDirection UHarmoniaGameplayAbility_HitReaction::CalculateHitDirection(const FVector& InstigatorLocation) const
{
	const AActor* Owner = GetOwningActorFromActorInfo();
	if (!Owner)
	{
		return EHarmoniaHitDirection::Front;
	}

	// Get direction to instigator
	const FVector OwnerLocation = Owner->GetActorLocation();
	const FVector DirectionToInstigator = (InstigatorLocation - OwnerLocation).GetSafeNormal2D();

	// Get owner forward vector
	const FVector OwnerForward = Owner->GetActorForwardVector();

	// Calculate angle between forward and direction to instigator
	const float DotProduct = FVector::DotProduct(OwnerForward, DirectionToInstigator);
	const float CrossZ = FVector::CrossProduct(OwnerForward, DirectionToInstigator).Z;

	// Calculate angle in degrees
	const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

	// Determine direction based on angle and cross product
	if (AngleDegrees <= DirectionAngleThreshold)
	{
		// Front
		return EHarmoniaHitDirection::Front;
	}
	else if (AngleDegrees >= (180.0f - DirectionAngleThreshold))
	{
		// Back
		return EHarmoniaHitDirection::Back;
	}
	else
	{
		// Left or Right based on cross product
		return CrossZ > 0.0f ? EHarmoniaHitDirection::Left : EHarmoniaHitDirection::Right;
	}
}

FHitReactionData UHarmoniaGameplayAbility_HitReaction::GetHitReactionData(EHarmoniaHitReactionType ReactionType) const
{
	if (!HitReactionDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("HitReaction: No HitReactionDataTable set"));
		return FHitReactionData();
	}

	// Convert enum to row name (e.g., "Light", "Medium", "Heavy")
	FString RowNameString;
	switch (ReactionType)
	{
	case EHarmoniaHitReactionType::Light:
		RowNameString = TEXT("Light");
		break;
	case EHarmoniaHitReactionType::Medium:
		RowNameString = TEXT("Medium");
		break;
	case EHarmoniaHitReactionType::Heavy:
		RowNameString = TEXT("Heavy");
		break;
	case EHarmoniaHitReactionType::Knockback:
		RowNameString = TEXT("Knockback");
		break;
	case EHarmoniaHitReactionType::Stun:
		RowNameString = TEXT("Stun");
		break;
	case EHarmoniaHitReactionType::Ragdoll:
		RowNameString = TEXT("Ragdoll");
		break;
	default:
		RowNameString = TEXT("Light");
		break;
	}

	const FName RowName = FName(*RowNameString);
	const FHitReactionData* Data = HitReactionDataTable->FindRow<FHitReactionData>(RowName, TEXT("GetHitReactionData"));
	return Data ? *Data : FHitReactionData();
}

void UHarmoniaGameplayAbility_HitReaction::ApplyMovementSpeedModification(float Multiplier)
{
	const AActor* Owner = GetOwningActorFromActorInfo();
	const ACharacter* Character = Cast<const ACharacter>(Owner);
	if (!Character || !Character->GetCharacterMovement())
	{
		return;
	}

	// Store original speed and apply modifier
	// Note: In a real implementation, you'd want to use a Gameplay Effect for this
	// This is a simplified version
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	Movement->MaxWalkSpeed *= Multiplier;
}

void UHarmoniaGameplayAbility_HitReaction::RemoveMovementSpeedModification()
{
	const AActor* Owner = GetOwningActorFromActorInfo();
	const ACharacter* Character = Cast<const ACharacter>(Owner);
	if (!Character || !Character->GetCharacterMovement())
	{
		return;
	}

	// Restore original speed
	// Note: In a real implementation, you'd remove the Gameplay Effect
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (CurrentReactionData.MovementSpeedMultiplier > 0.0f)
	{
		Movement->MaxWalkSpeed /= CurrentReactionData.MovementSpeedMultiplier;
	}
}

void UHarmoniaGameplayAbility_HitReaction::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Remove hit reaction tags
	if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTags(HitReactionTags);
		ASC->RemoveLooseGameplayTags(CurrentReactionData.AppliedTags);
		ASC->UnBlockAbilitiesWithTags(CurrentReactionData.BlockedTags);
	}

	// Remove movement speed modification
	RemoveMovementSpeedModification();

	// Clear timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StunTimerHandle);
		World->GetTimerManager().ClearTimer(InvincibilityTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_HitReaction::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHarmoniaGameplayAbility_HitReaction::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UHarmoniaGameplayAbility_HitReaction::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
