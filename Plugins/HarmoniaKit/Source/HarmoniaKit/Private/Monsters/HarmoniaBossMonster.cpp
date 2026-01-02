// Copyright 2025 Snow Game Studio.

#include "Monsters/HarmoniaBossMonster.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "HarmoniaGameplayTags.h"
#include "Components/AudioComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "MotionWarpingComponent.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AHarmoniaBossMonster::AHarmoniaBossMonster(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// MotionWarpingComponent is created in parent AHarmoniaMonsterBase
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

TArray<TSubclassOf<UGameplayAbility>> AHarmoniaBossMonster::GetActiveAbilities() const
{
	// NOTE: ActiveAbilities는 이제 PatternComponent에서 관리합니다.
	// BT에서는 PatternComponent::GetAvailablePatterns()를 사용하세요.
	return TArray<TSubclassOf<UGameplayAbility>>();
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

	// Update phase index
	CurrentPhaseIndex = NewPhaseIndex;

	// Broadcast phase change
	OnBossPhaseChanged.Broadcast(OldPhaseIndex, NewPhaseIndex, NewPhaseData);

	UE_LOG(LogTemp, Log, TEXT("Boss Phase Change: %s -> Phase %d (%s)"),
		*GetMonsterName_Implementation().ToString(),
		NewPhaseIndex,
		*NewPhaseData.PhaseName.ToString());

	// NOTE: 페이즈 전환 연출, GE, 어빌리티 부여/제거, BT 변경 등은
	// PatternComponent에서 ExecutePhaseTransitionPattern()으로 처리합니다.
	// 여기서는 즉시 페이즈 전환 완료 처리
	bInPhaseTransition = false;
}

void AHarmoniaBossMonster::ApplyPhaseEffects(const FHarmoniaBossPhase& PhaseData)
{
	// NOTE: 어빌리티 부여/제거, GE, 태그 등은 이제 PatternComponent에서 관리합니다.
	// 페이즈 전환 패턴(bIsPhaseTransitionPattern=true)에서 처리하세요.
}

void AHarmoniaBossMonster::RemovePreviousPhaseEffects()
{
	// NOTE: PatternComponent::RemovePatternEffects()로 대체되었습니다.
}

void AHarmoniaBossMonster::PlayPhaseTransitionAnimation(const FHarmoniaBossPhase& PhaseData)
{
	// NOTE: 페이즈 전환 애니메이션은 PatternComponent에서 TransitionMontage로 처리합니다.
}

void AHarmoniaBossMonster::OnPhaseTransitionComplete()
{
	bInPhaseTransition = false;
}

void AHarmoniaBossMonster::UpdatePhaseBehaviorTree()
{
	// NOTE: BT 변경은 PatternComponent::PatternBehaviorTree로 처리합니다.
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
