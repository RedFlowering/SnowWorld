// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaLearningAIComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UHarmoniaLearningAIComponent::UHarmoniaLearningAIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;

	bEnableLearning = true;
	MinSamplesForConfidence = 5;
	DetectionRadius = 300.0f;
	AnalysisInterval = 2.0f;

	bEnableAdaptiveDifficulty = true;
	EncountersBeforeAdjustment = 3;
	DifficultyAdjustmentRate = 0.1f;

	CurrentDifficulty = EHarmoniaAdaptiveDifficulty::Normal;
	AnalysisTimer = 0.0f;
	bInCombat = false;

	// Enable replication for multiplayer
	SetIsReplicatedByDefault(true);
}

void UHarmoniaLearningAIComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaLearningAIComponent, LearnedPatterns);
	DOREPLIFETIME(UHarmoniaLearningAIComponent, CurrentDifficulty);
}

void UHarmoniaLearningAIComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerMonster = Cast<AHarmoniaMonsterBase>(GetOwner());
	if (!OwnerMonster)
	{
		UE_LOG(LogTemp, Warning, TEXT("LearningAIComponent: Owner is not a HarmoniaMonsterBase!"));
	}
}

void UHarmoniaLearningAIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	LearnedPatterns.Empty();
	LastPlayerPositions.Empty();
	LastAttackTimes.Empty();
}

void UHarmoniaLearningAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bEnableLearning || !OwnerMonster)
	{
		return;
	}

	AnalysisTimer += DeltaTime;

	if (AnalysisTimer >= AnalysisInterval)
	{
		AnalysisTimer = 0.0f;
		AnalyzePlayerMovement(DeltaTime);
	}

	// Check if should adjust difficulty
	if (bEnableAdaptiveDifficulty)
	{
		// Periodic difficulty check
		AdjustDifficulty();
	}
}

void UHarmoniaLearningAIComponent::AnalyzePlayerMovement(float DeltaTime)
{
	if (!OwnerMonster)
	{
		return;
	}

	// Find nearby players
	TArray<AActor*> FoundPlayers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundPlayers);

	FVector MonsterLocation = OwnerMonster->GetActorLocation();

	for (AActor* PlayerActor : FoundPlayers)
	{
		APawn* PlayerPawn = Cast<APawn>(PlayerActor);
		if (!PlayerPawn || !PlayerPawn->IsPlayerControlled())
		{
			continue;
		}

		float Distance = FVector::Dist(MonsterLocation, PlayerPawn->GetActorLocation());
		if (Distance > DetectionRadius)
		{
			continue;
		}

		// Analyze movement
		if (LastPlayerPositions.Contains(PlayerActor))
		{
			FVector LastPosition = LastPlayerPositions[PlayerActor];
			FVector CurrentPosition = PlayerActor->GetActorLocation();
			FVector MovementDirection = (CurrentPosition - LastPosition).GetSafeNormal();

			// Find or create pattern
			FHarmoniaPlayerPatternEntry* FoundEntry = LearnedPatterns.FindByPredicate([PlayerActor](const FHarmoniaPlayerPatternEntry& Entry)
			{
				return Entry.Player == PlayerActor;
			});

			if (!FoundEntry)
			{
				FHarmoniaPlayerPatternEntry NewEntry;
				NewEntry.Player = PlayerActor;
				NewEntry.Pattern.Player = PlayerActor;
				LearnedPatterns.Add(NewEntry);
				FoundEntry = &LearnedPatterns.Last();
			}

			FHarmoniaPlayerPattern& Pattern = FoundEntry->Pattern;

			// Update preferred dodge direction (weighted average)
			if (!MovementDirection.IsNearlyZero())
			{
				Pattern.PreferredDodgeDirection = (Pattern.PreferredDodgeDirection * 0.8f + MovementDirection * 0.2f).GetSafeNormal();
			}

			UpdatePatternConfidence(PlayerActor);
		}

		LastPlayerPositions.Add(PlayerActor, PlayerActor->GetActorLocation());
	}
}

void UHarmoniaLearningAIComponent::UpdatePatternConfidence(AActor* Player)
{
	FHarmoniaPlayerPatternEntry* FoundEntry = LearnedPatterns.FindByPredicate([Player](const FHarmoniaPlayerPatternEntry& Entry)
	{
		return Entry.Player == Player;
	});

	if (!FoundEntry)
	{
		return;
	}

	FHarmoniaPlayerPattern& Pattern = FoundEntry->Pattern;

	// Calculate confidence based on encounter count
	int32 TotalEncounters = Pattern.EncounterCount;
	Pattern.Confidence = FMath::Clamp(
		static_cast<float>(TotalEncounters) / static_cast<float>(MinSamplesForConfidence),
		0.0f,
		1.0f
	);
}

void UHarmoniaLearningAIComponent::RecordPlayerDodge(AActor* Player, FVector DodgeDirection)
{
	// Only run on server
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!Player)
	{
		return;
	}

	FHarmoniaPlayerPatternEntry* FoundEntry = LearnedPatterns.FindByPredicate([Player](const FHarmoniaPlayerPatternEntry& Entry)
	{
		return Entry.Player == Player;
	});

	if (!FoundEntry)
	{
		FHarmoniaPlayerPatternEntry NewEntry;
		NewEntry.Player = Player;
		NewEntry.Pattern.Player = Player;
		LearnedPatterns.Add(NewEntry);
		FoundEntry = &LearnedPatterns.Last();
	}

	FHarmoniaPlayerPattern& Pattern = FoundEntry->Pattern;

	// Update dodge direction with weighted average
	Pattern.PreferredDodgeDirection = (Pattern.PreferredDodgeDirection * 0.7f + DodgeDirection * 0.3f).GetSafeNormal();

	UpdatePatternConfidence(Player);
}

void UHarmoniaLearningAIComponent::RecordPlayerAttack(AActor* Player, FName AbilityName)
{
	// Only run on server
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!Player)
	{
		return;
	}

	FHarmoniaPlayerPatternEntry* FoundEntry = LearnedPatterns.FindByPredicate([Player](const FHarmoniaPlayerPatternEntry& Entry)
	{
		return Entry.Player == Player;
	});

	if (!FoundEntry)
	{
		FHarmoniaPlayerPatternEntry NewEntry;
		NewEntry.Player = Player;
		NewEntry.Pattern.Player = Player;
		LearnedPatterns.Add(NewEntry);
		FoundEntry = &LearnedPatterns.Last();
	}

	FHarmoniaPlayerPattern& Pattern = FoundEntry->Pattern;

	// Record skill usage
	FHarmoniaSkillUsageEntry* SkillEntry = Pattern.SkillUsageData.FindByPredicate([AbilityName](const FHarmoniaSkillUsageEntry& Entry)
	{
		return Entry.SkillName == AbilityName;
	});

	if (SkillEntry)
	{
		SkillEntry->UsageCount++;
	}
	else
	{
		FHarmoniaSkillUsageEntry NewSkillEntry;
		NewSkillEntry.SkillName = AbilityName;
		NewSkillEntry.UsageCount = 1;
		Pattern.SkillUsageData.Add(NewSkillEntry);
	}

	// Calculate attack interval
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (LastAttackTimes.Contains(Player))
	{
		float TimeSinceLastAttack = CurrentTime - LastAttackTimes[Player];
		Pattern.AverageAttackInterval = (Pattern.AverageAttackInterval * 0.8f + TimeSinceLastAttack * 0.2f);
	}

	LastAttackTimes.Add(Player, CurrentTime);

	UpdatePatternConfidence(Player);
}

void UHarmoniaLearningAIComponent::RecordCombatResult(AActor* Player, bool bPlayerWon, float CombatDuration)
{
	// Only run on server
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!Player)
	{
		return;
	}

	FHarmoniaPlayerPatternEntry* FoundEntry = LearnedPatterns.FindByPredicate([Player](const FHarmoniaPlayerPatternEntry& Entry)
	{
		return Entry.Player == Player;
	});

	if (!FoundEntry)
	{
		FHarmoniaPlayerPatternEntry NewEntry;
		NewEntry.Player = Player;
		NewEntry.Pattern.Player = Player;
		LearnedPatterns.Add(NewEntry);
		FoundEntry = &LearnedPatterns.Last();
	}

	FHarmoniaPlayerPattern& Pattern = FoundEntry->Pattern;

	Pattern.EncounterCount++;

	if (bPlayerWon)
	{
		Pattern.PlayerDeathCount++;
	}
	else
	{
		Pattern.MonsterDeathCount++;
	}

	// Update average combat duration
	Pattern.AverageCombatDuration = (Pattern.AverageCombatDuration * 0.8f + CombatDuration * 0.2f);

	UpdatePatternConfidence(Player);

	UE_LOG(LogTemp, Log, TEXT("Combat result recorded: Player %s, Duration: %.1fs, Encounters: %d"),
		bPlayerWon ? TEXT("Won") : TEXT("Lost"), CombatDuration, Pattern.EncounterCount);
}

FHarmoniaPlayerPattern UHarmoniaLearningAIComponent::GetPlayerPattern(AActor* Player) const
{
	const FHarmoniaPlayerPatternEntry* FoundEntry = LearnedPatterns.FindByPredicate([Player](const FHarmoniaPlayerPatternEntry& Entry)
	{
		return Entry.Player == Player;
	});

	if (FoundEntry)
	{
		return FoundEntry->Pattern;
	}

	return FHarmoniaPlayerPattern();
}

FVector UHarmoniaLearningAIComponent::PredictPlayerDodgeDirection(AActor* Player) const
{
	const FHarmoniaPlayerPatternEntry* FoundEntry = LearnedPatterns.FindByPredicate([Player](const FHarmoniaPlayerPatternEntry& Entry)
	{
		return Entry.Player == Player;
	});

	if (!FoundEntry)
	{
		return FVector::ZeroVector;
	}

	const FHarmoniaPlayerPattern& Pattern = FoundEntry->Pattern;

	// Only predict if confidence is high enough
	if (Pattern.Confidence < 0.5f)
	{
		return FVector::ZeroVector;
	}

	return Pattern.PreferredDodgeDirection;
}

float UHarmoniaLearningAIComponent::GetOptimalAttackTiming(AActor* Player) const
{
	const FHarmoniaPlayerPatternEntry* FoundEntry = LearnedPatterns.FindByPredicate([Player](const FHarmoniaPlayerPatternEntry& Entry)
	{
		return Entry.Player == Player;
	});

	if (!FoundEntry)
	{
		return 2.0f; // Default
	}

	const FHarmoniaPlayerPattern& Pattern = FoundEntry->Pattern;

	// Attack slightly before player's average attack interval
	// This creates pressure and interrupts player rhythm
	return Pattern.AverageAttackInterval * 0.8f;
}

void UHarmoniaLearningAIComponent::AdjustDifficulty()
{
	// Only run on server
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!bEnableAdaptiveDifficulty)
	{
		return;
	}

	// Find most engaged player
	AActor* PrimaryPlayer = nullptr;
	int32 MaxEncounters = 0;

	for (const FHarmoniaPlayerPatternEntry& Entry : LearnedPatterns)
	{
		if (Entry.Pattern.EncounterCount > MaxEncounters)
		{
			MaxEncounters = Entry.Pattern.EncounterCount;
			PrimaryPlayer = Entry.Player;
		}
	}

	if (!PrimaryPlayer || MaxEncounters < EncountersBeforeAdjustment)
	{
		return; // Not enough data yet
	}

	float WinRate = CalculatePlayerWinRate(PrimaryPlayer);

	// Adjust difficulty based on win rate
	// Win rate > 70% = too easy, increase difficulty
	// Win rate < 30% = too hard, decrease difficulty
	if (WinRate > 0.7f)
	{
		// Player winning too much - increase difficulty
		if (CurrentDifficulty != EHarmoniaAdaptiveDifficulty::VeryHard)
		{
			CurrentDifficulty = static_cast<EHarmoniaAdaptiveDifficulty>(static_cast<int32>(CurrentDifficulty) + 1);
			ApplyDifficultyAdjustments();

			UE_LOG(LogTemp, Log, TEXT("Difficulty increased due to high player win rate (%.1f%%)"), WinRate * 100.0f);
		}
	}
	else if (WinRate < 0.3f)
	{
		// Player losing too much - decrease difficulty
		if (CurrentDifficulty != EHarmoniaAdaptiveDifficulty::VeryEasy)
		{
			CurrentDifficulty = static_cast<EHarmoniaAdaptiveDifficulty>(static_cast<int32>(CurrentDifficulty) - 1);
			ApplyDifficultyAdjustments();

			UE_LOG(LogTemp, Log, TEXT("Difficulty decreased due to low player win rate (%.1f%%)"), WinRate * 100.0f);
		}
	}
}

float UHarmoniaLearningAIComponent::CalculatePlayerWinRate(AActor* Player) const
{
	const FHarmoniaPlayerPatternEntry* FoundEntry = LearnedPatterns.FindByPredicate([Player](const FHarmoniaPlayerPatternEntry& Entry)
	{
		return Entry.Player == Player;
	});

	if (!FoundEntry)
	{
		return 0.5f;
	}

	const FHarmoniaPlayerPattern& Pattern = FoundEntry->Pattern;

	int32 TotalBattles = Pattern.PlayerDeathCount + Pattern.MonsterDeathCount;
	if (TotalBattles == 0)
	{
		return 0.5f;
	}

	return static_cast<float>(Pattern.PlayerDeathCount) / static_cast<float>(TotalBattles);
}

void UHarmoniaLearningAIComponent::ApplyDifficultyAdjustments()
{
	// Only run on server
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!OwnerMonster)
	{
		return;
	}

	float Multiplier = GetDifficultyMultiplier();

	UAbilitySystemComponent* ASC = OwnerMonster->GetAbilitySystemComponent();
	if (ASC)
	{
		const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
		if (AttributeSet)
		{
			// Adjust stats based on difficulty
			float CurrentAttackPower = AttributeSet->GetAttackPower();
			float CurrentDefense = AttributeSet->GetDefense();

			// Apply multiplier to base stats
			ASC->SetNumericAttributeBase(AttributeSet->GetAttackPowerAttribute(), CurrentAttackPower * Multiplier);
			ASC->SetNumericAttributeBase(AttributeSet->GetDefenseAttribute(), CurrentDefense * Multiplier);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Difficulty adjustments applied: Multiplier = %.2f"), Multiplier);
}

float UHarmoniaLearningAIComponent::GetDifficultyMultiplier() const
{
	switch (CurrentDifficulty)
	{
	case EHarmoniaAdaptiveDifficulty::VeryEasy:
		return 0.7f;
	case EHarmoniaAdaptiveDifficulty::Easy:
		return 0.85f;
	case EHarmoniaAdaptiveDifficulty::Normal:
		return 1.0f;
	case EHarmoniaAdaptiveDifficulty::Hard:
		return 1.2f;
	case EHarmoniaAdaptiveDifficulty::VeryHard:
		return 1.5f;
	default:
		return 1.0f;
	}
}
