// Copyright 2024 Snow Game Studio.

#include "Monsters/HarmoniaBossCharacter.h"
#include "Components/HarmoniaBossPhaseComponent.h"
#include "Components/HarmoniaBossPatternComponent.h"
#include "LyraHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"

AHarmoniaBossCharacter::ABossCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create boss phase component
	BossPhaseComponent = CreateDefaultSubobject<UHarmoniaBossPhaseComponent>(TEXT("BossPhaseComponent"));

	// Create boss pattern component
	BossPatternComponent = CreateDefaultSubobject<UHarmoniaBossPatternComponent>(TEXT("BossPatternComponent"));

	// Default phase configuration (3 phases at 66% and 33% health)
	PhaseHealthThresholds = {0.66f, 0.33f};

	// Boss defaults
	BossName = FText::FromString(TEXT("Boss"));
	BossTitle = FText::FromString(TEXT(""));
	bShowBossHealthBar = true;
	bAutoStartEncounter = false;

	// Set replication
	bReplicates = true;
	SetReplicateMovement(true);
}

void AHarmoniaBossCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Bind to health component
	if (ULyraHealthComponent* HealthComponent = ULyraHealthComponent::FindHealthComponent(this))
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &AHarmoniaBossCharacter::OnHealthChanged);
	}
}

void AHarmoniaBossCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStartEncounter)
	{
		StartEncounter();
	}
}

void AHarmoniaBossCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABossCharacter, CurrentPhase);
	DOREPLIFETIME(ABossCharacter, bEncounterActive);
}

//~=============================================================================
// Boss Encounter Management
//~=============================================================================

void AHarmoniaBossCharacter::StartEncounter()
{
	if (bEncounterActive)
	{
		return;
	}

	bEncounterActive = true;
	CurrentPhase = 0;

	// Apply phase 0 tags
	OnPhaseEnter(0);

	// Broadcast encounter start
	OnBossEncounterStart.Broadcast(this);
}

void AHarmoniaBossCharacter::EndEncounter(bool bDefeated)
{
	if (!bEncounterActive)
	{
		return;
	}

	bEncounterActive = false;

	// Broadcast encounter end
	OnBossEncounterEnd.Broadcast(this, bDefeated);
}

//~=============================================================================
// Boss Phase System
//~=============================================================================

void AHarmoniaBossCharacter::SetPhase(int32 NewPhase)
{
	if (NewPhase == CurrentPhase || NewPhase < 0 || NewPhase >= GetMaxPhases())
	{
		return;
	}

	if (bIsTransitioningPhase && !bCanInterruptPhaseTransition)
	{
		return;
	}

	const int32 OldPhase = CurrentPhase;
	bIsTransitioningPhase = true;

	// Exit old phase
	OnPhaseExit(OldPhase);

	// Update phase
	CurrentPhase = NewPhase;
	OnRep_CurrentPhase();

	// Enter new phase
	OnPhaseEnter(NewPhase);

	bIsTransitioningPhase = false;

	// Broadcast phase change
	OnBossPhaseChanged.Broadcast(OldPhase, NewPhase);
}

bool AHarmoniaBossCharacter::CanTransitionPhase() const
{
	if (!bEncounterActive)
	{
		return false;
	}

	if (bIsTransitioningPhase && !bCanInterruptPhaseTransition)
	{
		return false;
	}

	return CurrentPhase < GetMaxPhases() - 1;
}

void AHarmoniaBossCharacter::OnPhaseEnter_Implementation(int32 NewPhase)
{
	// Apply gameplay tags for this phase
	if (const FGameplayTagContainer* PhaseTags = PhaseGameplayTags.Find(NewPhase))
	{
		if (ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(GetAbilitySystemComponent()))
		{
			ASC->AddLooseGameplayTags(*PhaseTags);
		}
	}

	// Notify phase component
	if (BossPhaseComponent)
	{
		BossPhaseComponent->OnPhaseTransition(NewPhase);
	}
}

void AHarmoniaBossCharacter::OnPhaseExit_Implementation(int32 OldPhase)
{
	// Remove gameplay tags from old phase
	if (const FGameplayTagContainer* PhaseTags = PhaseGameplayTags.Find(OldPhase))
	{
		if (ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(GetAbilitySystemComponent()))
		{
			ASC->RemoveLooseGameplayTags(*PhaseTags);
		}
	}
}

void AHarmoniaBossCharacter::OnRep_CurrentPhase()
{
	// Handle phase replication on clients
	if (BossPhaseComponent)
	{
		BossPhaseComponent->OnPhaseTransition(CurrentPhase);
	}
}

//~=============================================================================
// Boss Pattern System
//~=============================================================================

void AHarmoniaBossCharacter::ExecutePattern(FName PatternName)
{
	if (!BossPatternComponent)
	{
		return;
	}

	if (bIsExecutingPattern)
	{
		// Don't interrupt current pattern
		return;
	}

	bIsExecutingPattern = true;
	CurrentPatternName = PatternName;

	OnPatternStart(PatternName);

	// Delegate actual pattern execution to pattern component
	BossPatternComponent->ExecutePattern(PatternName);
}

void AHarmoniaBossCharacter::OnPatternStart_Implementation(FName PatternName)
{
	// Blueprint implementable event
}

void AHarmoniaBossCharacter::OnPatternEnd_Implementation(FName PatternName)
{
	bIsExecutingPattern = false;
	CurrentPatternName = NAME_None;
}

//~=============================================================================
// Health Monitoring
//~=============================================================================

void AHarmoniaBossCharacter::OnHealthChanged(ULyraHealthComponent* InHealthComponent, float OldValue, float NewValue, AActor* Instigator)
{
	if (!bEncounterActive)
	{
		return;
	}

	const float MaxHealth = InHealthComponent->GetMaxHealth();
	CheckPhaseTransition(NewValue, MaxHealth);

	// Check if boss is defeated
	if (NewValue <= 0.0f && OldValue > 0.0f)
	{
		EndEncounter(true);
	}
}

void AHarmoniaBossCharacter::CheckPhaseTransition(float CurrentHealth, float MaxHealth)
{
	if (MaxHealth <= 0.0f)
	{
		return;
	}

	const float HealthPercent = CurrentHealth / MaxHealth;

	// Check if we should transition to next phase
	for (int32 PhaseIndex = 0; PhaseIndex < PhaseHealthThresholds.Num(); ++PhaseIndex)
	{
		const float Threshold = PhaseHealthThresholds[PhaseIndex];

		// If health drops below threshold and we're in earlier phase, transition
		if (HealthPercent <= Threshold && CurrentPhase <= PhaseIndex)
		{
			SetPhase(PhaseIndex + 1);
			break;
		}
	}
}
