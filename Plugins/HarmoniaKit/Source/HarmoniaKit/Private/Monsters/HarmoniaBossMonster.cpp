// Copyright 2025 Snow Game Studio.

#include "Monsters/HarmoniaBossMonster.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "Components/AudioComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AHarmoniaBossMonster::AHarmoniaBossMonster(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Bosses typically have larger aggro range
	// This can be overridden in MonsterData
}

void AHarmoniaBossMonster::BeginPlay()
{
	Super::BeginPlay();

	// Initialize to first phase if we have phases
	if (HasAuthority() && MonsterData && MonsterData->bIsBoss && MonsterData->BossPhases.Num() > 0)
	{
		CurrentPhaseIndex = 0;
		ApplyPhaseEffects(MonsterData->BossPhases[0]);
	}
}

void AHarmoniaBossMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check for phase transitions based on health
	if (HasAuthority() && bEncounterStarted && !bInPhaseTransition && !IsDead_Implementation())
	{
		CheckPhaseTransition();
	}
}

void AHarmoniaBossMonster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHarmoniaBossMonster, CurrentPhaseIndex);
	DOREPLIFETIME(AHarmoniaBossMonster, bInPhaseTransition);
	DOREPLIFETIME(AHarmoniaBossMonster, bEncounterStarted);
}

void AHarmoniaBossMonster::InitializeMonster(UHarmoniaMonsterData* InMonsterData, int32 InLevel)
{
	Super::InitializeMonster(InMonsterData, InLevel);

	// Validate boss data
	if (InMonsterData && InMonsterData->bIsBoss)
	{
		// Sort phases by health threshold (highest to lowest)
		if (InMonsterData->BossPhases.Num() > 0)
		{
			InMonsterData->BossPhases.Sort([](const FHarmoniaBossPhase& A, const FHarmoniaBossPhase& B)
			{
				return A.HealthThreshold > B.HealthThreshold;
			});
		}
	}
}

void AHarmoniaBossMonster::OnDeath_Implementation(AActor* Killer)
{
	// End encounter before death
	EndBossEncounter();

	Super::OnDeath_Implementation(Killer);
}

// ============================================================================
// Phase System
// ============================================================================

FHarmoniaBossPhase AHarmoniaBossMonster::GetCurrentPhaseData() const
{
	if (MonsterData && MonsterData->bIsBoss && CurrentPhaseIndex >= 0 && CurrentPhaseIndex < MonsterData->BossPhases.Num())
	{
		return MonsterData->BossPhases[CurrentPhaseIndex];
	}
	return FHarmoniaBossPhase();
}

int32 AHarmoniaBossMonster::GetTotalPhases() const
{
	if (MonsterData && MonsterData->bIsBoss)
	{
		return MonsterData->BossPhases.Num();
	}
	return 0;
}

void AHarmoniaBossMonster::TriggerPhaseChange(int32 NewPhaseIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!MonsterData || !MonsterData->bIsBoss)
	{
		return;
	}

	if (NewPhaseIndex < 0 || NewPhaseIndex >= MonsterData->BossPhases.Num())
	{
		return;
	}

	if (NewPhaseIndex == CurrentPhaseIndex)
	{
		return;
	}

	ChangeToPhase(NewPhaseIndex);
}

// ============================================================================
// Boss Functions
// ============================================================================

void AHarmoniaBossMonster::StartBossEncounter()
{
	if (bEncounterStarted)
	{
		return;
	}

	bEncounterStarted = true;

	// Play boss music
	if (HasAuthority())
	{
		PlayBossMusic();
	}

	// Broadcast to UI systems that boss encounter started
	// This would typically trigger boss health bar display, etc.
	UE_LOG(LogTemp, Log, TEXT("Boss Encounter Started: %s"), *GetMonsterName_Implementation().ToString());
}

void AHarmoniaBossMonster::EndBossEncounter()
{
	if (!bEncounterStarted)
	{
		return;
	}

	bEncounterStarted = false;

	// Stop boss music
	if (HasAuthority())
	{
		StopBossMusic();
	}

	UE_LOG(LogTemp, Log, TEXT("Boss Encounter Ended: %s"), *GetMonsterName_Implementation().ToString());
}

TArray<FHarmoniaMonsterAttackPattern> AHarmoniaBossMonster::GetCurrentPhaseAttacks() const
{
	FHarmoniaBossPhase CurrentPhase = GetCurrentPhaseData();
	if (CurrentPhase.PhaseAttacks.Num() > 0)
	{
		return CurrentPhase.PhaseAttacks;
	}

	// Fallback to base attacks if no phase-specific attacks
	if (MonsterData)
	{
		return MonsterData->AttackPatterns;
	}

	return TArray<FHarmoniaMonsterAttackPattern>();
}

// ============================================================================
// Protected Functions
// ============================================================================

void AHarmoniaBossMonster::CheckPhaseTransition()
{
	if (!MonsterData || !MonsterData->bIsBoss || !AttributeSet)
	{
		return;
	}

	if (MonsterData->BossPhases.Num() == 0)
	{
		return;
	}

	// Get current health percentage
	float CurrentHealth = AttributeSet->GetHealth();
	float MaxHealth = AttributeSet->GetMaxHealth();
	float HealthPercentage = MaxHealth > 0 ? (CurrentHealth / MaxHealth) : 0.0f;

	// Check if we should transition to next phase
	for (int32 i = CurrentPhaseIndex + 1; i < MonsterData->BossPhases.Num(); i++)
	{
		const FHarmoniaBossPhase& Phase = MonsterData->BossPhases[i];

		if (HealthPercentage <= Phase.HealthThreshold)
		{
			ChangeToPhase(i);
			break;
		}
	}
}

void AHarmoniaBossMonster::ChangeToPhase(int32 NewPhaseIndex)
{
	if (!MonsterData || !MonsterData->bIsBoss)
	{
		return;
	}

	if (NewPhaseIndex < 0 || NewPhaseIndex >= MonsterData->BossPhases.Num())
	{
		return;
	}

	if (bInPhaseTransition)
	{
		return;
	}

	int32 OldPhaseIndex = CurrentPhaseIndex;
	const FHarmoniaBossPhase& NewPhaseData = MonsterData->BossPhases[NewPhaseIndex];

	// Start phase transition
	bInPhaseTransition = true;

	// Make boss invulnerable during transition if configured
	if (NewPhaseData.bInvulnerableDuringTransition && AbilitySystemComponent)
	{
		// Add invulnerability tag (you'd need to define this tag in your project)
		// FGameplayTag InvulnerabilityTag = FGameplayTag::RequestGameplayTag(FName("Status.Invulnerable"));
		// AbilitySystemComponent->AddLooseGameplayTag(InvulnerabilityTag);
	}

	// Play transition animation
	PlayPhaseTransitionAnimation(NewPhaseData);

	// Spawn transition VFX - Particle System
	if (NewPhaseData.TransitionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			NewPhaseData.TransitionEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}

	// Spawn transition VFX - Niagara
	if (NewPhaseData.TransitionNiagaraEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			NewPhaseData.TransitionNiagaraEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}

	// Update phase index
	CurrentPhaseIndex = NewPhaseIndex;

	// Broadcast phase change
	OnBossPhaseChanged.Broadcast(OldPhaseIndex, NewPhaseIndex, NewPhaseData);

	UE_LOG(LogTemp, Log, TEXT("Boss Phase Change: %s -> Phase %d (%s)"),
		*GetMonsterName_Implementation().ToString(),
		NewPhaseIndex,
		*NewPhaseData.PhaseName.ToString());

	// Calculate transition duration (use override if set, otherwise use montage length)
	float TransitionDuration = NewPhaseData.TransitionDuration > 0.0f
		? NewPhaseData.TransitionDuration
		: (NewPhaseData.TransitionMontage ? NewPhaseData.TransitionMontage->GetPlayLength() : 1.0f);

	GetWorld()->GetTimerManager().SetTimer(
		PhaseTransitionTimerHandle,
		this,
		&AHarmoniaBossMonster::OnPhaseTransitionComplete,
		TransitionDuration,
		false
	);
}

void AHarmoniaBossMonster::ApplyPhaseEffects(const FHarmoniaBossPhase& PhaseData)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Remove previous phase effects
	RemovePreviousPhaseEffects();

	// Remove specified abilities
	for (TSubclassOf<class ULyraGameplayAbility> AbilityClass : PhaseData.AbilitiesToRemove)
	{
		if (UClass* AbilityClassPtr = AbilityClass.Get())
		{
			FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClassPtr);
			if (Spec)
			{
				AbilitySystemComponent->ClearAbility(Spec->Handle);
			}
		}
	}

	// Grant phase abilities
	for (TSubclassOf<class ULyraGameplayAbility> AbilityClass : PhaseData.PhaseAbilities)
	{
		if (UClass* AbilityClassPtr = AbilityClass.Get())
		{
			FGameplayAbilitySpec AbilitySpec(AbilityClassPtr, MonsterLevel, INDEX_NONE, this);
			FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(AbilitySpec);
			GrantedAbilityHandles.Add(Handle);
		}
	}

	// Apply gameplay tags
	if (PhaseData.PhaseTags.Num() > 0)
	{
		AbilitySystemComponent->AddLooseGameplayTags(PhaseData.PhaseTags);
	}

	// Remove specified tags
	if (PhaseData.TagsToRemove.Num() > 0)
	{
		AbilitySystemComponent->RemoveLooseGameplayTags(PhaseData.TagsToRemove);
	}

	// Apply new phase effects
	for (TSubclassOf<UGameplayEffect> EffectClass : PhaseData.PhaseEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
			EffectContext.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, MonsterLevel, EffectContext);
			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				ActivePhaseEffects.Add(ActiveHandle);
			}
		}
	}

	// Apply movement speed multiplier
	if (PhaseData.MovementSpeedMultiplier != 1.0f)
	{
		if (GetCharacterMovement())
		{
			float BaseSpeed = MonsterData->BaseStats.BaseMovementSpeed;
			GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * PhaseData.MovementSpeedMultiplier;
		}
	}

	// Apply damage multiplier via gameplay effect
	if (PhaseData.DamageMultiplier != 1.0f)
	{
		// Create dynamic damage multiplier effect
		UGameplayEffect* DamageEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("BossPhaseDamageMultiplier")));
		DamageEffect->DurationPolicy = EGameplayEffectDurationType::Infinite;

		// Add modifier for attack power
		int32 Idx = DamageEffect->Modifiers.Num();
		DamageEffect->Modifiers.SetNum(Idx + 1);
		FGameplayModifierInfo& DamageModifier = DamageEffect->Modifiers[Idx];
		DamageModifier.ModifierMagnitude = FScalableFloat(PhaseData.DamageMultiplier);
		DamageModifier.ModifierOp = EGameplayModOp::Multiplicative;
		DamageModifier.Attribute = UHarmoniaAttributeSet::GetAttackPowerAttribute();

		// Apply the effect
		FGameplayEffectContextHandle DamageContext = AbilitySystemComponent->MakeEffectContext();
		DamageContext.AddSourceObject(this);
		FGameplayEffectSpecHandle DamageSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageEffect->GetClass(), MonsterLevel, DamageContext);
		if (DamageSpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
			ActivePhaseEffects.Add(ActiveHandle);
		}
	}

	// Apply defense multiplier via gameplay effect
	if (PhaseData.DefenseMultiplier != 1.0f)
	{
		// Create dynamic defense multiplier effect
		UGameplayEffect* DefenseEffect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("BossPhaseDefenseMultiplier")));
		DefenseEffect->DurationPolicy = EGameplayEffectDurationType::Infinite;

		// Add modifier for armor (defense)
		int32 Idx = DefenseEffect->Modifiers.Num();
		DefenseEffect->Modifiers.SetNum(Idx + 1);
		FGameplayModifierInfo& DefenseModifier = DefenseEffect->Modifiers[Idx];
		DefenseModifier.ModifierMagnitude = FScalableFloat(PhaseData.DefenseMultiplier);
		DefenseModifier.ModifierOp = EGameplayModOp::Multiplicative;
		DefenseModifier.Attribute = UHarmoniaAttributeSet::GetArmorAttribute();

		// Apply the effect
		FGameplayEffectContextHandle DefenseContext = AbilitySystemComponent->MakeEffectContext();
		DefenseContext.AddSourceObject(this);
		FGameplayEffectSpecHandle DefenseSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefenseEffect->GetClass(), MonsterLevel, DefenseContext);
		if (DefenseSpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*DefenseSpecHandle.Data.Get());
			ActivePhaseEffects.Add(ActiveHandle);
		}
	}
}

void AHarmoniaBossMonster::RemovePreviousPhaseEffects()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Remove previous phase effects
	for (FActiveGameplayEffectHandle& EffectHandle : ActivePhaseEffects)
	{
		if (EffectHandle.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(EffectHandle);
		}
	}
	ActivePhaseEffects.Empty();

	// Remove previous phase abilities
	for (FGameplayAbilitySpecHandle& AbilityHandle : GrantedAbilityHandles)
	{
		if (AbilityHandle.IsValid())
		{
			AbilitySystemComponent->ClearAbility(AbilityHandle);
		}
	}
	GrantedAbilityHandles.Empty();

	// Remove previous phase tags
	if (MonsterData && MonsterData->bIsBoss && CurrentPhaseIndex >= 0)
	{
		if (CurrentPhaseIndex < MonsterData->BossPhases.Num())
		{
			const FHarmoniaBossPhase& OldPhase = MonsterData->BossPhases[CurrentPhaseIndex];
			if (OldPhase.PhaseTags.Num() > 0)
			{
				AbilitySystemComponent->RemoveLooseGameplayTags(OldPhase.PhaseTags);
			}
		}
	}
}

void AHarmoniaBossMonster::PlayPhaseTransitionAnimation(const FHarmoniaBossPhase& PhaseData)
{
	if (PhaseData.TransitionMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(PhaseData.TransitionMontage);
		}
	}
}

void AHarmoniaBossMonster::OnPhaseTransitionComplete()
{
	bInPhaseTransition = false;

	// Remove invulnerability if it was applied
	const FHarmoniaBossPhase& CurrentPhase = GetCurrentPhaseData();
	if (CurrentPhase.bInvulnerableDuringTransition && AbilitySystemComponent)
	{
		// Remove invulnerability tag
		// FGameplayTag InvulnerabilityTag = FGameplayTag::RequestGameplayTag(FName("Status.Invulnerable"));
		// AbilitySystemComponent->RemoveLooseGameplayTag(InvulnerabilityTag);
	}

	// Apply phase effects
	ApplyPhaseEffects(CurrentPhase);

	// Update behavior tree if phase has custom BT
	UpdatePhaseBehaviorTree();
}

void AHarmoniaBossMonster::UpdatePhaseBehaviorTree()
{
	const FHarmoniaBossPhase& CurrentPhase = GetCurrentPhaseData();

	if (CurrentPhase.PhaseBehaviorTree)
	{
		// Switch to phase-specific behavior tree
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			AIController->RunBehaviorTree(CurrentPhase.PhaseBehaviorTree);
		}
	}
}

void AHarmoniaBossMonster::PlayBossMusic()
{
	USoundBase* MusicToPlay = BossMusic;

	// Use music from monster data if not overridden
	if (!MusicToPlay && MonsterData && MonsterData->bIsBoss)
	{
		MusicToPlay = MonsterData->BossMusic;
	}

	if (MusicToPlay)
	{
		BossMusicComponent = UGameplayStatics::SpawnSound2D(this, MusicToPlay, 1.0f, 1.0f, 0.0f, nullptr, true, false);
		bMusicPlaying = true;
	}
}

void AHarmoniaBossMonster::StopBossMusic()
{
	if (BossMusicComponent)
	{
		BossMusicComponent->Stop();
		BossMusicComponent = nullptr;
	}

	bMusicPlaying = false;
}

void AHarmoniaBossMonster::OnHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue)
{
	Super::OnHealthChanged(EffectInstigator, EffectCauser, EffectSpec, EffectMagnitude, OldValue, NewValue);

	// Start encounter on first damage taken
	if (!bEncounterStarted && NewValue < OldValue)
	{
		StartBossEncounter();
	}
}

void AHarmoniaBossMonster::OnRep_CurrentPhaseIndex(int32 OldPhaseIndex)
{
	// Client-side notification of phase change
	if (MonsterData && MonsterData->bIsBoss && CurrentPhaseIndex >= 0 && CurrentPhaseIndex < MonsterData->BossPhases.Num())
	{
		const FHarmoniaBossPhase& NewPhaseData = MonsterData->BossPhases[CurrentPhaseIndex];
		OnBossPhaseChanged.Broadcast(OldPhaseIndex, CurrentPhaseIndex, NewPhaseData);
	}
}
