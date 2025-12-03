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
 * ?�악/?�주 ?�스??컴포?�트
 * ?�기 ?�주, 버프 ?�공, 리듬 미니게임
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
	// ?�주 기본 기능
	// ====================================

	/** ?�주 ?�작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	bool StartPerformance(FName MusicID);

	/** ?�주 취소 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void CancelPerformance();

	/** ?�주 중인지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool IsPerforming() const { return bIsPerforming; }

	/** ?�재 ?�주 진행??(0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	float GetPerformanceProgress() const;

	/** 곡을 ?�주?????�는지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool CanPerformMusic(FName MusicID) const;

	// ====================================
	// ?�기 관�?
	// ====================================

	/** ?�기 ?�착 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void EquipInstrument(const FInstrumentData& Instrument);

	/** ?�기 ?�제 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void UnequipInstrument();

	/** ?�재 ?�착???�기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	FInstrumentData GetEquippedInstrument() const { return EquippedInstrument; }

	/** ?�기가 ?�착?�어 ?�는지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool HasInstrumentEquipped() const;

	/** ?�기 ?�구??감소 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void ReduceInstrumentDurability(int32 Amount);

	// ====================================
	// ?�보 관�?
	// ====================================

	/** ?�보 배우�?*/
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void LearnMusicSheet(FName MusicID);

	/** ?�보�??�고 ?�는지 ?�인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool IsMusicSheetKnown(FName MusicID) const;

	/** ?�고 ?�는 모든 ?�보 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	TArray<FName> GetKnownMusicSheets() const { return KnownMusicSheets; }

	/** ?�보 ?�보 가?�오�?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool GetMusicSheetData(FName MusicID, FMusicSheetData& OutMusicSheet) const;

	// ====================================
	// 리듬 미니게임
	// ====================================

	/** 리듬 ?�트 ?�성 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	TArray<FRhythmNote> GenerateRhythmNotes(FName MusicID, int32 NoteCount);

	/** 리듬 ?�트 ?�력 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	bool HitRhythmNote(int32 NoteIndex, float InputTime, bool& bPerfectHit);

	/** ?�재 미니게임 ?�수 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	float GetMinigameScore() const { return MinigameScore; }

	// ====================================
	// ?�벨 �?경험�??�스??
	// ====================================

	/** ?�주 경험�??�득 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void AddPerformanceExperience(int32 Amount);

	/** ?�재 ?�주 ?�벨 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	int32 GetPerformanceLevel() const { return PerformanceLevel; }

	// ====================================
	// ?�성 ?�스??
	// ====================================

	/** ?�성 추�? */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void AddTrait(const FPerformanceTrait& Trait);

	/** ?�성 ?�거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void RemoveTrait(FName TraitName);

	/** 모든 ?�성 가?�오�?*/
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	TArray<FPerformanceTrait> GetAllTraits() const { return ActiveTraits; }

	// ====================================
	// ?�벤??
	// ====================================

	/** ?�주 ?�작 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceStarted OnPerformanceStarted;

	/** ?�주 취소 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceCancelled OnPerformanceCancelled;

	/** ?�주 ?�료 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceCompleted OnPerformanceCompleted;

	/** ?�벨???�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceLevelUp OnPerformanceLevelUp;

	/** ?�보 ?�득 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnMusicSheetLearned OnMusicSheetLearned;

	/** ?�악 버프 ?�용 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnMusicBuffApplied OnMusicBuffApplied;

	/** 리듬 ?�트 ?�트 ?�벤??*/
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnRhythmNoteHit OnRhythmNoteHit;

	// ====================================
	// ?�정
	// ====================================

	/** ?�보 ?�이?�베?�스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings")
	TMap<FName, FMusicSheetData> MusicSheetDatabase;

	/** 기본 ?�주 ?�공�?(%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings")
	float BaseSuccessRate = 70.0f;

	/** 미니게임 ?�용 ?��? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings")
	bool bUseMinigame = true;

private:
	/** ?�주 �??�래�?*/
	UPROPERTY()
	bool bIsPerforming = false;

	/** ?�재 ?�주 중인 �?*/
	UPROPERTY()
	FName CurrentMusicID;

	/** ?�주 ?�작 ?�간 */
	UPROPERTY()
	float PerformanceStartTime = 0.0f;

	/** ?�주 ?�료까�? ?�요???�간 */
	UPROPERTY()
	float RequiredPerformanceTime = 0.0f;

	/** ?�착???�기 */
	UPROPERTY()
	FInstrumentData EquippedInstrument;

	/** ?�주 ?�벨 */
	UPROPERTY()
	int32 PerformanceLevel = 1;

	/** ?�고 ?�는 ?�보 */
	UPROPERTY()
	TArray<FName> KnownMusicSheets;

	/** ?�성?�된 ?�성 목록 */
	UPROPERTY()
	TArray<FPerformanceTrait> ActiveTraits;

	/** ?�재 리듬 ?�트 */
	UPROPERTY()
	TArray<FRhythmNote> CurrentRhythmNotes;

	/** 미니게임 ?�수 */
	UPROPERTY()
	float MinigameScore = 0.0f;

	/** ?�벽???�트 ??*/
	UPROPERTY()
	int32 PerfectNoteCount = 0;

	/** �??�트 ??*/
	UPROPERTY()
	int32 TotalNoteCount = 0;

	/** ?�주 ?�료 처리 */
	void CompletePerformance();

	/** ?�주 결과 계산 */
	FPerformanceResult CalculatePerformanceResult(const FMusicSheetData& MusicSheet);

	/** ?�주 ?�질 결정 */
	EPerformanceQuality DeterminePerformanceQuality(int32 Difficulty, float Score);

	/** 버프 ?�과 ?�용 */
	void ApplyMusicBuff(const FMusicBuffEffect& BuffEffect);

	/** 주�? ?�터?�게 버프 ?�용 */
	int32 ApplyBuffToNearbyActors(const FMusicBuffEffect& BuffEffect);

	/** ?�벨 체크 �?처리 */
	void CheckAndProcessLevelUp();

	/** �??�질 보너??계산 */
	float GetTotalQualityBonus() const;

	/** �?버프 ?�과 보너??계산 */
	float GetTotalBuffEffectBonus() const;

	/** �?범위 보너??계산 */
	float GetTotalRangeBonus() const;
};
