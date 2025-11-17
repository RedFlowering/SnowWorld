// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaAdvancedAIComponent.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "Monsters/HarmoniaMonsterInterface.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UHarmoniaAdvancedAIComponent::UHarmoniaAdvancedAIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SetIsReplicatedByDefault(true);
}

void UHarmoniaAdvancedAIComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache owner monster
	OwnerMonster = Cast<AHarmoniaMonsterBase>(GetOwner());

	// Initialize emotion state
	EmotionState.CurrentEmotion = EHarmoniaMonsterEmotion::Neutral;
	EmotionState.Intensity = 0.0f;
	EmotionState.Duration = 0.0f;
}

void UHarmoniaAdvancedAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	// Update systems
	if (bEnableEmotions)
	{
		UpdateEmotionSystem(DeltaTime);
	}

	if (bEnableCombos)
	{
		UpdateComboSystem(DeltaTime);
	}

	// Track combat time
	if (OwnerMonster && OwnerMonster->IsInCombat_Implementation())
	{
		CombatTime += DeltaTime;
	}
	else
	{
		CombatTime = 0.0f;
	}
}

void UHarmoniaAdvancedAIComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaAdvancedAIComponent, EmotionState);
	DOREPLIFETIME(UHarmoniaAdvancedAIComponent, ComboState);
	DOREPLIFETIME(UHarmoniaAdvancedAIComponent, TacticalState);
}

// ============================================================================
// Emotion System
// ============================================================================

void UHarmoniaAdvancedAIComponent::TriggerEmotion(EHarmoniaMonsterEmotion Emotion, float Intensity, float Duration)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	EHarmoniaMonsterEmotion OldEmotion = EmotionState.CurrentEmotion;

	EmotionState.CurrentEmotion = Emotion;
	EmotionState.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
	EmotionState.Duration = Duration;

	// Apply emotion modifiers
	ApplyEmotionModifiers(Emotion, EmotionState.Intensity);

	// Broadcast event
	OnEmotionStateChanged.Broadcast(OldEmotion, Emotion);

	UE_LOG(LogTemp, Log, TEXT("%s entered %s state (Intensity: %.2f, Duration: %.1fs)"),
		*GetOwner()->GetName(), *UEnum::GetValueAsString(Emotion), Intensity, Duration);
}

bool UHarmoniaAdvancedAIComponent::IsInEmotion(EHarmoniaMonsterEmotion Emotion) const
{
	return EmotionState.CurrentEmotion == Emotion && EmotionState.IsActive();
}

void UHarmoniaAdvancedAIComponent::ApplyEmotionModifiers(EHarmoniaMonsterEmotion Emotion, float Intensity)
{
	// Reset modifiers
	EmotionState.AttackModifier = 1.0f;
	EmotionState.DefenseModifier = 1.0f;
	EmotionState.SpeedModifier = 1.0f;
	EmotionState.AttackSpeedModifier = 1.0f;

	switch (Emotion)
	{
	case EHarmoniaMonsterEmotion::Enraged:
		// High damage, low defense, increased speed
		EmotionState.AttackModifier = 1.0f + (0.5f * Intensity);
		EmotionState.DefenseModifier = 1.0f - (0.3f * Intensity);
		EmotionState.SpeedModifier = 1.0f + (0.2f * Intensity);
		EmotionState.AttackSpeedModifier = 1.0f + (0.3f * Intensity);
		break;

	case EHarmoniaMonsterEmotion::Fearful:
		// Low damage, high defense, increased speed (fleeing)
		EmotionState.AttackModifier = 1.0f - (0.4f * Intensity);
		EmotionState.DefenseModifier = 1.0f + (0.2f * Intensity);
		EmotionState.SpeedModifier = 1.0f + (0.4f * Intensity);
		EmotionState.AttackSpeedModifier = 1.0f - (0.2f * Intensity);
		break;

	case EHarmoniaMonsterEmotion::Cautious:
		// Balanced, slightly defensive
		EmotionState.AttackModifier = 1.0f - (0.1f * Intensity);
		EmotionState.DefenseModifier = 1.0f + (0.3f * Intensity);
		EmotionState.SpeedModifier = 1.0f - (0.1f * Intensity);
		break;

	case EHarmoniaMonsterEmotion::Confident:
		// High damage, low defense, aggressive
		EmotionState.AttackModifier = 1.0f + (0.3f * Intensity);
		EmotionState.DefenseModifier = 1.0f - (0.2f * Intensity);
		EmotionState.AttackSpeedModifier = 1.0f + (0.2f * Intensity);
		break;

	case EHarmoniaMonsterEmotion::Exhausted:
		// Reduced everything
		EmotionState.AttackModifier = 1.0f - (0.3f * Intensity);
		EmotionState.DefenseModifier = 1.0f - (0.2f * Intensity);
		EmotionState.SpeedModifier = 1.0f - (0.4f * Intensity);
		EmotionState.AttackSpeedModifier = 1.0f - (0.3f * Intensity);
		break;

	default:
		// Neutral - no changes
		break;
	}
}

// ============================================================================
// Combo System
// ============================================================================

void UHarmoniaAdvancedAIComponent::StartCombo(const FHarmoniaMonsterAttackPattern& AttackPattern)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!AttackPattern.bCanStartCombo || AttackPattern.ComboFollowUps.Num() == 0)
	{
		ComboState.Reset();
		return;
	}

	ComboState.CurrentAttackID = AttackPattern.AttackID;
	ComboState.AvailableFollowUps = AttackPattern.ComboFollowUps;
	ComboState.ComboWindowRemaining = AttackPattern.ComboWindow;
	ComboState.ComboCount = 1;

	UE_LOG(LogTemp, Log, TEXT("%s started combo with %s (%d follow-ups available)"),
		*GetOwner()->GetName(), *AttackPattern.AttackID.ToString(), AttackPattern.ComboFollowUps.Num());
}

bool UHarmoniaAdvancedAIComponent::TryContinueCombo(FName& OutNextAttackID)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!ComboState.IsActive())
	{
		return false;
	}

	// Pick a random follow-up
	if (ComboState.AvailableFollowUps.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, ComboState.AvailableFollowUps.Num() - 1);
		OutNextAttackID = ComboState.AvailableFollowUps[RandomIndex];

		ComboState.ComboCount++;
		OnComboExecuted.Broadcast(OutNextAttackID, ComboState.ComboCount);

		UE_LOG(LogTemp, Log, TEXT("%s continuing combo with %s (Count: %d)"),
			*GetOwner()->GetName(), *OutNextAttackID.ToString(), ComboState.ComboCount);

		return true;
	}

	return false;
}

void UHarmoniaAdvancedAIComponent::ResetCombo()
{
	ComboState.Reset();
}

// ============================================================================
// Tactical Positioning
// ============================================================================

bool UHarmoniaAdvancedAIComponent::FindTacticalPosition(AActor* Target, EHarmoniaTacticalPosition PreferredType)
{
	if (!GetOwner() || !Target)
	{
		return false;
	}

	FVector OwnerLocation = GetOwner()->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();

	// Find cover
	if (PreferredType == EHarmoniaTacticalPosition::Cover || PreferredType == EHarmoniaTacticalPosition::None)
	{
		TArray<FVector> CoverPoints = FindCoverPoints(OwnerLocation, Target, TacticalSearchRadius);
		if (CoverPoints.Num() > 0)
		{
			// Pick closest cover
			FVector BestCover = CoverPoints[0];
			float BestDist = FVector::DistSquared(OwnerLocation, BestCover);

			for (const FVector& Cover : CoverPoints)
			{
				float Dist = FVector::DistSquared(OwnerLocation, Cover);
				if (Dist < BestDist)
				{
					BestDist = Dist;
					BestCover = Cover;
				}
			}

			TacticalState.CurrentPosition = EHarmoniaTacticalPosition::Cover;
			TacticalState.TargetLocation = BestCover;
			TacticalState.bInOptimalPosition = false;

			OnTacticalPositionFound.Broadcast(EHarmoniaTacticalPosition::Cover);

			UE_LOG(LogTemp, Log, TEXT("%s found cover position at %s"), *GetOwner()->GetName(), *BestCover.ToString());
			return true;
		}
	}

	// Find high ground
	if (PreferredType == EHarmoniaTacticalPosition::HighGround || PreferredType == EHarmoniaTacticalPosition::None)
	{
		// Search for elevated positions around the target
		for (int32 i = 0; i < 8; ++i)
		{
			float Angle = (2.0f * PI * i) / 8.0f;
			FVector SearchOffset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * 800.0f;
			FVector SearchLocation = TargetLocation + SearchOffset;

			// Trace down to find ground
			FHitResult HitResult;
			FVector TraceStart = SearchLocation + FVector(0, 0, 500);
			FVector TraceEnd = SearchLocation - FVector(0, 0, 500);

			if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
			{
				if (HasHighGroundAdvantage(HitResult.Location, TargetLocation, 100.0f))
				{
					TacticalState.CurrentPosition = EHarmoniaTacticalPosition::HighGround;
					TacticalState.TargetLocation = HitResult.Location;
					TacticalState.bInOptimalPosition = false;
					TacticalState.HeightAdvantage = HitResult.Location.Z - TargetLocation.Z;

					OnTacticalPositionFound.Broadcast(EHarmoniaTacticalPosition::HighGround);

					UE_LOG(LogTemp, Log, TEXT("%s found high ground at %s (Advantage: %.0fcm)"),
						*GetOwner()->GetName(), *HitResult.Location.ToString(), TacticalState.HeightAdvantage);
					return true;
				}
			}
		}
	}

	return false;
}

TArray<FVector> UHarmoniaAdvancedAIComponent::FindCoverPoints(const FVector& FromLocation, AActor* ThreatActor, float SearchRadius)
{
	TArray<FVector> CoverPoints;

	if (!GetWorld() || !ThreatActor)
	{
		return CoverPoints;
	}

	FVector ThreatLocation = ThreatActor->GetActorLocation();

	// Sample points in a circle around the owner
	int32 NumSamples = 16;
	for (int32 i = 0; i < NumSamples; ++i)
	{
		float Angle = (2.0f * PI * i) / NumSamples;
		FVector SampleOffset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * SearchRadius * 0.5f;
		FVector SampleLocation = FromLocation + SampleOffset;

		// Check if this point has line of sight to threat
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwner());
		QueryParams.AddIgnoredActor(ThreatActor);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, SampleLocation, ThreatLocation, ECC_Visibility, QueryParams))
		{
			// Hit something between sample and threat = cover!
			if (HitResult.bBlockingHit)
			{
				CoverPoints.Add(SampleLocation);
			}
		}
	}

	return CoverPoints;
}

bool UHarmoniaAdvancedAIComponent::HasHighGroundAdvantage(const FVector& MyLocation, const FVector& TargetLocation, float MinHeightAdvantage) const
{
	float HeightDiff = MyLocation.Z - TargetLocation.Z;
	return HeightDiff >= MinHeightAdvantage;
}

// ============================================================================
// Enhanced Attack Selection
// ============================================================================

FHarmoniaMonsterAttackPattern UHarmoniaAdvancedAIComponent::SelectContextualAttack(AActor* Target, const TArray<FHarmoniaMonsterAttackPattern>& AvailableAttacks)
{
	if (AvailableAttacks.Num() == 0)
	{
		return FHarmoniaMonsterAttackPattern();
	}

	// Calculate priority for each attack
	TArray<TPair<float, int32>> AttackPriorities;
	for (int32 i = 0; i < AvailableAttacks.Num(); ++i)
	{
		float Priority = CalculateAttackPriority(AvailableAttacks[i], Target);
		AttackPriorities.Add(TPair<float, int32>(Priority, i));
	}

	// Sort by priority
	AttackPriorities.Sort([](const TPair<float, int32>& A, const TPair<float, int32>& B)
	{
		return A.Key > B.Key;
	});

	// Weighted random selection from top 3
	int32 TopCount = FMath::Min(3, AttackPriorities.Num());
	float TotalWeight = 0.0f;
	for (int32 i = 0; i < TopCount; ++i)
	{
		TotalWeight += AttackPriorities[i].Key;
	}

	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float CurrentWeight = 0.0f;

	for (int32 i = 0; i < TopCount; ++i)
	{
		CurrentWeight += AttackPriorities[i].Key;
		if (RandomValue <= CurrentWeight)
		{
			return AvailableAttacks[AttackPriorities[i].Value];
		}
	}

	// Fallback to highest priority
	return AvailableAttacks[AttackPriorities[0].Value];
}

float UHarmoniaAdvancedAIComponent::CalculateAttackPriority(const FHarmoniaMonsterAttackPattern& AttackPattern, AActor* Target) const
{
	float Priority = AttackPattern.SelectionWeight;

	if (!Target)
	{
		return Priority;
	}

	// Health-based priority
	float TargetHealthPercent = GetTargetHealthPercent(Target);
	if (TargetHealthPercent > 0.75f)
	{
		Priority *= AttackPattern.HighHealthPriority;
	}
	else if (TargetHealthPercent > 0.25f)
	{
		Priority *= AttackPattern.MediumHealthPriority;
	}
	else
	{
		Priority *= AttackPattern.LowHealthPriority;
	}

	// Tactical position bonus
	if (TacticalState.bInOptimalPosition)
	{
		Priority *= AttackPattern.TacticalPositionBonus;
	}

	// Emotion modifiers
	if (EmotionState.CurrentEmotion == EHarmoniaMonsterEmotion::Enraged)
	{
		// Prefer high damage attacks when enraged
		Priority *= 1.2f;
	}
	else if (EmotionState.CurrentEmotion == EHarmoniaMonsterEmotion::Fearful)
	{
		// Prefer long-range or defensive attacks when fearful
		if (AttackPattern.MinRange > 300.0f)
		{
			Priority *= 1.5f;
		}
	}

	// Combo priority
	if (bEnableCombos && ComboState.IsActive())
	{
		// Check if this attack is a valid combo follow-up
		if (ComboState.AvailableFollowUps.Contains(AttackPattern.AttackID))
		{
			Priority *= 3.0f; // Heavily favor combo continuations
		}
	}

	return FMath::Max(0.1f, Priority);
}

// ============================================================================
// Protected Functions
// ============================================================================

void UHarmoniaAdvancedAIComponent::UpdateEmotionSystem(float DeltaTime)
{
	// Update emotion duration
	if (EmotionState.IsActive())
	{
		EmotionState.Duration -= DeltaTime;
		if (EmotionState.Duration <= 0.0f)
		{
			// Return to neutral
			TriggerEmotion(EHarmoniaMonsterEmotion::Neutral, 0.0f, 0.0f);
		}
	}

	// Evaluate triggers
	EvaluateEmotionTriggers();
}

void UHarmoniaAdvancedAIComponent::UpdateComboSystem(float DeltaTime)
{
	if (ComboState.IsActive())
	{
		ComboState.ComboWindowRemaining -= DeltaTime;
		if (ComboState.ComboWindowRemaining <= 0.0f)
		{
			UE_LOG(LogTemp, Log, TEXT("%s combo window expired"), *GetOwner()->GetName());
			ComboState.Reset();
		}
	}
}

void UHarmoniaAdvancedAIComponent::EvaluateEmotionTriggers()
{
	if (!OwnerMonster || EmotionState.CurrentEmotion != EHarmoniaMonsterEmotion::Neutral)
	{
		return; // Already in an emotion state
	}

	// Check for rage trigger (low health)
	UAbilitySystemComponent* ASC = OwnerMonster->GetAbilitySystemComponent();
	if (ASC)
	{
		const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
		if (AttributeSet)
		{
			float HealthPercent = AttributeSet->GetHealth() / AttributeSet->GetMaxHealth();
			if (HealthPercent <= RageHealthThreshold)
			{
				TriggerEmotion(EHarmoniaMonsterEmotion::Enraged, 1.0f, 15.0f);
				return;
			}
		}
	}

	// Check for exhaustion trigger (long combat)
	if (CombatTime >= ExhaustionThreshold)
	{
		float Intensity = FMath::Min(1.0f, (CombatTime - ExhaustionThreshold) / 30.0f);
		TriggerEmotion(EHarmoniaMonsterEmotion::Exhausted, Intensity, 20.0f);
		return;
	}

	// Check for fear trigger (powerful enemy)
	AActor* CurrentTarget = OwnerMonster->GetCurrentTarget_Implementation();
	if (CurrentTarget)
	{
		float RelativePower = GetRelativePower(CurrentTarget);
		if (RelativePower >= FearPowerThreshold)
		{
			TriggerEmotion(EHarmoniaMonsterEmotion::Fearful, 0.8f, 10.0f);
			return;
		}
		else if (RelativePower <= 0.7f)
		{
			// Much weaker target, become confident
			TriggerEmotion(EHarmoniaMonsterEmotion::Confident, 0.6f, 15.0f);
			return;
		}
	}
}

float UHarmoniaAdvancedAIComponent::GetTargetHealthPercent(AActor* Target) const
{
	if (!Target)
	{
		return 1.0f;
	}

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target);
	if (!ASI)
	{
		return 1.0f;
	}

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC)
	{
		return 1.0f;
	}

	const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
	if (!AttributeSet)
	{
		return 1.0f;
	}

	float Health = AttributeSet->GetHealth();
	float MaxHealth = AttributeSet->GetMaxHealth();

	return MaxHealth > 0.0f ? (Health / MaxHealth) : 1.0f;
}

float UHarmoniaAdvancedAIComponent::GetRelativePower(AActor* Target) const
{
	if (!OwnerMonster || !Target)
	{
		return 1.0f;
	}

	IAbilitySystemInterface* OwnerASI = Cast<IAbilitySystemInterface>(OwnerMonster);
	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Target);

	if (!OwnerASI || !TargetASI)
	{
		return 1.0f;
	}

	UAbilitySystemComponent* OwnerASC = OwnerASI->GetAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();

	if (!OwnerASC || !TargetASC)
	{
		return 1.0f;
	}

	const UHarmoniaAttributeSet* OwnerAttrs = OwnerASC->GetSet<UHarmoniaAttributeSet>();
	const UHarmoniaAttributeSet* TargetAttrs = TargetASC->GetSet<UHarmoniaAttributeSet>();

	if (!OwnerAttrs || !TargetAttrs)
	{
		return 1.0f;
	}

	// Calculate simple power level (attack + defense + health)
	float OwnerPower = OwnerAttrs->GetAttackPower() + OwnerAttrs->GetDefense() + (OwnerAttrs->GetMaxHealth() * 0.1f);
	float TargetPower = TargetAttrs->GetAttackPower() + TargetAttrs->GetDefense() + (TargetAttrs->GetMaxHealth() * 0.1f);

	return OwnerPower > 0.0f ? (TargetPower / OwnerPower) : 1.0f;
}
