// Copyright 2024 Snow Game Studio.

#include "Components/HarmoniaMonsterPatternComponent.h"
#include "Monsters/HarmoniaBossMonster.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "HarmoniaGameplayTags.h"

UHarmoniaMonsterPatternComponent::UHarmoniaMonsterPatternComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetIsReplicatedByDefault(true);
}

void UHarmoniaMonsterPatternComponent::BeginPlay()
{
	Super::BeginPlay();

	MonsterOwner = Cast<AHarmoniaMonsterBase>(GetOwner());
}

void UHarmoniaMonsterPatternComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateCooldowns(DeltaTime);
}

//~=============================================================================
// Pattern Execution
//~=============================================================================

bool UHarmoniaMonsterPatternComponent::ExecutePattern(FName PatternName)
{
	if (!MonsterOwner)
	{
		return false;
	}

	const FMonsterAttackPattern* Pattern = GetPatternConfig(PatternName);
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

bool UHarmoniaMonsterPatternComponent::ExecuteRandomPattern()
{
	FName RandomPattern = SelectRandomPattern();
	if (RandomPattern.IsNone())
	{
		return false;
	}

	return ExecutePattern(RandomPattern);
}

bool UHarmoniaMonsterPatternComponent::ExecutePatternByIndex(int32 PatternIndex)
{
	if (!MonsterOwner)
	{
		return false;
	}

	const FMonsterPhasePatterns* CurrentPhaseData = GetCurrentPhasePatterns();
	if (!CurrentPhaseData)
	{
		return false;
	}

	if (!CurrentPhaseData->AttackPatterns.IsValidIndex(PatternIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("ExecutePatternByIndex: Index %d is out of range (Count: %d)"), 
			PatternIndex, CurrentPhaseData->AttackPatterns.Num());
		return false;
	}

	const FMonsterAttackPattern& Pattern = CurrentPhaseData->AttackPatterns[PatternIndex];
	if (!CanExecutePattern(Pattern))
	{
		return false;
	}

	StartPatternExecution(Pattern);
	return true;
}

int32 UHarmoniaMonsterPatternComponent::GetPatternCount() const
{
	const FMonsterPhasePatterns* CurrentPhaseData = GetCurrentPhasePatterns();
	if (CurrentPhaseData)
	{
		return CurrentPhaseData->AttackPatterns.Num();
	}
	return 0;
}

bool UHarmoniaMonsterPatternComponent::IsPatternAvailableByIndex(int32 PatternIndex) const
{
	const FMonsterPhasePatterns* CurrentPhaseData = GetCurrentPhasePatterns();
	if (!CurrentPhaseData)
	{
		return false;
	}

	if (!CurrentPhaseData->AttackPatterns.IsValidIndex(PatternIndex))
	{
		return false;
	}

	return CanExecutePattern(CurrentPhaseData->AttackPatterns[PatternIndex]);
}

void UHarmoniaMonsterPatternComponent::StopCurrentPattern()
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

void UHarmoniaMonsterPatternComponent::ForceResetPattern()
{
	UE_LOG(LogTemp, Warning, TEXT("ForceResetPattern: Forcing pattern state reset"));
	
	// Clear all timers
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AbilityDelayTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(RepeatDelayTimerHandle);
	}
	
	// Unbind ability ended delegate
	if (AbilityEndedDelegateHandle.IsValid() && MonsterOwner)
	{
		if (UAbilitySystemComponent* ASC = MonsterOwner->GetAbilitySystemComponent())
		{
			ASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
		}
		AbilityEndedDelegateHandle.Reset();
	}
	
	// Force reset all state
	bIsExecutingPattern = false;
	CurrentPatternName = NAME_None;
	CurrentAbilityIndex = 0;
	CurrentRepeatCount = 0;
	PendingAbilityCount = 0;
	ActivePattern = FMonsterAttackPattern();
}

bool UHarmoniaMonsterPatternComponent::IsPatternAvailable(FName PatternName) const
{
	const FMonsterAttackPattern* Pattern = GetPatternConfig(PatternName);
	if (!Pattern)
	{
		return false;
	}

	return CanExecutePattern(*Pattern);
}

TArray<FName> UHarmoniaMonsterPatternComponent::GetAvailablePatterns() const
{
	TArray<FName> AvailablePatterns;

	const FMonsterPhasePatterns* CurrentPhaseData = GetCurrentPhasePatterns();
	if (CurrentPhaseData)
	{
		for (const FMonsterAttackPattern& Pattern : CurrentPhaseData->AttackPatterns)
		{
			if (CanExecutePattern(Pattern))
			{
				AvailablePatterns.Add(Pattern.PatternName);
			}
		}
	}

	return AvailablePatterns;
}

const FMonsterAttackPattern* UHarmoniaMonsterPatternComponent::GetPatternConfig(FName PatternName) const
{
	const FMonsterPhasePatterns* CurrentPhaseData = GetCurrentPhasePatterns();
	if (CurrentPhaseData)
	{
		for (const FMonsterAttackPattern& Pattern : CurrentPhaseData->AttackPatterns)
		{
			if (Pattern.PatternName == PatternName)
			{
				return &Pattern;
			}
		}
	}
	return nullptr;
}

const FMonsterPhasePatterns* UHarmoniaMonsterPatternComponent::GetCurrentPhasePatterns() const
{
	if (!MonsterOwner)
	{
		return nullptr;
	}
	
	// Default to phase 0 for non-boss monsters
	int32 CurrentPhase = 0;
	
	// Check if owner is a boss and get current phase
	if (AHarmoniaBossMonster* BossMonster = Cast<AHarmoniaBossMonster>(MonsterOwner))
	{
		CurrentPhase = BossMonster->GetCurrentPhase();
	}
	
	if (Phases.IsValidIndex(CurrentPhaseIndex)) 
	{ 
		return &Phases[CurrentPhaseIndex]; 
	} 
	return nullptr;
}

bool UHarmoniaMonsterPatternComponent::IsPatternOnCooldown(FName PatternName) const
{
	const float* Cooldown = PatternCooldowns.Find(PatternName);
	return Cooldown && *Cooldown > 0.0f;
}

float UHarmoniaMonsterPatternComponent::GetPatternCooldownRemaining(FName PatternName) const
{
	const float* Cooldown = PatternCooldowns.Find(PatternName);
	return Cooldown ? *Cooldown : 0.0f;
}

bool UHarmoniaMonsterPatternComponent::HasAnyAvailablePattern(int32 Category) const
{
	const FMonsterPhasePatterns* CurrentPhaseData = GetCurrentPhasePatterns();
	if (!CurrentPhaseData)
	{
		return false;
	}

	for (const FMonsterAttackPattern& Pattern : CurrentPhaseData->AttackPatterns)
	{
		// Check category if specified (-1 means any)
		if (Category >= 0 && static_cast<int32>(Pattern.PatternCategory) != Category)
		{
			continue;
		}

		if (CanExecutePattern(Pattern))
		{
			return true;
		}
	}
	return false;
}

float UHarmoniaMonsterPatternComponent::GetShortestCooldownRemaining(int32 Category) const
{
	float ShortestCooldown = TNumericLimits<float>::Max();
	bool bFoundAny = false;

	const FMonsterPhasePatterns* CurrentPhaseData = GetCurrentPhasePatterns();
	if (!CurrentPhaseData)
	{
		return 0.0f;
	}

	for (const FMonsterAttackPattern& Pattern : CurrentPhaseData->AttackPatterns)
	{
		// Check category if specified (-1 means any)
		if (Category >= 0 && static_cast<int32>(Pattern.PatternCategory) != Category)
		{
			continue;
		}

		float RemainingCooldown = GetPatternCooldownRemaining(Pattern.PatternName);
		if (RemainingCooldown > 0.0f && RemainingCooldown < ShortestCooldown)
		{
			ShortestCooldown = RemainingCooldown;
			bFoundAny = true;
		}
	}

	return bFoundAny ? ShortestCooldown : 0.0f;
}


//~=============================================================================
// Pattern Execution Implementation
//~=============================================================================

void UHarmoniaMonsterPatternComponent::StartPatternExecution(const FMonsterAttackPattern& Pattern)
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

	// Apply pattern start effects (abilities, GE, tags)
	ApplyPatternEffects(Pattern);

	// Broadcast pattern start
	OnPatternExecutionStart.Broadcast(Pattern.PatternName);

	// Play start montage if set (for phase transitions, intro animations, etc.)
	if (Pattern.StartMontage)
	{
		PlayStartMontage(Pattern);
		return; // Abilities will execute after montage completes
	}

	// Execute abilities based on mode
	ExecutePatternAbilities();
}

void UHarmoniaMonsterPatternComponent::ExecutePatternAbilities()
{
	switch (ActivePattern.ExecutionMode)
	{
	case EMonsterPatternExecutionMode::Single:
	case EMonsterPatternExecutionMode::Sequence:
		ExecuteNextAbility();
		break;

	case EMonsterPatternExecutionMode::Simultaneous:
		for (int32 i = 0; i < ActivePattern.PatternAbilities.Num(); ++i)
		{
			ExecuteAbilityAtIndex(i);
		}
		CompletePatternExecution();
		break;

	case EMonsterPatternExecutionMode::Random:
		if (ActivePattern.PatternAbilities.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, ActivePattern.PatternAbilities.Num() - 1);
			ExecuteAbilityAtIndex(RandomIndex);
		}
		CompletePatternExecution();
		break;
	}
}

void UHarmoniaMonsterPatternComponent::ExecuteNextAbility()
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
					&UHarmoniaMonsterPatternComponent::OnRepeatDelayComplete,
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

	// For sequence mode with multiple abilities, set up delay for next ability
	// Otherwise, pattern completion will be handled by OnAbilityEnded callback
	if (CurrentAbilityIndex < ActivePattern.PatternAbilities.Num())
	{
		if (ActivePattern.AbilityDelay > 0.0f && GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				AbilityDelayTimerHandle,
				this,
				&UHarmoniaMonsterPatternComponent::OnAbilityDelayComplete,
				ActivePattern.AbilityDelay,
				false
			);
		}
		// Note: Don't call ExecuteNextAbility here - wait for current ability to end first
		// The OnAbilityEnded callback will handle progression
	}
	// Pattern completion is now handled by OnAbilityEnded callback
	// This prevents instant pattern completion before ability actually finishes
}

void UHarmoniaMonsterPatternComponent::ExecuteAbilityAtIndex(int32 AbilityIndex)
{
	if (!MonsterOwner)
	{
		return;
	}

	if (!ActivePattern.PatternAbilities.IsValidIndex(AbilityIndex))
	{
		return;
	}

	UAbilitySystemComponent* ASC = MonsterOwner->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	TSubclassOf<ULyraGameplayAbility> AbilityClass = ActivePattern.PatternAbilities[AbilityIndex];
	if (!AbilityClass)
	{
		HandleAbilityActivationFailure();
		return;
	}

	// Bind to ability ended callback (only once per pattern)
	if (!AbilityEndedDelegateHandle.IsValid())
	{
		AbilityEndedDelegateHandle = ASC->OnAbilityEnded.AddUObject(this, &UHarmoniaMonsterPatternComponent::OnAbilityEnded);
	}

	// Try to activate the ability
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AbilityClass);
	bool bSuccess = false;
	
	if (Spec)
	{
		bSuccess = ASC->TryActivateAbility(Spec->Handle);
	}
	else
	{
		// Ability not granted, try to grant and activate it temporarily
		FGameplayAbilitySpec TempSpec(AbilityClass, 1, INDEX_NONE, MonsterOwner);
		FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(TempSpec);
		bSuccess = ASC->TryActivateAbility(Handle);
		
		// Track temporarily granted abilities for cleanup
		TemporarilyGrantedAbilities.Add(Handle);
	}

	if (bSuccess)
	{
		PendingAbilityCount++;
	}
	else
	{
		// Ability failed to activate - proceed to next step or complete pattern
		HandleAbilityActivationFailure();
	}
}

void UHarmoniaMonsterPatternComponent::CompletePatternExecution()
{
	FName CompletedPattern = CurrentPatternName;

	// Unbind ability ended delegate
	if (AbilityEndedDelegateHandle.IsValid() && MonsterOwner)
	{
		if (UAbilitySystemComponent* ASC = MonsterOwner->GetAbilitySystemComponent())
		{
			ASC->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
		}
		AbilityEndedDelegateHandle.Reset();
	}
	PendingAbilityCount = 0;

	// Start cooldown
	if (ActivePattern.Cooldown > 0.0f)
	{
		PatternCooldowns.Add(CurrentPatternName, ActivePattern.Cooldown);
	}

	// Clean up temporarily granted abilities to prevent memory leaks
	if (MonsterOwner)
	{
		if (UAbilitySystemComponent* ASC = MonsterOwner->GetAbilitySystemComponent())
		{
			for (const FGameplayAbilitySpecHandle& TempHandle : TemporarilyGrantedAbilities)
			{
				if (TempHandle.IsValid())
				{
					ASC->ClearAbility(TempHandle);
				}
			}
		}
	}
	TemporarilyGrantedAbilities.Empty();

	// Reset state
	bIsExecutingPattern = false;
	CurrentPatternName = NAME_None;
	CurrentAbilityIndex = 0;
	CurrentRepeatCount = 0;

	// Broadcast pattern end
	OnPatternExecutionEnd.Broadcast(CompletedPattern);
}

void UHarmoniaMonsterPatternComponent::OnAbilityEnded(const FAbilityEndedData& AbilityEndedData)
{
	if (!bIsExecutingPattern)
	{
		return; // Not executing a pattern, ignore
	}

	// Decrement pending ability count
	PendingAbilityCount = FMath::Max(0, PendingAbilityCount - 1);

	// If there are still pending abilities, wait for them
	if (PendingAbilityCount > 0)
	{
		return;
	}

	// All abilities for current step have completed
	// Check if we need to execute more abilities in this pattern
	if (CurrentAbilityIndex < ActivePattern.PatternAbilities.Num())
	{
		// More abilities to execute - check for delay or execute immediately
		if (ActivePattern.AbilityDelay > 0.0f && GetWorld())
		{
			// Timer already set in ExecuteNextAbility, just return
			return;
		}
		else
		{
			// Execute next ability immediately
			ExecuteNextAbility();
		}
	}
	else
	{
		// All abilities executed, check for repeat
		CurrentRepeatCount++;
		if (CurrentRepeatCount < ActivePattern.RepeatCount)
		{
			// Need to repeat pattern
			CurrentAbilityIndex = 0;
			if (ActivePattern.RepeatDelay > 0.0f && GetWorld())
			{
				GetWorld()->GetTimerManager().SetTimer(
					RepeatDelayTimerHandle,
					this,
					&UHarmoniaMonsterPatternComponent::OnRepeatDelayComplete,
					ActivePattern.RepeatDelay,
					false
				);
			}
			else
			{
				ExecuteNextAbility();
			}
		}
		else
		{
			// Pattern complete
			CompletePatternExecution();
		}
	}
}

bool UHarmoniaMonsterPatternComponent::CanExecutePattern(const FMonsterAttackPattern& Pattern) const
{
	if (!MonsterOwner)
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

	// Check required tags and blocked tags
	ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(MonsterOwner->GetAbilitySystemComponent());
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

FName UHarmoniaMonsterPatternComponent::SelectRandomPattern() const
{
	TArray<FName> AvailablePatternNames;
	TArray<float> Weights;

	// Collect available patterns and their weights from current phase
	const FMonsterPhasePatterns* CurrentPhaseData = GetCurrentPhasePatterns();
	if (CurrentPhaseData)
	{
		for (const FMonsterAttackPattern& Pattern : CurrentPhaseData->AttackPatterns)
		{
			if (CanExecutePattern(Pattern))
			{
				AvailablePatternNames.Add(Pattern.PatternName);
				Weights.Add(Pattern.RandomWeight);
			}
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

void UHarmoniaMonsterPatternComponent::UpdateCooldowns(float DeltaTime)
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

void UHarmoniaMonsterPatternComponent::OnAbilityDelayComplete()
{
	ExecuteNextAbility();
}

void UHarmoniaMonsterPatternComponent::OnRepeatDelayComplete()
{
	ExecuteNextAbility();
}

void UHarmoniaMonsterPatternComponent::HandleAbilityActivationFailure()
{
	// This function handles the case when an ability fails to activate
	// Without this, the pattern would get stuck forever with bIsExecutingPattern = true
	
	// Check if there are more abilities in the current sequence
	if (CurrentAbilityIndex < ActivePattern.PatternAbilities.Num())
	{
		// More abilities to try - proceed to next one
		if (ActivePattern.AbilityDelay > 0.0f && GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(
				AbilityDelayTimerHandle,
				this,
				&UHarmoniaMonsterPatternComponent::OnAbilityDelayComplete,
				ActivePattern.AbilityDelay,
				false
			);
		}
		else
		{
			ExecuteNextAbility();
		}
	}
	else if (PendingAbilityCount == 0)
	{
		// No more abilities and nothing pending - handle repeat or complete
		CurrentRepeatCount++;
		if (CurrentRepeatCount < ActivePattern.RepeatCount)
		{
			// Need to repeat pattern
			CurrentAbilityIndex = 0;
			
			if (ActivePattern.RepeatDelay > 0.0f && GetWorld())
			{
				GetWorld()->GetTimerManager().SetTimer(
					RepeatDelayTimerHandle,
					this,
					&UHarmoniaMonsterPatternComponent::OnRepeatDelayComplete,
					ActivePattern.RepeatDelay,
					false
				);
			}
			else
			{
			ExecuteNextAbility();
			}
		}
		else
		{
			// Pattern complete
			CompletePatternExecution();
		}
	}
	// If some abilities are still pending, just wait for them
}

//~=============================================================================
// Start Montage (패턴 시작 시 재생)
//~=============================================================================
void UHarmoniaMonsterPatternComponent::PlayStartMontage(const FMonsterAttackPattern& Pattern)
{
	if (!MonsterOwner || !Pattern.StartMontage)
	{
		ExecutePatternAbilities();
		return;
	}

	float MontageDuration = 1.0f;
	
	if (UAnimInstance* AnimInstance = MonsterOwner->GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(Pattern.StartMontage);
		MontageDuration = Pattern.StartMontage->GetPlayLength();
	}

	// Set timer to execute abilities after montage completes
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			StartMontageTimerHandle,
			this,
			&UHarmoniaMonsterPatternComponent::OnStartMontageComplete,
			MontageDuration,
			false
		);
	}
}

void UHarmoniaMonsterPatternComponent::OnStartMontageComplete()
{
	// Execute pattern abilities after start montage finishes
	ExecutePatternAbilities();
}

//~=============================================================================
// Pattern Effects
//~=============================================================================
void UHarmoniaMonsterPatternComponent::ApplyPatternEffects(const FMonsterAttackPattern& Pattern)
{
	if (!MonsterOwner)
	{
		return;
	}

	ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(MonsterOwner->GetAbilitySystemComponent());
	if (!ASC)
	{
		return;
	}

	// Remove abilities specified
	for (TSubclassOf<UGameplayAbility> AbilityClass : Pattern.AbilitiesToRemove)
	{
		if (UClass* AbilityClassPtr = AbilityClass.Get())
		{
			FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AbilityClassPtr);
			if (Spec)
			{
				ASC->ClearAbility(Spec->Handle);
			}
		}
	}

	// Grant abilities specified
	for (TSubclassOf<UGameplayAbility> AbilityClass : Pattern.AbilitiesToGrant)
	{
		if (UClass* AbilityClassPtr = AbilityClass.Get())
		{
			FGameplayAbilitySpec AbilitySpec(AbilityClassPtr, 1, INDEX_NONE, MonsterOwner);
			FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AbilitySpec);
			TemporarilyGrantedAbilities.Add(Handle);
		}
	}

	// Remove specified tags
	if (Pattern.TagsToRemove.Num() > 0)
	{
		ASC->RemoveLooseGameplayTags(Pattern.TagsToRemove);
	}

	// Apply specified tags
	if (Pattern.TagsToApply.Num() > 0)
	{
		ASC->AddLooseGameplayTags(Pattern.TagsToApply);
	}

	// Apply gameplay effects
	for (TSubclassOf<UGameplayEffect> EffectClass : Pattern.PatternEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddSourceObject(MonsterOwner);

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1, EffectContext);
			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				ActivePatternEffects.Add(ActiveHandle);
			}
		}
	}
}

void UHarmoniaMonsterPatternComponent::RemovePatternEffects()
{
	if (!MonsterOwner)
	{
		return;
	}

	UAbilitySystemComponent* ASC = MonsterOwner->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Remove active effects
	for (FActiveGameplayEffectHandle& EffectHandle : ActivePatternEffects)
	{
		if (EffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(EffectHandle);
		}
	}
	ActivePatternEffects.Empty();

	// Remove applied tags
	if (ActivePattern.TagsToApply.Num() > 0)
	{
		ASC->RemoveLooseGameplayTags(ActivePattern.TagsToApply);
	}
}





// ============================================================================
// Phase Management
// ============================================================================

void UHarmoniaMonsterPatternComponent::UpdatePhase(float HealthPercent)
{
    if (Phases.Num() == 0)
    {
        return;
    }

    // Find the appropriate phase based on health percentage
    int32 NewPhaseIndex = 0;
    for (int32 i = 0; i < Phases.Num(); i++)
    {
        if (HealthPercent <= Phases[i].HealthThreshold)
        {
            NewPhaseIndex = i;
        }
    }

    // Check if phase changed
    if (NewPhaseIndex != CurrentPhaseIndex)
    {
        int32 OldPhase = CurrentPhaseIndex;
        CurrentPhaseIndex = NewPhaseIndex;

        // Apply phase enter effects
        ApplyPhaseEnterEffects(Phases[CurrentPhaseIndex]);

        // Broadcast phase change
        OnPhaseChanged.Broadcast(OldPhase, CurrentPhaseIndex);
    }
}

const FMonsterPhasePatterns& UHarmoniaMonsterPatternComponent::GetCurrentPhaseData() const
{
    static FMonsterPhasePatterns EmptyPhase;
    if (Phases.IsValidIndex(CurrentPhaseIndex))
    {
        return Phases[CurrentPhaseIndex];
    }
    return EmptyPhase;
}

void UHarmoniaMonsterPatternComponent::ApplyPhaseEnterEffects(const FMonsterPhasePatterns& Phase)
{
    if (!MonsterOwner)
    {
        return;
    }

    UAbilitySystemComponent* ASC = MonsterOwner->GetAbilitySystemComponent();
    if (!ASC)
    {
        return;
    }

    // Play phase enter montage
    if (Phase.PhaseEnterMontage)
    {
        if (USkeletalMeshComponent* Mesh = MonsterOwner->GetMesh())
        {
            if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
            {
                AnimInstance->Montage_Play(Phase.PhaseEnterMontage);
            }
        }
    }

    // Grant abilities
    for (const TSubclassOf<UGameplayAbility>& AbilityClass : Phase.AbilitiesToGrant)
    {
        if (AbilityClass)
        {
            FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, MonsterOwner);
            ASC->GiveAbility(AbilitySpec);
        }
    }

    // Remove abilities
    for (const TSubclassOf<UGameplayAbility>& AbilityClass : Phase.AbilitiesToRemove)
    {
        if (AbilityClass)
        {
            FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AbilityClass);
            if (Spec)
            {
                ASC->ClearAbility(Spec->Handle);
            }
        }
    }

    // Apply phase effects
    for (const TSubclassOf<UGameplayEffect>& EffectClass : Phase.PhaseEffects)
    {
        if (EffectClass)
        {
            FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
            Context.AddSourceObject(MonsterOwner);
            FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1, Context);
            if (SpecHandle.IsValid())
            {
                ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            }
        }
    }

    // Apply tags
    if (Phase.TagsToApply.Num() > 0)
    {
        ASC->AddLooseGameplayTags(Phase.TagsToApply);
    }

    // Remove tags
    if (Phase.TagsToRemove.Num() > 0)
    {
        ASC->RemoveLooseGameplayTags(Phase.TagsToRemove);
    }
}


// ============================================================================
// Contextual Pattern Execution
// ============================================================================

bool UHarmoniaMonsterPatternComponent::ExecuteContextualPattern(EPatternCategory Category, AActor* Target)
{
    if (!MonsterOwner || !Target)
    {
        return false;
    }

    // Get patterns matching category and conditions
    TArray<int32> AvailableIndices = GetAvailablePatternsForCategory(Category, Target);
    if (AvailableIndices.Num() == 0)
    {
        return false;
    }

    // Weighted random selection
    const FMonsterPhasePatterns* CurrentPhaseData = GetCurrentPhasePatterns();
    if (!CurrentPhaseData)
    {
        return false;
    }

    TArray<float> Weights;
    float TotalWeight = 0.0f;
    for (int32 Index : AvailableIndices)
    {
        float Weight = CurrentPhaseData->AttackPatterns[Index].RandomWeight;
        Weights.Add(Weight);
        TotalWeight += Weight;
    }

    int32 SelectedIndex = AvailableIndices[0];
    if (TotalWeight > 0.0f)
    {
        float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
        float CurrentWeight = 0.0f;
        for (int32 i = 0; i < AvailableIndices.Num(); ++i)
        {
            CurrentWeight += Weights[i];
            if (RandomValue <= CurrentWeight)
            {
                SelectedIndex = AvailableIndices[i];
                break;
            }
        }
    }

    // Apply pattern state tag before execution
    ApplyPatternStateTag(Category);

    // Execute the selected pattern
    return ExecutePatternByIndex(SelectedIndex);
}

TArray<int32> UHarmoniaMonsterPatternComponent::GetAvailablePatternsForCategory(EPatternCategory Category, AActor* Target) const
{
    TArray<int32> AvailableIndices;

    const FMonsterPhasePatterns* CurrentPhaseData = GetCurrentPhasePatterns();
    if (!CurrentPhaseData)
    {
        return AvailableIndices;
    }

    for (int32 i = 0; i < CurrentPhaseData->AttackPatterns.Num(); ++i)
    {
        const FMonsterAttackPattern& Pattern = CurrentPhaseData->AttackPatterns[i];
        
        // Check category match
        if (Pattern.PatternCategory != Category)
        {
            continue;
        }

        // Check if pattern can be executed (cooldown, tags, etc.)
        if (!CanExecutePattern(Pattern))
        {
            continue;
        }

        // Check contextual conditions
        if (!EvaluatePatternConditions(Pattern, Target))
        {
            continue;
        }

        AvailableIndices.Add(i);
    }

    return AvailableIndices;
}

bool UHarmoniaMonsterPatternComponent::EvaluatePatternConditions(const FMonsterAttackPattern& Pattern, AActor* Target) const
{
    if (!MonsterOwner || !Target)
    {
        return false;
    }

    // Distance check
    float Distance = GetDistanceToTarget(Target);
    if (Pattern.MinDistance > 0.0f && Distance < Pattern.MinDistance)
    {
        return false;
    }
    if (Pattern.MaxDistance > 0.0f && Distance > Pattern.MaxDistance)
    {
        return false;
    }

    // Height check
    float HeightDiff = GetHeightDifferenceToTarget(Target);
    if (HeightDiff < Pattern.MinHeightDifference || HeightDiff > Pattern.MaxHeightDifference)
    {
        return false;
    }

    // Direction check
    if (Pattern.DirectionRequirement == EPatternDirectionRequirement::FacingTarget)
    {
        if (!IsFacingTarget(Target, 90.0f))
        {
            return false;
        }
    }

    return true;
}

float UHarmoniaMonsterPatternComponent::GetDistanceToTarget(AActor* Target) const
{
    if (!MonsterOwner || !Target)
    {
        return 0.0f;
    }
    return FVector::Dist(MonsterOwner->GetActorLocation(), Target->GetActorLocation());
}

float UHarmoniaMonsterPatternComponent::GetHeightDifferenceToTarget(AActor* Target) const
{
    if (!MonsterOwner || !Target)
    {
        return 0.0f;
    }
    return Target->GetActorLocation().Z - MonsterOwner->GetActorLocation().Z;
}

bool UHarmoniaMonsterPatternComponent::IsFacingTarget(AActor* Target, float AngleTolerance) const
{
    if (!MonsterOwner || !Target)
    {
        return false;
    }

    FVector ToTarget = (Target->GetActorLocation() - MonsterOwner->GetActorLocation()).GetSafeNormal2D();
    FVector Forward = MonsterOwner->GetActorForwardVector().GetSafeNormal2D();
    
    float DotProduct = FVector::DotProduct(Forward, ToTarget);
    float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
    
    return AngleDegrees <= (AngleTolerance / 2.0f);
}

// ============================================================================
// State Tag Management
// ============================================================================

void UHarmoniaMonsterPatternComponent::ApplyPatternStateTag(EPatternCategory Category)
{
    if (!MonsterOwner)
    {
        return;
    }

    UAbilitySystemComponent* ASC = MonsterOwner->GetAbilitySystemComponent();
    if (!ASC)
    {
        return;
    }

    switch (Category)
    {
        case EPatternCategory::Attack:
            ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Attacking);
            break;
        case EPatternCategory::Defense:
            ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Defending);
            break;
        case EPatternCategory::Evasion:
            ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Evading);
            break;
        case EPatternCategory::Movement:
            ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Moving);
            break;
    }
}

void UHarmoniaMonsterPatternComponent::RemovePatternStateTag(EPatternCategory Category)
{
    if (!MonsterOwner)
    {
        return;
    }

    UAbilitySystemComponent* ASC = MonsterOwner->GetAbilitySystemComponent();
    if (!ASC)
    {
        return;
    }

    switch (Category)
    {
        case EPatternCategory::Attack:
            ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Attacking);
            break;
        case EPatternCategory::Defense:
            ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Defending);
            break;
        case EPatternCategory::Evasion:
            ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Evading);
            break;
        case EPatternCategory::Movement:
            ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Moving);
            break;
    }
}

void UHarmoniaMonsterPatternComponent::SetCombatState(bool bInCombat)
{
    if (!MonsterOwner)
    {
        return;
    }

    UAbilitySystemComponent* ASC = MonsterOwner->GetAbilitySystemComponent();
    if (!ASC)
    {
        return;
    }

    if (bInCombat)
    {
        ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Idle);
        ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Monster_InCombat);
    }
    else
    {
        ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Monster_InCombat);
        ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Idle);
    }
}

void UHarmoniaMonsterPatternComponent::SetGroggyState(bool bGroggy, bool bRecoverable)
{
    if (!MonsterOwner)
    {
        return;
    }

    UAbilitySystemComponent* ASC = MonsterOwner->GetAbilitySystemComponent();
    if (!ASC)
    {
        return;
    }

    if (bGroggy)
    {
        ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Groggy);
        if (bRecoverable)
        {
            ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Groggy_Recoverable);
            ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Groggy_Execution);
        }
        else
        {
            ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Groggy_Recoverable);
            ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Groggy_Execution);
        }
    }
    else
    {
        ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Groggy);
        ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Groggy_Recoverable);
        ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Monster_Groggy_Execution);
    }
}
