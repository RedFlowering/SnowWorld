// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaDynamicDifficultySubsystem.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"
#include "HarmoniaLogCategories.h"

// ============================================================================
// Subsystem Lifecycle
// ============================================================================

void UHarmoniaDynamicDifficultySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SessionStartTime = FPlatformTime::Seconds();
	LastRecalculationTime = SessionStartTime;

	// Initialize default profiles
	{
		FHarmoniaDDAProfile BeginnerProfile;
		BeginnerProfile.ProfileName = TEXT("Beginner");
		BeginnerProfile.DisplayName = FText::FromString(TEXT("Beginner"));
		BeginnerProfile.Description = FText::FromString(TEXT("Maximum assistance for new players"));
		BeginnerProfile.MinSkillRating = 0.0f;
		BeginnerProfile.MaxSkillRating = 25.0f;
		BeginnerProfile.BaseParameters.EnemyHealthMultiplier = 0.8f;
		BeginnerProfile.BaseParameters.EnemyDamageMultiplier = 0.7f;
		BeginnerProfile.BaseParameters.EnemyAggressionMultiplier = 0.7f;
		BeginnerProfile.BaseParameters.EnemyReactionTimeMultiplier = 1.3f;
		BeginnerProfile.BaseParameters.ParryWindowMultiplier = 1.4f;
		BeginnerProfile.BaseParameters.DodgeIFrameMultiplier = 1.3f;
		BeginnerProfile.BaseParameters.HealingMultiplier = 1.3f;
		BeginnerProfile.BaseParameters.AIDelayBetweenAttacks = 0.5f;
		BeginnerProfile.BaseParameters.bReduceGroupAggression = true;
		Profiles.Add(BeginnerProfile);

		FHarmoniaDDAProfile LearningProfile;
		LearningProfile.ProfileName = TEXT("Learning");
		LearningProfile.DisplayName = FText::FromString(TEXT("Learning"));
		LearningProfile.Description = FText::FromString(TEXT("Moderate assistance while learning"));
		LearningProfile.MinSkillRating = 25.0f;
		LearningProfile.MaxSkillRating = 45.0f;
		LearningProfile.BaseParameters.EnemyHealthMultiplier = 0.9f;
		LearningProfile.BaseParameters.EnemyDamageMultiplier = 0.85f;
		LearningProfile.BaseParameters.EnemyAggressionMultiplier = 0.85f;
		LearningProfile.BaseParameters.EnemyReactionTimeMultiplier = 1.15f;
		LearningProfile.BaseParameters.ParryWindowMultiplier = 1.2f;
		LearningProfile.BaseParameters.DodgeIFrameMultiplier = 1.15f;
		LearningProfile.BaseParameters.HealingMultiplier = 1.15f;
		LearningProfile.BaseParameters.AIDelayBetweenAttacks = 0.25f;
		LearningProfile.BaseParameters.bReduceGroupAggression = true;
		Profiles.Add(LearningProfile);

		FHarmoniaDDAProfile StandardProfile;
		StandardProfile.ProfileName = TEXT("Standard");
		StandardProfile.DisplayName = FText::FromString(TEXT("Standard"));
		StandardProfile.Description = FText::FromString(TEXT("Intended difficulty experience"));
		StandardProfile.MinSkillRating = 45.0f;
		StandardProfile.MaxSkillRating = 65.0f;
		StandardProfile.BaseParameters.EnemyHealthMultiplier = 1.0f;
		StandardProfile.BaseParameters.EnemyDamageMultiplier = 1.0f;
		StandardProfile.BaseParameters.EnemyAggressionMultiplier = 1.0f;
		StandardProfile.BaseParameters.EnemyReactionTimeMultiplier = 1.0f;
		StandardProfile.BaseParameters.ParryWindowMultiplier = 1.0f;
		StandardProfile.BaseParameters.DodgeIFrameMultiplier = 1.0f;
		StandardProfile.BaseParameters.HealingMultiplier = 1.0f;
		Profiles.Add(StandardProfile);

		FHarmoniaDDAProfile SkilledProfile;
		SkilledProfile.ProfileName = TEXT("Skilled");
		SkilledProfile.DisplayName = FText::FromString(TEXT("Skilled"));
		SkilledProfile.Description = FText::FromString(TEXT("Increased challenge for experienced players"));
		SkilledProfile.MinSkillRating = 65.0f;
		SkilledProfile.MaxSkillRating = 85.0f;
		SkilledProfile.BaseParameters.EnemyHealthMultiplier = 1.1f;
		SkilledProfile.BaseParameters.EnemyDamageMultiplier = 1.1f;
		SkilledProfile.BaseParameters.EnemyAggressionMultiplier = 1.15f;
		SkilledProfile.BaseParameters.EnemyReactionTimeMultiplier = 0.9f;
		SkilledProfile.BaseParameters.EnemyAttackFrequencyMultiplier = 1.1f;
		SkilledProfile.BaseParameters.ParryWindowMultiplier = 0.95f;
		SkilledProfile.BaseParameters.DodgeIFrameMultiplier = 0.95f;
		Profiles.Add(SkilledProfile);

		FHarmoniaDDAProfile MasterProfile;
		MasterProfile.ProfileName = TEXT("Master");
		MasterProfile.DisplayName = FText::FromString(TEXT("Master"));
		MasterProfile.Description = FText::FromString(TEXT("Maximum challenge for expert players"));
		MasterProfile.MinSkillRating = 85.0f;
		MasterProfile.MaxSkillRating = 100.0f;
		MasterProfile.BaseParameters.EnemyHealthMultiplier = 1.2f;
		MasterProfile.BaseParameters.EnemyDamageMultiplier = 1.2f;
		MasterProfile.BaseParameters.EnemyAggressionMultiplier = 1.3f;
		MasterProfile.BaseParameters.EnemyReactionTimeMultiplier = 0.8f;
		MasterProfile.BaseParameters.EnemyAttackFrequencyMultiplier = 1.2f;
		MasterProfile.BaseParameters.ParryWindowMultiplier = 0.9f;
		MasterProfile.BaseParameters.DodgeIFrameMultiplier = 0.9f;
		MasterProfile.BaseParameters.HealingMultiplier = 0.9f;
		Profiles.Add(MasterProfile);
	}

	// Start tick timer
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UWorld* World = GameInstance->GetWorld())
		{
			World->GetTimerManager().SetTimer(
				TickTimerHandle,
				FTimerDelegate::CreateUObject(this, &UHarmoniaDynamicDifficultySubsystem::TickDDA),
				1.0f,
				true
			);
		}
	}

	RecalculateParameters();

	UE_LOG(LogHarmoniaKit, Log, TEXT("HarmoniaDynamicDifficultySubsystem: Initialized with %d profiles"), Profiles.Num());
}

void UHarmoniaDynamicDifficultySubsystem::Deinitialize()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UWorld* World = GameInstance->GetWorld())
		{
			World->GetTimerManager().ClearTimer(TickTimerHandle);
		}
	}

	Super::Deinitialize();
}

// ============================================================================
// Performance Tracking
// ============================================================================

void UHarmoniaDynamicDifficultySubsystem::ReportDeath(FGameplayTag DeathCause, const FString& KillerInfo)
{
	float CurrentTime = FPlatformTime::Seconds();

	Metrics.TotalDeaths++;
	Metrics.ConsecutiveDeaths++;
	Metrics.KillsWithoutDeath = 0;

	// Track death timestamp
	Metrics.RecentDeathTimestamps.Add(CurrentTime);

	// Update deaths in last hour
	Metrics.DeathsInLastHour = 0;
	float OneHourAgo = CurrentTime - 3600.0f;
	for (int32 i = Metrics.RecentDeathTimestamps.Num() - 1; i >= 0; --i)
	{
		if (Metrics.RecentDeathTimestamps[i] > OneHourAgo)
		{
			Metrics.DeathsInLastHour++;
		}
		else
		{
			// Remove old timestamps
			Metrics.RecentDeathTimestamps.RemoveAt(i);
		}
	}

	// Boss tracking
	if (!CurrentBossEncounter.IsNone())
	{
		BossEncounterDeaths++;
	}

	UE_LOG(LogHarmoniaKit, Log, TEXT("DDA: Player death reported (Total: %d, Consecutive: %d, LastHour: %d)"),
		Metrics.TotalDeaths, Metrics.ConsecutiveDeaths, Metrics.DeathsInLastHour);

	// Check for frustration
	if (IsPlayerFrustrated())
	{
		UE_LOG(LogHarmoniaKit, Log, TEXT("DDA: Frustration detected, adjusting parameters"));
		ForceRecalculate();
	}
}

void UHarmoniaDynamicDifficultySubsystem::ReportVictory(float HealthRemaining, float CombatDuration, FGameplayTag EnemyType)
{
	Metrics.ConsecutiveDeaths = 0;
	Metrics.KillsWithoutDeath++;

	// Track health remaining
	Metrics.RecentVictoryHealthPercents.Add(HealthRemaining);
	if (Metrics.RecentVictoryHealthPercents.Num() > 20)
	{
		Metrics.RecentVictoryHealthPercents.RemoveAt(0);
	}

	// Calculate average health on victory
	float TotalHealth = 0.0f;
	for (float Health : Metrics.RecentVictoryHealthPercents)
	{
		TotalHealth += Health;
	}
	Metrics.AverageHealthOnVictory = TotalHealth / Metrics.RecentVictoryHealthPercents.Num();

	// Track combat duration
	Metrics.RecentCombatDurations.Add(CombatDuration);
	if (Metrics.RecentCombatDurations.Num() > 20)
	{
		Metrics.RecentCombatDurations.RemoveAt(0);
	}

	// Calculate average combat duration
	float TotalDuration = 0.0f;
	for (float Duration : Metrics.RecentCombatDurations)
	{
		TotalDuration += Duration;
	}
	Metrics.AverageCombatDuration = TotalDuration / Metrics.RecentCombatDurations.Num();

	// Track elite/boss kills
	if (EnemyType.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Character.Type.Elite"))))
	{
		Metrics.ElitesDefeated++;
	}
}

void UHarmoniaDynamicDifficultySubsystem::ReportParry(bool bPerfect)
{
	ParryAttempts++;
	ParrySuccesses++;

	if (bPerfect)
	{
		Metrics.PerfectParries++;
	}

	Metrics.ParrySuccessRate = ParryAttempts > 0 ? (float)ParrySuccesses / (float)ParryAttempts : 0.0f;
}

void UHarmoniaDynamicDifficultySubsystem::ReportParryFailure()
{
	ParryAttempts++;
	Metrics.ParrySuccessRate = ParryAttempts > 0 ? (float)ParrySuccesses / (float)ParryAttempts : 0.0f;
}

void UHarmoniaDynamicDifficultySubsystem::ReportDodge(bool bPerfect)
{
	DodgeAttempts++;
	DodgeSuccesses++;

	if (bPerfect)
	{
		Metrics.PerfectDodges++;
	}

	Metrics.DodgeSuccessRate = DodgeAttempts > 0 ? (float)DodgeSuccesses / (float)DodgeAttempts : 0.0f;
}

void UHarmoniaDynamicDifficultySubsystem::ReportDodgeFailure()
{
	DodgeAttempts++;
	Metrics.DodgeSuccessRate = DodgeAttempts > 0 ? (float)DodgeSuccesses / (float)DodgeAttempts : 0.0f;
}

void UHarmoniaDynamicDifficultySubsystem::ReportDamageDealt(float Damage)
{
	Metrics.TotalDamageDealt += Damage;
	Metrics.DamageRatio = Metrics.TotalDamageTaken > 0 ? Metrics.TotalDamageDealt / Metrics.TotalDamageTaken : 1.0f;
}

void UHarmoniaDynamicDifficultySubsystem::ReportDamageTaken(float Damage)
{
	Metrics.TotalDamageTaken += Damage;
	Metrics.DamageRatio = Metrics.TotalDamageTaken > 0 ? Metrics.TotalDamageDealt / Metrics.TotalDamageTaken : 1.0f;
}

void UHarmoniaDynamicDifficultySubsystem::ReportAttackAttempt(bool bHit)
{
	AttackAttempts++;
	if (bHit)
	{
		AttackHits++;
	}
	Metrics.HitAccuracy = AttackAttempts > 0 ? (float)AttackHits / (float)AttackAttempts : 0.0f;
}

void UHarmoniaDynamicDifficultySubsystem::ReportBossEncounterStarted(FName BossID)
{
	CurrentBossEncounter = BossID;
	BossEncounterDeaths = 0;
	UE_LOG(LogHarmoniaKit, Log, TEXT("DDA: Boss encounter started: %s"), *BossID.ToString());
}

void UHarmoniaDynamicDifficultySubsystem::ReportBossDefeated(FName BossID, int32 Attempts)
{
	Metrics.BossesDefeated++;
	CurrentBossEncounter = NAME_None;

	UE_LOG(LogHarmoniaKit, Log, TEXT("DDA: Boss defeated: %s after %d attempts"), *BossID.ToString(), Attempts);

	// Recalculate after boss victory
	ForceRecalculate();
}

// ============================================================================
// DDA Parameters
// ============================================================================

FHarmoniaDDAParameters UHarmoniaDynamicDifficultySubsystem::GetCurrentParameters() const
{
	return CurrentParameters;
}

float UHarmoniaDynamicDifficultySubsystem::GetCombinedEnemyHealthMultiplier() const
{
	// Could combine with manual difficulty settings here
	return CurrentParameters.EnemyHealthMultiplier;
}

float UHarmoniaDynamicDifficultySubsystem::GetCombinedEnemyDamageMultiplier() const
{
	return CurrentParameters.EnemyDamageMultiplier;
}

// ============================================================================
// Player Skill Rating
// ============================================================================

float UHarmoniaDynamicDifficultySubsystem::GetPlayerSkillRating() const
{
	return Metrics.OverallSkillRating;
}

FText UHarmoniaDynamicDifficultySubsystem::GetPlayerSkillTier() const
{
	float Rating = Metrics.OverallSkillRating;

	if (Rating < 25.0f) return FText::FromString(TEXT("Novice"));
	if (Rating < 45.0f) return FText::FromString(TEXT("Apprentice"));
	if (Rating < 65.0f) return FText::FromString(TEXT("Journeyman"));
	if (Rating < 85.0f) return FText::FromString(TEXT("Expert"));
	return FText::FromString(TEXT("Master"));
}

FHarmoniaPerformanceMetrics UHarmoniaDynamicDifficultySubsystem::GetPerformanceMetrics() const
{
	return Metrics;
}

// ============================================================================
// System Control
// ============================================================================

void UHarmoniaDynamicDifficultySubsystem::SetDDAEnabled(bool bEnabled)
{
	bDDAEnabled = bEnabled;

	if (!bEnabled)
	{
		// Reset to standard parameters
		CurrentParameters = FHarmoniaDDAParameters();
		TargetParameters = FHarmoniaDDAParameters();
	}
	else
	{
		ForceRecalculate();
	}

	UE_LOG(LogHarmoniaKit, Log, TEXT("DDA: System %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UHarmoniaDynamicDifficultySubsystem::ForceRecalculate()
{
	if (!bDDAEnabled) return;

	LastRecalculationTime = FPlatformTime::Seconds();
	RecalculateParameters();
}

void UHarmoniaDynamicDifficultySubsystem::ResetMetrics()
{
	Metrics = FHarmoniaPerformanceMetrics();
	ParryAttempts = 0;
	ParrySuccesses = 0;
	DodgeAttempts = 0;
	DodgeSuccesses = 0;
	AttackAttempts = 0;
	AttackHits = 0;
	CurrentBossEncounter = NAME_None;
	BossEncounterDeaths = 0;

	ForceRecalculate();

	UE_LOG(LogHarmoniaKit, Log, TEXT("DDA: Metrics reset"));
}

void UHarmoniaDynamicDifficultySubsystem::LoadProfilesFromDataTable(UDataTable* DataTable)
{
	if (!DataTable) return;

	Profiles.Empty();

	TArray<FHarmoniaDDAProfile*> Rows;
	DataTable->GetAllRows<FHarmoniaDDAProfile>(TEXT("LoadDDAProfiles"), Rows);

	for (FHarmoniaDDAProfile* Row : Rows)
	{
		if (Row)
		{
			Profiles.Add(*Row);
		}
	}

	// Sort by skill rating
	Profiles.Sort([](const FHarmoniaDDAProfile& A, const FHarmoniaDDAProfile& B) {
		return A.MinSkillRating < B.MinSkillRating;
	});

	UE_LOG(LogHarmoniaKit, Log, TEXT("DDA: Loaded %d profiles from DataTable"), Profiles.Num());
}

void UHarmoniaDynamicDifficultySubsystem::SetMetricsFromSave(const FHarmoniaPerformanceMetrics& SavedMetrics)
{
	Metrics = SavedMetrics;
	ForceRecalculate();
}

// ============================================================================
// Internal Functions
// ============================================================================

void UHarmoniaDynamicDifficultySubsystem::RecalculateParameters()
{
	float OldRating = Metrics.OverallSkillRating;

	// Calculate skill rating
	Metrics.OverallSkillRating = CalculateSkillRating();

	// Get interpolated parameters
	TargetParameters = InterpolateParameters(Metrics.OverallSkillRating);

	// Apply frustration mitigation if needed
	if (IsPlayerFrustrated())
	{
		TargetParameters.EnemyHealthMultiplier *= 0.9f;
		TargetParameters.EnemyDamageMultiplier *= 0.85f;
		TargetParameters.ParryWindowMultiplier *= 1.2f;
		TargetParameters.DodgeIFrameMultiplier *= 1.15f;
		TargetParameters.AIDelayBetweenAttacks += 0.3f;
	}

	// Boss encounter adjustments
	if (!CurrentBossEncounter.IsNone() && BossEncounterDeaths > 3)
	{
		float BossAssistance = FMath::Min(0.15f, (BossEncounterDeaths - 3) * 0.03f);
		TargetParameters.EnemyHealthMultiplier *= (1.0f - BossAssistance);
		TargetParameters.EnemyDamageMultiplier *= (1.0f - BossAssistance * 0.5f);
	}

	// Fire events if rating changed significantly
	if (FMath::Abs(OldRating - Metrics.OverallSkillRating) > 5.0f)
	{
		OnPlayerSkillRatingChanged.Broadcast(OldRating, Metrics.OverallSkillRating);
	}

	UE_LOG(LogHarmoniaKit, Verbose, TEXT("DDA: Recalculated - Skill: %.1f, EnemyHP: %.2f, EnemyDmg: %.2f"),
		Metrics.OverallSkillRating, TargetParameters.EnemyHealthMultiplier, TargetParameters.EnemyDamageMultiplier);
}

float UHarmoniaDynamicDifficultySubsystem::CalculateSkillRating() const
{
	float Rating = 50.0f; // Start at middle

	// Adjust based on death rate
	float DeathsPerHour = Metrics.DeathsInLastHour;
	if (DeathsPerHour > 10) Rating -= 20.0f;
	else if (DeathsPerHour > 5) Rating -= 10.0f;
	else if (DeathsPerHour < 2) Rating += 10.0f;

	// Adjust based on consecutive deaths
	Rating -= Metrics.ConsecutiveDeaths * 5.0f;

	// Adjust based on kill streak
	if (Metrics.KillsWithoutDeath > 20) Rating += 15.0f;
	else if (Metrics.KillsWithoutDeath > 10) Rating += 10.0f;
	else if (Metrics.KillsWithoutDeath > 5) Rating += 5.0f;

	// Adjust based on defensive skills
	if (Metrics.ParrySuccessRate > 0.7f) Rating += 10.0f;
	else if (Metrics.ParrySuccessRate > 0.5f) Rating += 5.0f;
	else if (Metrics.ParrySuccessRate < 0.2f) Rating -= 5.0f;

	if (Metrics.DodgeSuccessRate > 0.8f) Rating += 10.0f;
	else if (Metrics.DodgeSuccessRate > 0.6f) Rating += 5.0f;

	// Adjust based on damage ratio
	if (Metrics.DamageRatio > 3.0f) Rating += 15.0f;
	else if (Metrics.DamageRatio > 2.0f) Rating += 10.0f;
	else if (Metrics.DamageRatio > 1.5f) Rating += 5.0f;
	else if (Metrics.DamageRatio < 0.5f) Rating -= 10.0f;

	// Adjust based on health on victory
	if (Metrics.AverageHealthOnVictory > 0.8f) Rating += 10.0f;
	else if (Metrics.AverageHealthOnVictory > 0.6f) Rating += 5.0f;
	else if (Metrics.AverageHealthOnVictory < 0.2f) Rating -= 5.0f;

	// Perfect actions bonus
	Rating += FMath::Min(10.0f, (Metrics.PerfectParries + Metrics.PerfectDodges) * 0.5f);

	// Boss/Elite kills bonus
	Rating += FMath::Min(10.0f, Metrics.BossesDefeated * 2.0f);
	Rating += FMath::Min(5.0f, Metrics.ElitesDefeated * 0.2f);

	// Clamp to valid range
	return FMath::Clamp(Rating, 0.0f, 100.0f);
}

FHarmoniaDDAParameters UHarmoniaDynamicDifficultySubsystem::InterpolateParameters(float SkillRating) const
{
	if (Profiles.Num() == 0)
	{
		return FHarmoniaDDAParameters();
	}

	// Find the two profiles to interpolate between
	const FHarmoniaDDAProfile* LowerProfile = nullptr;
	const FHarmoniaDDAProfile* UpperProfile = nullptr;

	for (int32 i = 0; i < Profiles.Num(); ++i)
	{
		if (SkillRating >= Profiles[i].MinSkillRating && SkillRating <= Profiles[i].MaxSkillRating)
		{
			// Within a profile's range
			return Profiles[i].BaseParameters;
		}

		if (SkillRating < Profiles[i].MinSkillRating)
		{
			UpperProfile = &Profiles[i];
			if (i > 0) LowerProfile = &Profiles[i - 1];
			break;
		}

		LowerProfile = &Profiles[i];
	}

	// If only one profile available
	if (!LowerProfile && UpperProfile) return UpperProfile->BaseParameters;
	if (LowerProfile && !UpperProfile) return LowerProfile->BaseParameters;
	if (!LowerProfile && !UpperProfile) return FHarmoniaDDAParameters();

	// Interpolate between profiles
	float Alpha = (SkillRating - LowerProfile->MaxSkillRating) / (UpperProfile->MinSkillRating - LowerProfile->MaxSkillRating);
	Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

	FHarmoniaDDAParameters Result;
	const FHarmoniaDDAParameters& A = LowerProfile->BaseParameters;
	const FHarmoniaDDAParameters& B = UpperProfile->BaseParameters;

	Result.EnemyHealthMultiplier = FMath::Lerp(A.EnemyHealthMultiplier, B.EnemyHealthMultiplier, Alpha);
	Result.EnemyDamageMultiplier = FMath::Lerp(A.EnemyDamageMultiplier, B.EnemyDamageMultiplier, Alpha);
	Result.EnemyAggressionMultiplier = FMath::Lerp(A.EnemyAggressionMultiplier, B.EnemyAggressionMultiplier, Alpha);
	Result.EnemyReactionTimeMultiplier = FMath::Lerp(A.EnemyReactionTimeMultiplier, B.EnemyReactionTimeMultiplier, Alpha);
	Result.EnemyAttackFrequencyMultiplier = FMath::Lerp(A.EnemyAttackFrequencyMultiplier, B.EnemyAttackFrequencyMultiplier, Alpha);
	Result.PlayerDamageMultiplier = FMath::Lerp(A.PlayerDamageMultiplier, B.PlayerDamageMultiplier, Alpha);
	Result.PlayerDefenseMultiplier = FMath::Lerp(A.PlayerDefenseMultiplier, B.PlayerDefenseMultiplier, Alpha);
	Result.ParryWindowMultiplier = FMath::Lerp(A.ParryWindowMultiplier, B.ParryWindowMultiplier, Alpha);
	Result.DodgeIFrameMultiplier = FMath::Lerp(A.DodgeIFrameMultiplier, B.DodgeIFrameMultiplier, Alpha);
	Result.HealingMultiplier = FMath::Lerp(A.HealingMultiplier, B.HealingMultiplier, Alpha);
	Result.ItemDropRateMultiplier = FMath::Lerp(A.ItemDropRateMultiplier, B.ItemDropRateMultiplier, Alpha);
	Result.ResourceGainMultiplier = FMath::Lerp(A.ResourceGainMultiplier, B.ResourceGainMultiplier, Alpha);
	Result.AIDelayBetweenAttacks = FMath::Lerp(A.AIDelayBetweenAttacks, B.AIDelayBetweenAttacks, Alpha);
	Result.AIAccuracyPenalty = FMath::Lerp(A.AIAccuracyPenalty, B.AIAccuracyPenalty, Alpha);
	Result.bReduceGroupAggression = Alpha < 0.5f ? A.bReduceGroupAggression : B.bReduceGroupAggression;

	return Result;
}

bool UHarmoniaDynamicDifficultySubsystem::IsPlayerFrustrated() const
{
	// Multiple deaths in short time
	if (Metrics.ConsecutiveDeaths >= 3) return true;

	// Many deaths in last hour
	if (Metrics.DeathsInLastHour >= 8) return true;

	// Boss encounter with many deaths
	if (!CurrentBossEncounter.IsNone() && BossEncounterDeaths >= 5) return true;

	return false;
}

bool UHarmoniaDynamicDifficultySubsystem::IsPlayerSkilled() const
{
	// High kill streak
	if (Metrics.KillsWithoutDeath >= 15) return true;

	// High defensive success
	if (Metrics.ParrySuccessRate > 0.7f && ParryAttempts > 10) return true;
	if (Metrics.DodgeSuccessRate > 0.8f && DodgeAttempts > 10) return true;

	// Consistently wins with high health
	if (Metrics.AverageHealthOnVictory > 0.7f && Metrics.RecentVictoryHealthPercents.Num() >= 10) return true;

	return false;
}

void UHarmoniaDynamicDifficultySubsystem::UpdateTimeBasedMetrics()
{
	float CurrentTime = FPlatformTime::Seconds();
	Metrics.PlayTimeHours = (CurrentTime - SessionStartTime) / 3600.0f;
}

void UHarmoniaDynamicDifficultySubsystem::TickDDA()
{
	if (!bDDAEnabled) return;

	UpdateTimeBasedMetrics();

	// Smooth transition to target parameters
	float DeltaTime = 1.0f; // 1 second tick

	CurrentParameters.EnemyHealthMultiplier = FMath::FInterpTo(CurrentParameters.EnemyHealthMultiplier, TargetParameters.EnemyHealthMultiplier, DeltaTime, ParameterTransitionSpeed);
	CurrentParameters.EnemyDamageMultiplier = FMath::FInterpTo(CurrentParameters.EnemyDamageMultiplier, TargetParameters.EnemyDamageMultiplier, DeltaTime, ParameterTransitionSpeed);
	CurrentParameters.EnemyAggressionMultiplier = FMath::FInterpTo(CurrentParameters.EnemyAggressionMultiplier, TargetParameters.EnemyAggressionMultiplier, DeltaTime, ParameterTransitionSpeed);
	CurrentParameters.EnemyReactionTimeMultiplier = FMath::FInterpTo(CurrentParameters.EnemyReactionTimeMultiplier, TargetParameters.EnemyReactionTimeMultiplier, DeltaTime, ParameterTransitionSpeed);
	CurrentParameters.ParryWindowMultiplier = FMath::FInterpTo(CurrentParameters.ParryWindowMultiplier, TargetParameters.ParryWindowMultiplier, DeltaTime, ParameterTransitionSpeed);
	CurrentParameters.DodgeIFrameMultiplier = FMath::FInterpTo(CurrentParameters.DodgeIFrameMultiplier, TargetParameters.DodgeIFrameMultiplier, DeltaTime, ParameterTransitionSpeed);
	CurrentParameters.HealingMultiplier = FMath::FInterpTo(CurrentParameters.HealingMultiplier, TargetParameters.HealingMultiplier, DeltaTime, ParameterTransitionSpeed);
	CurrentParameters.AIDelayBetweenAttacks = FMath::FInterpTo(CurrentParameters.AIDelayBetweenAttacks, TargetParameters.AIDelayBetweenAttacks, DeltaTime, ParameterTransitionSpeed);

	// Periodic recalculation
	float CurrentTime = FPlatformTime::Seconds();
	if (CurrentTime - LastRecalculationTime > RecalculationCooldown)
	{
		RecalculateParameters();
	}
}
