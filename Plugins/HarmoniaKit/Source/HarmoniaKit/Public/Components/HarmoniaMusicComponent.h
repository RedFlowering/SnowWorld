// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * @file HarmoniaMusicComponent.h
 * @brief Music performance system component for instrument playing and buffs
 * @author Harmonia Team
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/HarmoniaBaseLifeContentComponent.h"
#include "Definitions/HarmoniaMusicSystemDefinitions.h"
#include "HarmoniaMusicComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPerformanceStarted, FName, MusicID, float, Duration, EMusicGenre, Genre);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPerformanceCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceCompleted, const FPerformanceResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerformanceLevelUp, int32, NewLevel, int32, SkillPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMusicSheetLearned, FName, MusicID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMusicBuffApplied, AActor*, Target, FName, BuffName, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRhythmNoteHit, int32, NoteIndex, bool, bPerfect);

/**
 * @class UHarmoniaMusicComponent
 * @brief Music/Performance system component
 * 
 * Handles instrument playing, buff provision, and rhythm minigame.
 */
UCLASS(ClassGroup=(HarmoniaKit), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaMusicComponent : public UHarmoniaBaseLifeContentComponent
{
	GENERATED_BODY()

public:	
	UHarmoniaMusicComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ====================================
	// Performance Basic Functions
	// ====================================

	/** Start performance */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	bool StartPerformance(FName MusicID);

	/** Cancel current performance */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void CancelPerformance();

	/** Check if currently performing */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool IsPerforming() const { return bIsPerforming; }

	/** Get current performance progress (0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	float GetPerformanceProgress() const;

	/** Check if can perform the music */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool CanPerformMusic(FName MusicID) const;

	// ====================================
	// Instrument Management
	// ====================================

	/** Equip instrument */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void EquipInstrument(const FInstrumentData& Instrument);

	/** Unequip current instrument */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void UnequipInstrument();

	/** Get currently equipped instrument */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	FInstrumentData GetEquippedInstrument() const { return EquippedInstrument; }

	/** Check if instrument is equipped */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool HasInstrumentEquipped() const;

	/** Reduce instrument durability */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void ReduceInstrumentDurability(int32 Amount);

	// ====================================
	// Music Sheet Management
	// ====================================

	/** Learn music sheet */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void LearnMusicSheet(FName MusicID);

	/** Check if music sheet is known */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool IsMusicSheetKnown(FName MusicID) const;

	/** Get all known music sheets */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	TArray<FName> GetKnownMusicSheets() const { return KnownMusicSheets; }

	/** Get music sheet data */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool GetMusicSheetData(FName MusicID, FMusicSheetData& OutMusicSheet) const;

	// ====================================
	// Rhythm Minigame
	// ====================================

	/** Generate rhythm notes */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	TArray<FRhythmNote> GenerateRhythmNotes(FName MusicID, int32 NoteCount);

	/** Hit rhythm note input */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	bool HitRhythmNote(int32 NoteIndex, float InputTime, bool& bPerfectHit);

	/** Get current minigame score */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	float GetMinigameScore() const { return MinigameScore; }

	// ====================================
	// Level & Experience System
	// ====================================

	/** Add performance experience */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void AddPerformanceExperience(int32 Amount);

	/** Get current performance level */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	int32 GetPerformanceLevel() const { return PerformanceLevel; }

	// ====================================
	// Trait System
	// ====================================

	/** Add performance trait */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void AddTrait(const FPerformanceTrait& Trait);

	/** Remove performance trait */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void RemoveTrait(FName TraitName);

	/** Get all active traits */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	TArray<FPerformanceTrait> GetAllTraits() const { return ActiveTraits; }

	// ====================================
	// Events
	// ====================================

	/** Event fired when performance starts */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceStarted OnPerformanceStarted;

	/** Event fired when performance is cancelled */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceCancelled OnPerformanceCancelled;

	/** Event fired when performance completes */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceCompleted OnPerformanceCompleted;

	/** Event fired on level up */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceLevelUp OnPerformanceLevelUp;

	/** Event fired when music sheet is learned */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnMusicSheetLearned OnMusicSheetLearned;

	/** Event fired when music buff is applied */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnMusicBuffApplied OnMusicBuffApplied;

	/** Event fired when rhythm note is hit */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnRhythmNoteHit OnRhythmNoteHit;

	// ====================================
	// Settings
	// ====================================

	/** Music sheet database */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings")
	TMap<FName, FMusicSheetData> MusicSheetDatabase;

	/** Base performance success rate (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings")
	float BaseSuccessRate = 70.0f;

	/** Whether to use minigame */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings")
	bool bUseMinigame = true;

private:
	/** Performance status flag */
	UPROPERTY()
	bool bIsPerforming = false;

	/** Currently performing music ID */
	UPROPERTY()
	FName CurrentMusicID;

	/** Performance start time */
	UPROPERTY()
	float PerformanceStartTime = 0.0f;

	/** Required time to complete performance */
	UPROPERTY()
	float RequiredPerformanceTime = 0.0f;

	/** Equipped instrument data */
	UPROPERTY()
	FInstrumentData EquippedInstrument;

	/** Performance level */
	UPROPERTY()
	int32 PerformanceLevel = 1;

	/** List of known music sheets */
	UPROPERTY()
	TArray<FName> KnownMusicSheets;

	/** List of active performance traits */
	UPROPERTY()
	TArray<FPerformanceTrait> ActiveTraits;

	/** Current rhythm notes */
	UPROPERTY()
	TArray<FRhythmNote> CurrentRhythmNotes;

	/** Minigame score */
	UPROPERTY()
	float MinigameScore = 0.0f;

	/** Perfect note hit count */
	UPROPERTY()
	int32 PerfectNoteCount = 0;

	/** Total note count */
	UPROPERTY()
	int32 TotalNoteCount = 0;

	/** Process performance completion */
	void CompletePerformance();

	/** Calculate performance result */
	FPerformanceResult CalculatePerformanceResult(const FMusicSheetData& MusicSheet);

	/** Determine performance quality */
	EPerformanceQuality DeterminePerformanceQuality(int32 Difficulty, float Score);

	/** Apply music buff effect */
	void ApplyMusicBuff(const FMusicBuffEffect& BuffEffect);

	/** Apply buff to nearby actors */
	int32 ApplyBuffToNearbyActors(const FMusicBuffEffect& BuffEffect);

	/** Check and process level up */
	void CheckAndProcessLevelUp();

	/** Calculate total quality bonus */
	float GetTotalQualityBonus() const;

	/** Calculate total buff effect bonus */
	float GetTotalBuffEffectBonus() const;

	/** Calculate total range bonus */
	float GetTotalRangeBonus() const;
};
