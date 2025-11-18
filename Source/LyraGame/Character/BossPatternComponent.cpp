// Copyright 2024 Snow Game Studio.

#include "BossPatternComponent.h"
#include "BossCharacter.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"

UBossPatternComponent::UBossPatternComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetIsReplicatedByDefault(true);
}

void UBossPatternComponent::BeginPlay()
{
	Super::BeginPlay();

	BossOwner = Cast<ABossCharacter>(GetOwner());
}

void UBossPatternComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateCooldowns(DeltaTime);
}

//~=============================================================================
// Pattern Execution
//~=============================================================================

bool UBossPatternComponent::ExecutePattern(FName PatternName)
{
	if (!BossOwner)
	{
		return false;
	}

	const FBossAttackPattern* Pattern = GetPatternConfig(PatternName);
	if (!Pattern)
	{
		return false;
	}

	if (!CanExecutePattern(*Pattern))
	{
		return false;
	}

	StartPatternExecution(*Pattern);
	return true;
}

bool UBossPatternComponent::ExecuteRandomPattern()
{
	FName RandomPattern = SelectRandomPattern();
	if (RandomPattern.IsNone())
	{
		return false;
	}

	return ExecutePattern(RandomPattern);
}

void UBossPatternComponent::StopCurrentPattern()
{
	if (!bIsExecutingPattern)
	{
		return;
	}

	// Clear timers
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AbilityDelayTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(RepeatDelayTimerHandle);
	}

	CompletePatternExecution();
}

bool UBossPatternComponent::IsPatternAvailable(FName PatternName) const
{
	const FBossAttackPattern* Pattern = GetPatternConfig(PatternName);
	if (!Pattern)
	{
		return false;
	}

	return CanExecutePattern(*Pattern);
}

TArray<FName> UBossPatternComponent::GetAvailablePatterns() const
{
	TArray<FName> AvailablePatterns;

	for (const auto& PatternPair : AttackPatterns)
	{
		if (CanExecutePattern(PatternPair.Value))
		{
			AvailablePatterns.Add(PatternPair.Key);
		}
	}

	return AvailablePatterns;
}

const FBossAttackPattern* UBossPatternComponent::GetPatternConfig(FName PatternName) const
{
	return AttackPatterns.Find(PatternName);
}

bool UBossPatternComponent::IsPatternOnCooldown(FName PatternName) const
{
	const float* Cooldown = PatternCooldowns.Find(PatternName);
	return Cooldown && *Cooldown > 0.0f;
}

float UBossPatternComponent::GetPatternCooldownRemaining(FName PatternName) const
{
	const float* Cooldown = PatternCooldowns.Find(PatternName);
	return Cooldown ? *Cooldown : 0.0f;
}

//~=============================================================================
// Pattern Execution Implementation
//~=============================================================================

void UBossPatternComponent::StartPatternExecution(const FBossAttackPattern& Pattern)
{
	if (bIsExecutingPattern && !Pattern.bCanBeInterrupted)
	{
		return;
	}

	// Stop current pattern if any
	if (bIsExecutingPattern)
	{
		StopCurrentPattern();
	}

	bIsExecutingPattern = true;
	CurrentPatternName = Pattern.PatternName;
	ActivePattern = Pattern;
	CurrentAbilityIndex = 0;
	CurrentRepeatCount = 0;

	// Play pattern montage
	if (Pattern.PatternMontage && BossOwner)
	{
		if (UAnimInstance* AnimInstance = BossOwner->GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(Pattern.PatternMontage);
		}
	}

	// Broadcast pattern start
	OnPatternExecutionStart.Broadcast(Pattern.PatternName);

	// Execute based on mode
	switch (Pattern.ExecutionMode)
	{
	case EBossPatternExecutionMode::Single:
	case EBossPatternExecutionMode::Sequence:
		ExecuteNextAbility();
		break;

	case EBossPatternExecutionMode::Simultaneous:
		// Execute all abilities at once
		for (int32 i = 0; i < Pattern.PatternAbilities.Num(); ++i)
		{
			ExecuteAbilityAtIndex(i);
		}
		CompletePatternExecution();
		break;

	case EBossPatternExecutionMode::Random:
		{
			// Execute random ability from pattern
			if (Pattern.PatternAbilities.Num() > 0)
			{
				int32 RandomIndex = FMath::RandRange(0, Pattern.PatternAbilities.Num() - 1);
				ExecuteAbilityAtIndex(RandomIndex);
			}
			CompletePatternExecution();
		}
		break;
	}
}

void UBossPatternComponent::ExecuteNextAbility()
{
	if (!bIsExecutingPattern)
	{
		return;
	}

	// Check if we've executed all abilities
	if (CurrentAbilityIndex >= ActivePattern.PatternAbilities.Num())
	{
		// Check if we should repeat
		CurrentRepeatCount++;
		if (CurrentRepeatCount < ActivePattern.RepeatCount)
		{
			CurrentAbilityIndex = 0;

			// Wait for repeat delay
			if (ActivePattern.RepeatDelay > 0.0f && GetWorld())
			{
				GetWorld()->GetTimerManager().SetTimer(
					RepeatDelayTimerHandle,
					this,
					&UBossPatternComponent::OnRepeatDelayComplete,
					ActivePattern.RepeatDelay,
					false
				);
				return;
			}
			else
			{
				ExecuteNextAbility();
				return;
			}
		}
		else
		{
			// Pattern complete
			CompletePatternExecution();
			return;
		}
	}

	// Execute current ability
	ExecuteAbilityAtIndex(CurrentAbilityIndex);
	CurrentAbilityIndex++;

	// Set up delay for next ability
	if (CurrentAbilityIndex < ActivePattern.PatternAbilities.Num())
	{
		if (ActivePattern.AbilityDelay > 0.0f && GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				AbilityDelayTimerHandle,
				this,
				&UBossPatternComponent::OnAbilityDelayComplete,
				ActivePattern.AbilityDelay,
				false
			);
		}
		else
		{
			ExecuteNextAbility();
		}
	}
	else if (ActivePattern.ExecutionMode == EBossPatternExecutionMode::Single)
	{
		// Single execution mode completes after one ability
		CompletePatternExecution();
	}
	else
	{
		// Check for repeat
		ExecuteNextAbility();
	}
}

void UBossPatternComponent::ExecuteAbilityAtIndex(int32 AbilityIndex)
{
	if (!BossOwner)
	{
		return;
	}

	if (!ActivePattern.PatternAbilities.IsValidIndex(AbilityIndex))
	{
		return;
	}

	ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(BossOwner->GetAbilitySystemComponent());
	if (!ASC)
	{
		return;
	}

	TSubclassOf<ULyraGameplayAbility> AbilityClass = ActivePattern.PatternAbilities[AbilityIndex];
	if (!AbilityClass)
	{
		return;
	}

	// Try to activate the ability
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AbilityClass);
	if (Spec)
	{
		ASC->TryActivateAbility(Spec->Handle);
	}
	else
	{
		// Ability not granted, try to grant and activate it temporarily
		FGameplayAbilitySpec TempSpec(AbilityClass, 1, INDEX_NONE, BossOwner);
		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(TempSpec);
		ASC->TryActivateAbility(Handle);
	}
}

void UBossPatternComponent::CompletePatternExecution()
{
	FName CompletedPattern = CurrentPatternName;

	// Start cooldown
	if (ActivePattern.Cooldown > 0.0f)
	{
		PatternCooldowns.Add(CurrentPatternName, ActivePattern.Cooldown);
	}

	// Reset state
	bIsExecutingPattern = false;
	CurrentPatternName = NAME_None;
	CurrentAbilityIndex = 0;
	CurrentRepeatCount = 0;

	// Notify boss owner
	if (BossOwner)
	{
		BossOwner->OnPatternEnd(CompletedPattern);
	}

	// Broadcast pattern end
	OnPatternExecutionEnd.Broadcast(CompletedPattern);
}

bool UBossPatternComponent::CanExecutePattern(const FBossAttackPattern& Pattern) const
{
	if (!BossOwner)
	{
		return false;
	}

	// Check if pattern is on cooldown
	if (IsPatternOnCooldown(Pattern.PatternName))
	{
		return false;
	}

	// Check if currently executing and can't interrupt
	if (bIsExecutingPattern && !Pattern.bCanBeInterrupted)
	{
		return false;
	}

	// Check valid phases
	if (Pattern.ValidPhases.Num() > 0)
	{
		int32 CurrentPhase = BossOwner->GetCurrentPhase();
		if (!Pattern.ValidPhases.Contains(CurrentPhase))
		{
			return false;
		}
	}

	// Check required tags and blocked tags
	ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(BossOwner->GetAbilitySystemComponent());
	if (ASC)
	{
		if (Pattern.RequiredTags.Num() > 0)
		{
			if (!ASC->HasAllMatchingGameplayTags(Pattern.RequiredTags))
			{
				return false;
			}
		}

		if (Pattern.BlockedByTags.Num() > 0)
		{
			if (ASC->HasAnyMatchingGameplayTags(Pattern.BlockedByTags))
			{
				return false;
			}
		}
	}

	return true;
}

FName UBossPatternComponent::SelectRandomPattern() const
{
	TArray<FName> AvailablePatternNames;
	TArray<float> Weights;

	// Collect available patterns and their weights
	for (const auto& PatternPair : AttackPatterns)
	{
		if (CanExecutePattern(PatternPair.Value))
		{
			AvailablePatternNames.Add(PatternPair.Key);
			Weights.Add(PatternPair.Value.RandomWeight);
		}
	}

	if (AvailablePatternNames.Num() == 0)
	{
		return NAME_None;
	}

	// Calculate total weight
	float TotalWeight = 0.0f;
	for (float Weight : Weights)
	{
		TotalWeight += Weight;
	}

	if (TotalWeight <= 0.0f)
	{
		// All weights are 0, choose randomly
		int32 RandomIndex = FMath::RandRange(0, AvailablePatternNames.Num() - 1);
		return AvailablePatternNames[RandomIndex];
	}

	// Weighted random selection
	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;

	for (int32 i = 0; i < AvailablePatternNames.Num(); ++i)
	{
		CurrentWeight += Weights[i];
		if (RandomValue <= CurrentWeight)
		{
			return AvailablePatternNames[i];
		}
	}

	// Fallback (shouldn't reach here)
	return AvailablePatternNames.Last();
}

void UBossPatternComponent::UpdateCooldowns(float DeltaTime)
{
	TArray<FName> PatternsToRemove;

	for (auto& CooldownPair : PatternCooldowns)
	{
		CooldownPair.Value -= DeltaTime;
		if (CooldownPair.Value <= 0.0f)
		{
			PatternsToRemove.Add(CooldownPair.Key);
		}
	}

	for (const FName& PatternName : PatternsToRemove)
	{
		PatternCooldowns.Remove(PatternName);
	}
}

void UBossPatternComponent::OnAbilityDelayComplete()
{
	ExecuteNextAbility();
}

void UBossPatternComponent::OnRepeatDelayComplete()
{
	ExecuteNextAbility();
}
