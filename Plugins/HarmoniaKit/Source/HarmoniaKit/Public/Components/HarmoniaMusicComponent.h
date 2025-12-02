// Copyright Epic Games, Inc. All Rights Reserved.

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
 * ?Œì•…/?°ì£¼ ?œìŠ¤??ì»´í¬?ŒíŠ¸
 * ?…ê¸° ?°ì£¼, ë²„í”„ ?œê³µ, ë¦¬ë“¬ ë¯¸ë‹ˆê²Œì„
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
	// ?°ì£¼ ê¸°ë³¸ ê¸°ëŠ¥
	// ====================================

	/** ?°ì£¼ ?œì‘ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	bool StartPerformance(FName MusicID);

	/** ?°ì£¼ ì·¨ì†Œ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void CancelPerformance();

	/** ?°ì£¼ ì¤‘ì¸ì§€ ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool IsPerforming() const { return bIsPerforming; }

	/** ?„ì¬ ?°ì£¼ ì§„í–‰??(0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	float GetPerformanceProgress() const;

	/** ê³¡ì„ ?°ì£¼?????ˆëŠ”ì§€ ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool CanPerformMusic(FName MusicID) const;

	// ====================================
	// ?…ê¸° ê´€ë¦?
	// ====================================

	/** ?…ê¸° ?¥ì°© */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void EquipInstrument(const FInstrumentData& Instrument);

	/** ?…ê¸° ?´ì œ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void UnequipInstrument();

	/** ?„ì¬ ?¥ì°©???…ê¸° */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	FInstrumentData GetEquippedInstrument() const { return EquippedInstrument; }

	/** ?…ê¸°ê°€ ?¥ì°©?˜ì–´ ?ˆëŠ”ì§€ ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool HasInstrumentEquipped() const;

	/** ?…ê¸° ?´êµ¬??ê°ì†Œ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void ReduceInstrumentDurability(int32 Amount);

	// ====================================
	// ?…ë³´ ê´€ë¦?
	// ====================================

	/** ?…ë³´ ë°°ìš°ê¸?*/
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void LearnMusicSheet(FName MusicID);

	/** ?…ë³´ë¥??Œê³  ?ˆëŠ”ì§€ ?•ì¸ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool IsMusicSheetKnown(FName MusicID) const;

	/** ?Œê³  ?ˆëŠ” ëª¨ë“  ?…ë³´ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	TArray<FName> GetKnownMusicSheets() const { return KnownMusicSheets; }

	/** ?…ë³´ ?•ë³´ ê°€?¸ì˜¤ê¸?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool GetMusicSheetData(FName MusicID, FMusicSheetData& OutMusicSheet) const;

	// ====================================
	// ë¦¬ë“¬ ë¯¸ë‹ˆê²Œì„
	// ====================================

	/** ë¦¬ë“¬ ?¸íŠ¸ ?ì„± */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	TArray<FRhythmNote> GenerateRhythmNotes(FName MusicID, int32 NoteCount);

	/** ë¦¬ë“¬ ?¸íŠ¸ ?…ë ¥ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	bool HitRhythmNote(int32 NoteIndex, float InputTime, bool& bPerfectHit);

	/** ?„ì¬ ë¯¸ë‹ˆê²Œì„ ?ìˆ˜ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	float GetMinigameScore() const { return MinigameScore; }

	// ====================================
	// ?ˆë²¨ ë°?ê²½í—˜ì¹??œìŠ¤??
	// ====================================

	/** ?°ì£¼ ê²½í—˜ì¹??ë“ */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void AddPerformanceExperience(int32 Amount);

	/** ?„ì¬ ?°ì£¼ ?ˆë²¨ */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	int32 GetPerformanceLevel() const { return PerformanceLevel; }

	// ====================================
	// ?¹ì„± ?œìŠ¤??
	// ====================================

	/** ?¹ì„± ì¶”ê? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void AddTrait(const FPerformanceTrait& Trait);

	/** ?¹ì„± ?œê±° */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void RemoveTrait(FName TraitName);

	/** ëª¨ë“  ?¹ì„± ê°€?¸ì˜¤ê¸?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	TArray<FPerformanceTrait> GetAllTraits() const { return ActiveTraits; }

	// ====================================
	// ?´ë²¤??
	// ====================================

	/** ?°ì£¼ ?œì‘ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceStarted OnPerformanceStarted;

	/** ?°ì£¼ ì·¨ì†Œ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceCancelled OnPerformanceCancelled;

	/** ?°ì£¼ ?„ë£Œ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceCompleted OnPerformanceCompleted;

	/** ?ˆë²¨???´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceLevelUp OnPerformanceLevelUp;

	/** ?…ë³´ ?µë“ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnMusicSheetLearned OnMusicSheetLearned;

	/** ?Œì•… ë²„í”„ ?ìš© ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnMusicBuffApplied OnMusicBuffApplied;

	/** ë¦¬ë“¬ ?¸íŠ¸ ?ˆíŠ¸ ?´ë²¤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnRhythmNoteHit OnRhythmNoteHit;

	// ====================================
	// ?¤ì •
	// ====================================

	/** ?…ë³´ ?°ì´?°ë² ?´ìŠ¤ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings")
	TMap<FName, FMusicSheetData> MusicSheetDatabase;

	/** ê¸°ë³¸ ?°ì£¼ ?±ê³µë¥?(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings")
	float BaseSuccessRate = 70.0f;

	/** ë¯¸ë‹ˆê²Œì„ ?¬ìš© ?¬ë? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings")
	bool bUseMinigame = true;

private:
	/** ?°ì£¼ ì¤??Œë˜ê·?*/
	UPROPERTY()
	bool bIsPerforming = false;

	/** ?„ì¬ ?°ì£¼ ì¤‘ì¸ ê³?*/
	UPROPERTY()
	FName CurrentMusicID;

	/** ?°ì£¼ ?œì‘ ?œê°„ */
	UPROPERTY()
	float PerformanceStartTime = 0.0f;

	/** ?°ì£¼ ?„ë£Œê¹Œì? ?„ìš”???œê°„ */
	UPROPERTY()
	float RequiredPerformanceTime = 0.0f;

	/** ?¥ì°©???…ê¸° */
	UPROPERTY()
	FInstrumentData EquippedInstrument;

	/** ?°ì£¼ ?ˆë²¨ */
	UPROPERTY()
	int32 PerformanceLevel = 1;

	/** ?Œê³  ?ˆëŠ” ?…ë³´ */
	UPROPERTY()
	TArray<FName> KnownMusicSheets;

	/** ?œì„±?”ëœ ?¹ì„± ëª©ë¡ */
	UPROPERTY()
	TArray<FPerformanceTrait> ActiveTraits;

	/** ?„ì¬ ë¦¬ë“¬ ?¸íŠ¸ */
	UPROPERTY()
	TArray<FRhythmNote> CurrentRhythmNotes;

	/** ë¯¸ë‹ˆê²Œì„ ?ìˆ˜ */
	UPROPERTY()
	float MinigameScore = 0.0f;

	/** ?„ë²½???¸íŠ¸ ??*/
	UPROPERTY()
	int32 PerfectNoteCount = 0;

	/** ì´??¸íŠ¸ ??*/
	UPROPERTY()
	int32 TotalNoteCount = 0;

	/** ?°ì£¼ ?„ë£Œ ì²˜ë¦¬ */
	void CompletePerformance();

	/** ?°ì£¼ ê²°ê³¼ ê³„ì‚° */
	FPerformanceResult CalculatePerformanceResult(const FMusicSheetData& MusicSheet);

	/** ?°ì£¼ ?ˆì§ˆ ê²°ì • */
	EPerformanceQuality DeterminePerformanceQuality(int32 Difficulty, float Score);

	/** ë²„í”„ ?¨ê³¼ ?ìš© */
	void ApplyMusicBuff(const FMusicBuffEffect& BuffEffect);

	/** ì£¼ë? ?¡í„°?ê²Œ ë²„í”„ ?ìš© */
	int32 ApplyBuffToNearbyActors(const FMusicBuffEffect& BuffEffect);

	/** ?ˆë²¨ ì²´í¬ ë°?ì²˜ë¦¬ */
	void CheckAndProcessLevelUp();

	/** ì´??ˆì§ˆ ë³´ë„ˆ??ê³„ì‚° */
	float GetTotalQualityBonus() const;

	/** ì´?ë²„í”„ ?¨ê³¼ ë³´ë„ˆ??ê³„ì‚° */
	float GetTotalBuffEffectBonus() const;

	/** ì´?ë²”ìœ„ ë³´ë„ˆ??ê³„ì‚° */
	float GetTotalRangeBonus() const;
};
