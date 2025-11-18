// Copyright 2024 Snow Game Studio.

#pragma once

#include "Blueprint/UserWidget.h"
#include "HarmoniaBossHealthBarWidget.generated.h"

class AHarmoniaBossCharacter;
class ULyraHealthComponent;
class UProgressBar;
class UTextBlock;

/**
 * UBossHealthBarWidget
 *
 * Widget for displaying boss health, name, and phase information.
 */
UCLASS(Abstract, Blueprintable)
class HARMONIAKIT_API UBossHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UBossHealthBarWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	//~=============================================================================
	// Boss Binding
	//~=============================================================================

	/** Set the boss character to monitor */
	UFUNCTION(BlueprintCallable, Category = "Boss|UI")
	void SetBossCharacter(AHarmoniaBossCharacter* InBossCharacter);

	/** Get the currently bound boss character */
	UFUNCTION(BlueprintPure, Category = "Boss|UI")
	AHarmoniaBossCharacter* GetBossCharacter() const { return BossCharacter; }

	//~=============================================================================
	// Display Control
	//~=============================================================================

	/** Show the boss health bar with animation */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Boss|UI")
	void ShowBossHealthBar();
	virtual void ShowBossHealthBar_Implementation();

	/** Hide the boss health bar with animation */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Boss|UI")
	void HideBossHealthBar();
	virtual void HideBossHealthBar_Implementation();

	/** Check if health bar is currently visible */
	UFUNCTION(BlueprintPure, Category = "Boss|UI")
	bool IsBossHealthBarVisible() const { return bIsVisible; }

protected:
	//~=============================================================================
	// Update Functions
	//~=============================================================================

	/** Update health bar display */
	UFUNCTION(BlueprintNativeEvent, Category = "Boss|UI")
	void UpdateHealthBar(float HealthPercent);
	virtual void UpdateHealthBar_Implementation(float HealthPercent);

	/** Update boss name display */
	UFUNCTION(BlueprintNativeEvent, Category = "Boss|UI")
	void UpdateBossName(const FText& InBossName, const FText& InBossTitle);
	virtual void UpdateBossName_Implementation(const FText& InBossName, const FText& InBossTitle);

	/** Update phase display */
	UFUNCTION(BlueprintNativeEvent, Category = "Boss|UI")
	void UpdatePhaseDisplay(int32 CurrentPhase, int32 MaxPhases);
	virtual void UpdatePhaseDisplay_Implementation(int32 CurrentPhase, int32 MaxPhases);

	//~=============================================================================
	// Event Handlers
	//~=============================================================================

	/** Called when boss health changes */
	UFUNCTION()
	virtual void OnBossHealthChanged(ULyraHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator);

	/** Called when boss phase changes */
	UFUNCTION()
	virtual void OnBossPhaseChanged(int32 OldPhase, int32 NewPhase);

	/** Called when boss encounter starts */
	UFUNCTION()
	virtual void OnBossEncounterStart(AHarmoniaBossCharacter* Boss);

	/** Called when boss encounter ends */
	UFUNCTION()
	virtual void OnBossEncounterEnd(AHarmoniaBossCharacter* Boss, bool bDefeated);

	//~=============================================================================
	// Animation Events
	//~=============================================================================

	/** Called when phase transition animation should play */
	UFUNCTION(BlueprintNativeEvent, Category = "Boss|UI")
	void PlayPhaseTransitionAnimation(int32 NewPhase);
	virtual void PlayPhaseTransitionAnimation_Implementation(int32 NewPhase);

	/** Called when damage taken animation should play */
	UFUNCTION(BlueprintNativeEvent, Category = "Boss|UI")
	void PlayDamageTakenAnimation(float DamagePercent);
	virtual void PlayDamageTakenAnimation_Implementation(float DamagePercent);

	//~=============================================================================
	// Blueprint Bindable Widgets (bind these in UMG designer)
	//~=============================================================================

	/** Main health bar progress widget */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|UI", meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	/** Boss name text */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BossNameText;

	/** Boss title/subtitle text */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BossTitleText;

	/** Current phase indicator text */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PhaseText;

	/** Health percentage text */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|UI", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthPercentText;

	//~=============================================================================
	// Display Settings
	//~=============================================================================

	/** Color for full health */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|UI|Appearance")
	FLinearColor FullHealthColor = FLinearColor::Green;

	/** Color for medium health */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|UI|Appearance")
	FLinearColor MediumHealthColor = FLinearColor::Yellow;

	/** Color for low health */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|UI|Appearance")
	FLinearColor LowHealthColor = FLinearColor::Red;

	/** Health threshold for medium color (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|UI|Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MediumHealthThreshold = 0.5f;

	/** Health threshold for low color (0.0 - 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|UI|Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LowHealthThreshold = 0.25f;

	/** Whether to show health percentage text */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|UI|Display")
	bool bShowHealthPercentage = true;

	/** Whether to show phase indicator */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|UI|Display")
	bool bShowPhaseIndicator = true;

	/** Whether to smoothly interpolate health bar changes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|UI|Display")
	bool bSmoothHealthBarTransition = true;

	/** Speed of health bar interpolation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|UI|Display", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float HealthBarInterpolationSpeed = 5.0f;

	//~=============================================================================
	// State
	//~=============================================================================

	/** Currently bound boss character */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|UI")
	TObjectPtr<ABossCharacter> BossCharacter;

	/** Whether the health bar is visible */
	UPROPERTY(BlueprintReadOnly, Category = "Boss|UI")
	bool bIsVisible = false;

	/** Current displayed health percentage (for smooth interpolation) */
	UPROPERTY()
	float CurrentDisplayedHealth = 1.0f;

	/** Target health percentage (actual boss health) */
	UPROPERTY()
	float TargetHealth = 1.0f;

private:
	/** Unbind from current boss */
	void UnbindFromBoss();

	/** Bind to new boss */
	void BindToBoss();

	/** Get interpolated health color based on percentage */
	FLinearColor GetHealthBarColor(float HealthPercent) const;
};
