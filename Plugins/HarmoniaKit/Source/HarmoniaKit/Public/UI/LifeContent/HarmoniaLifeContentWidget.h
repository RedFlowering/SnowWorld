// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HarmoniaLifeContentWidget.generated.h"

class UHarmoniaBaseLifeContentComponent;
class UProgressBar;
class UTextBlock;

/**
 * Base widget for all Life Content UI (Fishing, Gathering, Cooking, Farming, Music)
 * Provides common UI elements: level display, experience bar, activity status
 */
UCLASS(Abstract)
class HARMONIAKIT_API UHarmoniaLifeContentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UHarmoniaLifeContentWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// ====================================
	// Setup
	// ====================================

	/** Set the life content component to display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|LifeContent")
	virtual void SetLifeContentComponent(UHarmoniaBaseLifeContentComponent* InComponent);

	/** Refresh the widget display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|LifeContent")
	virtual void RefreshDisplay();

	// ====================================
	// UI Update Functions
	// ====================================

	/** Update level display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|LifeContent")
	void UpdateLevelDisplay();

	/** Update experience bar */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|LifeContent")
	void UpdateExperienceBar();

	/** Update activity status display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|LifeContent")
	void UpdateActivityStatus();

protected:
	// ====================================
	// Event Handlers
	// ====================================

	UFUNCTION()
	void OnLevelUp(int32 NewLevel, int32 SkillPoints);

	UFUNCTION()
	void OnActivityStarted();

	UFUNCTION()
	void OnActivityCancelled();

	UFUNCTION()
	void OnActivityCompleted();

	// ====================================
	// Widget Bindings
	// ====================================

	/** Level text display */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LevelText;

	/** Experience progress bar */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ExperienceBar;

	/** Experience text (current/required) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ExperienceText;

	/** Activity status text */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ActivityStatusText;

	/** Activity progress bar */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ActivityProgressBar;

	// ====================================
	// References
	// ====================================

	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|UI|LifeContent")
	TObjectPtr<UHarmoniaBaseLifeContentComponent> LifeContentComponent;

	// ====================================
	// Settings
	// ====================================

	/** Whether to update activity progress every tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|UI|LifeContent")
	bool bUpdateActivityProgressOnTick = true;
};
