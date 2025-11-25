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
 * 음악/연주 시스템 컴포넌트
 * 악기 연주, 버프 제공, 리듬 미니게임
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
	// 연주 기본 기능
	// ====================================

	/** 연주 시작 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	bool StartPerformance(FName MusicID);

	/** 연주 취소 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void CancelPerformance();

	/** 연주 중인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool IsPerforming() const { return bIsPerforming; }

	/** 현재 연주 진행도 (0-1) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	float GetPerformanceProgress() const;

	/** 곡을 연주할 수 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool CanPerformMusic(FName MusicID) const;

	// ====================================
	// 악기 관리
	// ====================================

	/** 악기 장착 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void EquipInstrument(const FInstrumentData& Instrument);

	/** 악기 해제 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void UnequipInstrument();

	/** 현재 장착된 악기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	FInstrumentData GetEquippedInstrument() const { return EquippedInstrument; }

	/** 악기가 장착되어 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool HasInstrumentEquipped() const;

	/** 악기 내구도 감소 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void ReduceInstrumentDurability(int32 Amount);

	// ====================================
	// 악보 관리
	// ====================================

	/** 악보 배우기 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void LearnMusicSheet(FName MusicID);

	/** 악보를 알고 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool IsMusicSheetKnown(FName MusicID) const;

	/** 알고 있는 모든 악보 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	TArray<FName> GetKnownMusicSheets() const { return KnownMusicSheets; }

	/** 악보 정보 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	bool GetMusicSheetData(FName MusicID, FMusicSheetData& OutMusicSheet) const;

	// ====================================
	// 리듬 미니게임
	// ====================================

	/** 리듬 노트 생성 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	TArray<FRhythmNote> GenerateRhythmNotes(FName MusicID, int32 NoteCount);

	/** 리듬 노트 입력 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	bool HitRhythmNote(int32 NoteIndex, float InputTime, bool& bPerfectHit);

	/** 현재 미니게임 점수 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	float GetMinigameScore() const { return MinigameScore; }

	// ====================================
	// 레벨 및 경험치 시스템
	// ====================================

	/** 연주 경험치 획득 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void AddPerformanceExperience(int32 Amount);

	/** 현재 연주 레벨 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	int32 GetPerformanceLevel() const { return PerformanceLevel; }

	// ====================================
	// 특성 시스템
	// ====================================

	/** 특성 추가 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void AddTrait(const FPerformanceTrait& Trait);

	/** 특성 제거 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Music")
	void RemoveTrait(FName TraitName);

	/** 모든 특성 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Music")
	TArray<FPerformanceTrait> GetAllTraits() const { return ActiveTraits; }

	// ====================================
	// 이벤트
	// ====================================

	/** 연주 시작 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceStarted OnPerformanceStarted;

	/** 연주 취소 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceCancelled OnPerformanceCancelled;

	/** 연주 완료 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceCompleted OnPerformanceCompleted;

	/** 레벨업 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnPerformanceLevelUp OnPerformanceLevelUp;

	/** 악보 습득 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnMusicSheetLearned OnMusicSheetLearned;

	/** 음악 버프 적용 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnMusicBuffApplied OnMusicBuffApplied;

	/** 리듬 노트 히트 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Harmonia|Music")
	FOnRhythmNoteHit OnRhythmNoteHit;

	// ====================================
	// 설정
	// ====================================

	/** 악보 데이터베이스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings")
	TMap<FName, FMusicSheetData> MusicSheetDatabase;

	/** 기본 연주 성공률 (%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings")
	float BaseSuccessRate = 70.0f;

	/** 미니게임 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Settings")
	bool bUseMinigame = true;

private:
	/** 연주 중 플래그 */
	UPROPERTY()
	bool bIsPerforming = false;

	/** 현재 연주 중인 곡 */
	UPROPERTY()
	FName CurrentMusicID;

	/** 연주 시작 시간 */
	UPROPERTY()
	float PerformanceStartTime = 0.0f;

	/** 연주 완료까지 필요한 시간 */
	UPROPERTY()
	float RequiredPerformanceTime = 0.0f;

	/** 장착된 악기 */
	UPROPERTY()
	FInstrumentData EquippedInstrument;

	/** 연주 레벨 */
	UPROPERTY()
	int32 PerformanceLevel = 1;

	/** 알고 있는 악보 */
	UPROPERTY()
	TArray<FName> KnownMusicSheets;

	/** 활성화된 특성 목록 */
	UPROPERTY()
	TArray<FPerformanceTrait> ActiveTraits;

	/** 현재 리듬 노트 */
	UPROPERTY()
	TArray<FRhythmNote> CurrentRhythmNotes;

	/** 미니게임 점수 */
	UPROPERTY()
	float MinigameScore = 0.0f;

	/** 완벽한 노트 수 */
	UPROPERTY()
	int32 PerfectNoteCount = 0;

	/** 총 노트 수 */
	UPROPERTY()
	int32 TotalNoteCount = 0;

	/** 연주 완료 처리 */
	void CompletePerformance();

	/** 연주 결과 계산 */
	FPerformanceResult CalculatePerformanceResult(const FMusicSheetData& MusicSheet);

	/** 연주 품질 결정 */
	EPerformanceQuality DeterminePerformanceQuality(int32 Difficulty, float Score);

	/** 버프 효과 적용 */
	void ApplyMusicBuff(const FMusicBuffEffect& BuffEffect);

	/** 주변 액터에게 버프 적용 */
	int32 ApplyBuffToNearbyActors(const FMusicBuffEffect& BuffEffect);

	/** 레벨 체크 및 처리 */
	void CheckAndProcessLevelUp();

	/** 총 품질 보너스 계산 */
	float GetTotalQualityBonus() const;

	/** 총 버프 효과 보너스 계산 */
	float GetTotalBuffEffectBonus() const;

	/** 총 범위 보너스 계산 */
	float GetTotalRangeBonus() const;
};
