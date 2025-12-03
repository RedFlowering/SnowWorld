// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UI/LifeContent/HarmoniaLifeContentWidget.h"
#include "Definitions/HarmoniaMusicSystemDefinitions.h"
#include "HarmoniaMusicWidget.generated.h"

class UHarmoniaMusicComponent;
class UImage;
class UTextBlock;
class UProgressBar;
class UUniformGridPanel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUIRhythmNoteHit, int32, NoteIndex, bool, bPerfect);

/**
 * Music/Performance UI Widget
 * Displays rhythm game, performance quality, and buff effects
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaMusicWidget : public UHarmoniaLifeContentWidget
{
	GENERATED_BODY()

public:
	UHarmoniaMusicWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// ====================================
	// Setup
	// ====================================

	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Music")
	void SetMusicComponent(UHarmoniaMusicComponent* InComponent);

	virtual void RefreshDisplay() override;

	// ====================================
	// Instrument Display
	// ====================================

	/** Update equipped instrument display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Music")
	void UpdateInstrumentDisplay(EInstrumentType InstrumentType, int32 InstrumentTier, 
								 float Durability, float MaxDurability);

	// ====================================
	// Rhythm Game
	// ====================================

	/** Start rhythm game display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Music")
	void StartRhythmGame(const FText& SongName, EMusicGenre Genre, float Duration);

	/** Update rhythm note position */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Music")
	void UpdateRhythmNotes(const TArray<float>& NotePositions);

	/** Show note hit result */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Music")
	void ShowNoteHitResult(int32 NoteIndex, bool bHit, bool bPerfect);

	/** End rhythm game */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Music")
	void EndRhythmGame();

	// ====================================
	// Performance Display
	// ====================================

	/** Update performance score */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Music")
	void UpdatePerformanceScore(int32 Score, int32 Combo, EPerformanceQuality Quality);

	/** Show performance result */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Music")
	void ShowPerformanceResult(EPerformanceQuality Quality, int32 FinalScore, 
							   int32 MaxCombo, const FMusicBuffEffect& BuffApplied);

	/** Hide performance result */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Music")
	void HidePerformanceResult();

	// ====================================
	// Music Sheet Display
	// ====================================

	/** Show music sheet list */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Music")
	void RefreshMusicSheetList();

	/** Show music sheet details */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Music")
	void ShowMusicSheetDetails(FName MusicID, const FText& MusicName, EMusicGenre Genre,
							   int32 Difficulty, const FMusicBuffEffect& BuffEffect);

	// ====================================
	// Events
	// ====================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|UI|Music")
	FOnUIRhythmNoteHit OnRhythmNoteHit;

protected:
	// ====================================
	// Instrument Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> InstrumentIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> InstrumentNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> InstrumentTierText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> InstrumentDurabilityBar;

	// ====================================
	// Rhythm Game Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> RhythmGamePanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SongNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GenreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> SongProgressBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> NoteTrackWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> HitZoneWidget;

	// ====================================
	// Performance Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ComboText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QualityText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> HitResultWidget;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HitResultText;

	// ====================================
	// Result Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> PerformanceResultPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FinalScoreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MaxComboText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FinalQualityText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BuffAppliedText;

	// ====================================
	// Music Sheet Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUniformGridPanel> MusicSheetListPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> MusicSheetDetailsPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SheetNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SheetGenreText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SheetDifficultyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SheetBuffText;

	// ====================================
	// References
	// ====================================

	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|UI|Music")
	TObjectPtr<UHarmoniaMusicComponent> MusicComponent;

	// ====================================
	// State
	// ====================================

	UPROPERTY()
	bool bIsRhythmGameActive = false;

	UPROPERTY()
	float RhythmGameStartTime = 0.0f;

	UPROPERTY()
	float RhythmGameDuration = 0.0f;

private:
	FText GetInstrumentTypeName(EInstrumentType Type) const;
	FText GetGenreText(EMusicGenre Genre) const;
	FText GetQualityText(EPerformanceQuality Quality) const;
	FLinearColor GetQualityColor(EPerformanceQuality Quality) const;
	FText GetDifficultyText(int32 Difficulty) const;
	FText FormatBuffEffects(const FMusicBuffEffect& BuffEffect) const;
};
