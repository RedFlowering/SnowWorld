// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UI/LifeContent/HarmoniaLifeContentWidget.h"
#include "Definitions/HarmoniaFishingSystemDefinitions.h"
#include "HarmoniaFishingWidget.generated.h"

class UHarmoniaFishingComponent;
class UImage;
class UTextBlock;
class UProgressBar;
class UUniformGridPanel;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinigameInputRequired, EFishingMinigameType, MinigameType);

/**
 * Fishing UI Widget
 * Displays fishing status, minigame, and caught fish information
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaFishingWidget : public UHarmoniaLifeContentWidget
{
	GENERATED_BODY()

public:
	UHarmoniaFishingWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// ====================================
	// Setup
	// ====================================

	/** Set the fishing component */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Fishing")
	void SetFishingComponent(UHarmoniaFishingComponent* InComponent);

	virtual void RefreshDisplay() override;

	// ====================================
	// Minigame UI
	// ====================================

	/** Start minigame display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Fishing")
	void ShowMinigame(EFishingMinigameType MinigameType);

	/** Hide minigame display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Fishing")
	void HideMinigame();

	/** Update minigame progress (0-1) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Fishing")
	void UpdateMinigameProgress(float Progress);

	/** Update timing indicator position (0-1) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Fishing")
	void UpdateTimingIndicator(float Position);

	/** Update tension meter (0-1, with safe zone 0.3-0.7) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Fishing")
	void UpdateTensionMeter(float Tension);

	/** Show minigame result */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Fishing")
	void ShowMinigameResult(bool bSuccess, bool bPerfect);

	// ====================================
	// Catch Display
	// ====================================

	/** Show caught fish info */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Fishing")
	void ShowCaughtFish(const FCaughtFish& FishInfo, const FFishData& FishData);

	/** Hide caught fish display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Fishing")
	void HideCaughtFish();

	// ====================================
	// Fishing Spot Info
	// ====================================

	/** Update fishing spot display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Fishing")
	void UpdateFishingSpotInfo(const UFishingSpotData* SpotData);

	// ====================================
	// Events
	// ====================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|UI|Fishing")
	FOnMinigameInputRequired OnMinigameInputRequired;

protected:
	// ====================================
	// Fishing Spot Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FishingSpotNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FishingSpotTypeText;

	// ====================================
	// Minigame Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> MinigamePanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> MinigameProgressBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> TimingBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> TimingIndicator;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> TensionMeter;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MinigameInstructionText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MinigameResultText;

	// ====================================
	// Caught Fish Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> CaughtFishPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CaughtFishIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CaughtFishNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CaughtFishRarityText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CaughtFishSizeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CaughtFishWeightText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CaughtFishQualityText;

	// ====================================
	// References
	// ====================================

	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|UI|Fishing")
	TObjectPtr<UHarmoniaFishingComponent> FishingComponent;

	// ====================================
	// State
	// ====================================

	UPROPERTY()
	EFishingMinigameType CurrentMinigameType = EFishingMinigameType::TimingBased;

	UPROPERTY()
	bool bIsMinigameActive = false;

private:
	FText GetMinigameInstruction(EFishingMinigameType Type) const;
	FText GetRarityText(EFishRarity Rarity) const;
	FLinearColor GetRarityColor(EFishRarity Rarity) const;
};
