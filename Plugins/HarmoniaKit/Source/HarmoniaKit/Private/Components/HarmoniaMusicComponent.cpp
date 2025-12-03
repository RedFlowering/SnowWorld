// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/HarmoniaMusicComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

UHarmoniaMusicComponent::UHarmoniaMusicComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UHarmoniaMusicComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHarmoniaMusicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsPerforming)
	{
		float ElapsedTime = GetWorld()->GetTimeSeconds() - PerformanceStartTime;
		if (ElapsedTime >= RequiredPerformanceTime)
		{
			CompletePerformance();
		}
	}
}

bool UHarmoniaMusicComponent::StartPerformance(FName MusicID)
{
	if (bIsPerforming || !MusicSheetDatabase.Contains(MusicID))
	{
		return false;
	}

	const FMusicSheetData& MusicSheet = MusicSheetDatabase[MusicID];

	// Check level
	if (PerformanceLevel < MusicSheet.MinPerformanceLevel)
	{
		return false;
	}

	// Check if holding sheet music
	if (MusicSheet.bHidden && !IsMusicSheetKnown(MusicID))
	{
		return false;
	}

	// Check instrument
	if (MusicSheet.RequiredInstruments.Num() > 0)
	{
		if (!HasInstrumentEquipped())
		{
			return false;
		}

		bool bHasRequiredInstrument = false;
		for (EInstrumentType RequiredType : MusicSheet.RequiredInstruments)
		{
			if (EquippedInstrument.InstrumentType == RequiredType)
			{
				bHasRequiredInstrument = true;
				break;
			}
		}

		if (!bHasRequiredInstrument)
		{
			return false;
		}
	}

	CurrentMusicID = MusicID;
	bIsPerforming = true;
	PerformanceStartTime = GetWorld()->GetTimeSeconds();
	RequiredPerformanceTime = MusicSheet.PerformanceDuration;

	// 미니게임 초기??
	if (bUseMinigame)
	{
		int32 NoteCount = FMath::Max(5, MusicSheet.Difficulty * 3);
		CurrentRhythmNotes = GenerateRhythmNotes(MusicID, NoteCount);
		TotalNoteCount = CurrentRhythmNotes.Num();
		PerfectNoteCount = 0;
		MinigameScore = 0.0f;
	}

	SetComponentTickEnabled(true);

	OnPerformanceStarted.Broadcast(MusicID, RequiredPerformanceTime, MusicSheet.Genre);

	return true;
}

void UHarmoniaMusicComponent::CancelPerformance()
{
	if (!bIsPerforming)
	{
		return;
	}

	bIsPerforming = false;
	CurrentMusicID = NAME_None;
	CurrentRhythmNotes.Empty();
	SetComponentTickEnabled(false);

	OnPerformanceCancelled.Broadcast();
}

float UHarmoniaMusicComponent::GetPerformanceProgress() const
{
	if (!bIsPerforming || RequiredPerformanceTime <= 0.0f)
	{
		return 0.0f;
	}

	float ElapsedTime = GetWorld()->GetTimeSeconds() - PerformanceStartTime;
	return FMath::Clamp(ElapsedTime / RequiredPerformanceTime, 0.0f, 1.0f);
}

bool UHarmoniaMusicComponent::CanPerformMusic(FName MusicID) const
{
	if (!MusicSheetDatabase.Contains(MusicID))
	{
		return false;
	}

	const FMusicSheetData& MusicSheet = MusicSheetDatabase[MusicID];

	if (PerformanceLevel < MusicSheet.MinPerformanceLevel)
	{
		return false;
	}

	if (MusicSheet.bHidden && !IsMusicSheetKnown(MusicID))
	{
		return false;
	}

	return true;
}

void UHarmoniaMusicComponent::EquipInstrument(const FInstrumentData& Instrument)
{
	EquippedInstrument = Instrument;
}

void UHarmoniaMusicComponent::UnequipInstrument()
{
	EquippedInstrument = FInstrumentData();
}

bool UHarmoniaMusicComponent::HasInstrumentEquipped() const
{
	return EquippedInstrument.Durability > 0;
}

void UHarmoniaMusicComponent::ReduceInstrumentDurability(int32 Amount)
{
	if (!HasInstrumentEquipped())
	{
		return;
	}

	EquippedInstrument.Durability = FMath::Max(0, EquippedInstrument.Durability - Amount);

	if (EquippedInstrument.Durability <= 0)
	{
		UnequipInstrument();
	}
}

void UHarmoniaMusicComponent::LearnMusicSheet(FName MusicID)
{
	if (!KnownMusicSheets.Contains(MusicID))
	{
		KnownMusicSheets.Add(MusicID);
		OnMusicSheetLearned.Broadcast(MusicID);
	}
}

bool UHarmoniaMusicComponent::IsMusicSheetKnown(FName MusicID) const
{
	return KnownMusicSheets.Contains(MusicID);
}

bool UHarmoniaMusicComponent::GetMusicSheetData(FName MusicID, FMusicSheetData& OutMusicSheet) const
{
	if (const FMusicSheetData* MusicSheet = MusicSheetDatabase.Find(MusicID))
	{
		OutMusicSheet = *MusicSheet;
		return true;
	}
	return false;
}

TArray<FRhythmNote> UHarmoniaMusicComponent::GenerateRhythmNotes(FName MusicID, int32 NoteCount)
{
	TArray<FRhythmNote> Notes;

	if (!MusicSheetDatabase.Contains(MusicID))
	{
		return Notes;
	}

	const FMusicSheetData& MusicSheet = MusicSheetDatabase[MusicID];
	float Duration = MusicSheet.PerformanceDuration;

	for (int32 i = 0; i < NoteCount; ++i)
	{
		FRhythmNote Note;
		Note.NoteType = FMath::RandRange(0, 3);
		Note.Timing = (Duration / NoteCount) * (i + 1);
		Note.PerfectWindow = 0.05f;
		Note.GoodWindow = 0.15f;

		Notes.Add(Note);
	}

	return Notes;
}

bool UHarmoniaMusicComponent::HitRhythmNote(int32 NoteIndex, float InputTime, bool& bPerfectHit)
{
	if (NoteIndex < 0 || NoteIndex >= CurrentRhythmNotes.Num())
	{
		return false;
	}

	const FRhythmNote& Note = CurrentRhythmNotes[NoteIndex];
	float TimeDiff = FMath::Abs(InputTime - Note.Timing);

	bPerfectHit = TimeDiff <= Note.PerfectWindow;
	bool bGoodHit = TimeDiff <= Note.GoodWindow;

	if (bPerfectHit)
	{
		MinigameScore += 100.0f;
		PerfectNoteCount++;
	}
	else if (bGoodHit)
	{
		MinigameScore += 70.0f;
	}
	else
	{
		MinigameScore += 30.0f;
	}

	OnRhythmNoteHit.Broadcast(NoteIndex, bPerfectHit);

	return bGoodHit;
}

void UHarmoniaMusicComponent::AddPerformanceExperience(int32 Amount)
{
	int32 ModifiedAmount = FMath::CeilToInt(Amount * ExperienceMultiplier);
	CurrentExperience += ModifiedAmount;

	CheckAndProcessLevelUp();
}

void UHarmoniaMusicComponent::AddTrait(const FPerformanceTrait& Trait)
{
	for (const FPerformanceTrait& ExistingTrait : ActiveTraits)
	{
		if (ExistingTrait.TraitName == Trait.TraitName)
		{
			return;
		}
	}

	ActiveTraits.Add(Trait);
}

void UHarmoniaMusicComponent::RemoveTrait(FName TraitName)
{
	ActiveTraits.RemoveAll([TraitName](const FPerformanceTrait& Trait)
	{
		return Trait.TraitName == TraitName;
	});
}

void UHarmoniaMusicComponent::CompletePerformance()
{
	if (!MusicSheetDatabase.Contains(CurrentMusicID))
	{
		CancelPerformance();
		return;
	}

	const FMusicSheetData& MusicSheet = MusicSheetDatabase[CurrentMusicID];

	// Calculate performance result
	FPerformanceResult Result = CalculatePerformanceResult(MusicSheet);

	// Gain experience
	AddPerformanceExperience(Result.Experience);

	// Gain gold
	LearnMusicSheet(CurrentMusicID);

	// Decrease instrument durability
	if (HasInstrumentEquipped())
	{
		ReduceInstrumentDurability(1);
	}

	// Apply buff
	Result.AffectedActorCount = ApplyBuffToNearbyActors(Result.AppliedBuff);

	OnPerformanceCompleted.Broadcast(Result);

	// End performance
	bIsPerforming = false;
	CurrentMusicID = NAME_None;
	CurrentRhythmNotes.Empty();
	SetComponentTickEnabled(false);
}

FPerformanceResult UHarmoniaMusicComponent::CalculatePerformanceResult(const FMusicSheetData& MusicSheet)
{
	FPerformanceResult Result;
	Result.MusicID = CurrentMusicID;

	// Calculate minigame score
	float FinalScore = 0.0f;
	if (bUseMinigame && TotalNoteCount > 0)
	{
		float MaxScore = TotalNoteCount * 100.0f;
		FinalScore = (MinigameScore / MaxScore) * 100.0f;
	}
	else
	{
		// Auto-calculate without minigame
		float SuccessRate = BaseSuccessRate + GetTotalQualityBonus();
		FinalScore = FMath::Clamp(SuccessRate, 0.0f, 100.0f);
	}

	// Determine quality
	Result.Quality = DeterminePerformanceQuality(MusicSheet.Difficulty, FinalScore);
	Result.bPerfect = Result.Quality == EPerformanceQuality::Legendary;

	// Calculate buff effect
	Result.AppliedBuff = MusicSheet.BuffEffect;

	float QualityMultiplier = 1.0f;
	if (MusicSheet.QualityMultipliers.Contains(Result.Quality))
	{
		QualityMultiplier = MusicSheet.QualityMultipliers[Result.Quality];
	}

	float BuffMultiplier = QualityMultiplier * (1.0f + GetTotalBuffEffectBonus() / 100.0f);

	// Apply multiplier to buff effect
	Result.AppliedBuff.HealthRegenPerSecond *= BuffMultiplier;
	Result.AppliedBuff.ManaRegenPerSecond *= BuffMultiplier;
	Result.AppliedBuff.StaminaRegenPerSecond *= BuffMultiplier;
	Result.AppliedBuff.AttackBonus *= BuffMultiplier;
	Result.AppliedBuff.DefenseBonus *= BuffMultiplier;
	Result.AppliedBuff.SpeedBonus *= BuffMultiplier;
	Result.AppliedBuff.CriticalChanceBonus *= BuffMultiplier;
	Result.AppliedBuff.CooldownReduction *= BuffMultiplier;
	Result.AppliedBuff.ExperienceBonus *= BuffMultiplier;

	// 범위 보너??
	Result.AppliedBuff.EffectRadius *= (1.0f + GetTotalRangeBonus() / 100.0f);

	// Check encore
	for (const FPerformanceTrait& Trait : ActiveTraits)
	{
		if (FMath::FRand() * 100.0f <= Trait.EncoreChance)
		{
			Result.AppliedBuff.Duration *= 2.0f;
			break;
		}
	}

	// 경험�?계산
	float ExpMultiplier = 1.0f;
	for (const FPerformanceTrait& Trait : ActiveTraits)
	{
		ExpMultiplier += Trait.ExperienceBonus / 100.0f;
	}

	Result.Experience = FMath::CeilToInt(MusicSheet.ExperienceReward * ExpMultiplier * (FinalScore / 50.0f));

	return Result;
}

EPerformanceQuality UHarmoniaMusicComponent::DeterminePerformanceQuality(int32 Difficulty, float Score)
{
	// Adjust quality threshold based on tier
	float LegendaryThreshold = FMath::Max(95.0f, 98.0f - (Difficulty * 0.3f));
	float PerfectThreshold = FMath::Max(85.0f, 90.0f - (Difficulty * 0.5f));
	float GreatThreshold = FMath::Max(70.0f, 75.0f - (Difficulty * 0.5f));
	float GoodThreshold = 60.0f;
	float FairThreshold = 40.0f;

	if (Score >= LegendaryThreshold)
	{
		return EPerformanceQuality::Legendary;
	}
	else if (Score >= PerfectThreshold)
	{
		return EPerformanceQuality::Perfect;
	}
	else if (Score >= GreatThreshold)
	{
		return EPerformanceQuality::Great;
	}
	else if (Score >= GoodThreshold)
	{
		return EPerformanceQuality::Good;
	}
	else if (Score >= FairThreshold)
	{
		return EPerformanceQuality::Fair;
	}
	else
	{
		return EPerformanceQuality::Poor;
	}
}

void UHarmoniaMusicComponent::ApplyMusicBuff(const FMusicBuffEffect& BuffEffect)
{
	// Individual actor buff application logic
	// Actual implementation varies by game's buff system
}

int32 UHarmoniaMusicComponent::ApplyBuffToNearbyActors(const FMusicBuffEffect& BuffEffect)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return 0;
	}

	TArray<AActor*> OverlappingActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Owner);

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Owner->GetActorLocation(),
		BuffEffect.EffectRadius,
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		OverlappingActors
	);

	int32 AffectedCount = 0;

	for (AActor* Actor : OverlappingActors)
	{
		// Need to add ally check logic
		if (BuffEffect.bAllyOnly)
		{
			// TODO: Integrate with team system to check if ally
		}

		// Apply buff
		ApplyMusicBuff(BuffEffect);
		OnMusicBuffApplied.Broadcast(Actor, BuffEffect.BuffName, BuffEffect.Duration);

		AffectedCount++;
	}

	// Apply to self
	if (BuffEffect.bAffectSelf)
	{
		ApplyMusicBuff(BuffEffect);
		OnMusicBuffApplied.Broadcast(Owner, BuffEffect.BuffName, BuffEffect.Duration);
		AffectedCount++;
	}

	return AffectedCount;
}

void UHarmoniaMusicComponent::CheckAndProcessLevelUp()
{
	int32 ExpNeeded = GetExperienceForNextLevel();

	while (CurrentExperience >= ExpNeeded)
	{
		CurrentExperience -= ExpNeeded;
		PerformanceLevel++;

		OnPerformanceLevelUp.Broadcast(PerformanceLevel, 1);

		ExpNeeded = GetExperienceForNextLevel();
	}
}

float UHarmoniaMusicComponent::GetTotalQualityBonus() const
{
	float TotalBonus = 0.0f;

	for (const FPerformanceTrait& Trait : ActiveTraits)
	{
		TotalBonus += Trait.QualityBonus;
	}

	if (HasInstrumentEquipped())
	{
		TotalBonus += EquippedInstrument.QualityBonus;
	}

	return TotalBonus;
}

float UHarmoniaMusicComponent::GetTotalBuffEffectBonus() const
{
	float TotalBonus = 0.0f;

	for (const FPerformanceTrait& Trait : ActiveTraits)
	{
		TotalBonus += Trait.BuffEffectBonus;
	}

	if (HasInstrumentEquipped())
	{
		TotalBonus += EquippedInstrument.BuffEffectBonus;
	}

	return TotalBonus;
}

float UHarmoniaMusicComponent::GetTotalRangeBonus() const
{
	float TotalBonus = 0.0f;

	for (const FPerformanceTrait& Trait : ActiveTraits)
	{
		TotalBonus += Trait.RangeBonus;
	}

	if (HasInstrumentEquipped())
	{
		TotalBonus += EquippedInstrument.RangeBonus;
	}

	return TotalBonus;
}
