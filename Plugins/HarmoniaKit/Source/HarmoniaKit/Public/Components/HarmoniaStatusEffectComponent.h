// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "HarmoniaStatusEffectComponent.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class UHarmoniaUIConfigDataAsset;

/**
 * Status effect type
 */
UENUM(BlueprintType)
enum class EHarmoniaStatusEffectType : uint8
{
	Buff,          // Positive effect
	Debuff,        // Negative effect
	Neutral        // Neither positive nor negative
};

/**
 * Status effect display category
 */
UENUM(BlueprintType)
enum class EHarmoniaStatusEffectCategory : uint8
{
	Combat,        // Combat-related (damage, defense, etc.)
	Movement,      // Movement-related (speed, jump, etc.)
	Resource,      // Resource-related (health, mana regen, etc.)
	Control,       // Control effects (stun, slow, etc.)
	Elemental,     // Elemental effects (burn, freeze, etc.)
	Special        // Special/unique effects
};

/**
 * Status effect visual data for UI
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaStatusEffectUIData
{
	GENERATED_BODY()

	/** Unique identifier */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect")
	FGuid EffectID;

	/** Effect tag for identification */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	FGameplayTag EffectTag;

	/** Associated gameplay effect class */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect")
	TSubclassOf<UGameplayEffect> EffectClass;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	FText DisplayName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	FText Description;

	/** Icon texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	TSoftObjectPtr<UTexture2D> IconTexture;

	/** Icon tint color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	FLinearColor IconColor = FLinearColor::White;

	/** Border color (based on type) */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect")
	FLinearColor BorderColor = FLinearColor::White;

	/** Effect type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	EHarmoniaStatusEffectType EffectType = EHarmoniaStatusEffectType::Neutral;

	/** Effect category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	EHarmoniaStatusEffectCategory Category = EHarmoniaStatusEffectCategory::Combat;

	/** Current stack count */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect")
	int32 StackCount = 1;

	/** Maximum stack count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	int32 MaxStackCount = 1;

	/** Whether effect has duration */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect")
	bool bHasDuration = false;

	/** Total duration in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect")
	float TotalDuration = 0.0f;

	/** Remaining duration in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect")
	float RemainingDuration = 0.0f;

	/** Start time (world time) */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect")
	float StartTime = 0.0f;

	/** Progress (0-1, for duration-based effects) */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect")
	float DurationProgress = 1.0f;

	/** Whether effect is expiring soon */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect")
	bool bIsExpiringSoon = false;

	/** Priority for display order */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	int32 DisplayPriority = 0;

	/** Whether to show on HUD */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	bool bShowOnHUD = true;

	/** Effect magnitude text (e.g., "+25%") */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect")
	FText MagnitudeText;

	/** Active effect handle (from GAS) */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect")
	FActiveGameplayEffectHandle ActiveHandle;

	FHarmoniaStatusEffectUIData()
	{
		EffectID = FGuid::NewGuid();
	}
};

/**
 * Status effect configuration (data asset friendly)
 */
USTRUCT(BlueprintType)
struct HARMONIAKIT_API FHarmoniaStatusEffectConfig
{
	GENERATED_BODY()

	/** Effect tag for matching */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FGameplayTag EffectTag;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FText DisplayName;

	/** Description template (supports {magnitude}, {duration}) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FText DescriptionTemplate;

	/** Icon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TSoftObjectPtr<UTexture2D> Icon;

	/** Effect type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EHarmoniaStatusEffectType EffectType = EHarmoniaStatusEffectType::Neutral;

	/** Effect category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EHarmoniaStatusEffectCategory Category = EHarmoniaStatusEffectCategory::Combat;

	/** Display priority */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 Priority = 0;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusEffectAdded, const FHarmoniaStatusEffectUIData&, Effect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusEffectRemoved, FGuid, EffectID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusEffectUpdated, const FHarmoniaStatusEffectUIData&, Effect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatusEffectStackChanged, FGuid, EffectID, int32, NewStackCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusEffectExpiring, const FHarmoniaStatusEffectUIData&, Effect);

/**
 * Harmonia Status Effect Component
 *
 * Manages and tracks status effects (buffs/debuffs) for UI display.
 * Integrates with Gameplay Ability System to monitor active effects.
 *
 * Features:
 * - Automatic GAS effect tracking
 * - Duration and stack management
 * - Effect categorization
 * - Expiring effect warnings
 * - Custom effect configurations
 * - Blueprint-friendly events
 */
UCLASS(ClassGroup=(Harmonia), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaStatusEffectComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// GAS Integration
	// ============================================================================

	/** Initialize with Ability System Component */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|StatusEffect")
	void InitializeWithASC(UAbilitySystemComponent* ASC);

	/** Get tracked Ability System Component */
	UFUNCTION(BlueprintPure, Category = "Harmonia|StatusEffect")
	UAbilitySystemComponent* GetAbilitySystemComponent() const { return TrackedASC.Get(); }

	// ============================================================================
	// Effect Queries
	// ============================================================================

	/** Get all active effects */
	UFUNCTION(BlueprintPure, Category = "Harmonia|StatusEffect")
	TArray<FHarmoniaStatusEffectUIData> GetAllEffects() const { return ActiveEffects; }

	/** Get effects by type (buff/debuff) */
	UFUNCTION(BlueprintPure, Category = "Harmonia|StatusEffect")
	TArray<FHarmoniaStatusEffectUIData> GetEffectsByType(EHarmoniaStatusEffectType Type) const;

	/** Get effects by category */
	UFUNCTION(BlueprintPure, Category = "Harmonia|StatusEffect")
	TArray<FHarmoniaStatusEffectUIData> GetEffectsByCategory(EHarmoniaStatusEffectCategory Category) const;

	/** Get all buffs */
	UFUNCTION(BlueprintPure, Category = "Harmonia|StatusEffect")
	TArray<FHarmoniaStatusEffectUIData> GetBuffs() const;

	/** Get all debuffs */
	UFUNCTION(BlueprintPure, Category = "Harmonia|StatusEffect")
	TArray<FHarmoniaStatusEffectUIData> GetDebuffs() const;

	/** Get effect by ID */
	UFUNCTION(BlueprintPure, Category = "Harmonia|StatusEffect")
	bool GetEffectByID(FGuid EffectID, FHarmoniaStatusEffectUIData& OutEffect) const;

	/** Get effect by tag */
	UFUNCTION(BlueprintPure, Category = "Harmonia|StatusEffect")
	bool GetEffectByTag(FGameplayTag EffectTag, FHarmoniaStatusEffectUIData& OutEffect) const;

	/** Check if effect is active */
	UFUNCTION(BlueprintPure, Category = "Harmonia|StatusEffect")
	bool IsEffectActive(FGameplayTag EffectTag) const;

	/** Get effect stack count */
	UFUNCTION(BlueprintPure, Category = "Harmonia|StatusEffect")
	int32 GetEffectStackCount(FGameplayTag EffectTag) const;

	// ============================================================================
	// Manual Effect Management (for non-GAS effects)
	// ============================================================================

	/** Add a custom status effect (not from GAS) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|StatusEffect")
	FGuid AddCustomEffect(const FHarmoniaStatusEffectUIData& Effect);

	/** Remove a custom effect */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|StatusEffect")
	bool RemoveCustomEffect(FGuid EffectID);

	/** Update custom effect duration */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|StatusEffect")
	bool UpdateCustomEffectDuration(FGuid EffectID, float NewRemainingDuration);

	/** Update custom effect stacks */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|StatusEffect")
	bool UpdateCustomEffectStacks(FGuid EffectID, int32 NewStackCount);

	// ============================================================================
	// Configuration
	// ============================================================================

	/** Register effect configuration */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|StatusEffect|Config")
	void RegisterEffectConfig(const FHarmoniaStatusEffectConfig& Config);

	/** Get effect configuration by tag */
	UFUNCTION(BlueprintPure, Category = "Harmonia|StatusEffect|Config")
	bool GetEffectConfig(FGameplayTag EffectTag, FHarmoniaStatusEffectConfig& OutConfig) const;

	/** Set UI configuration from data asset */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|StatusEffect|Config")
	void SetUIConfigDataAsset(UHarmoniaUIConfigDataAsset* ConfigAsset);

	/** Get current UI config data asset */
	UFUNCTION(BlueprintPure, Category = "Harmonia|StatusEffect|Config")
	UHarmoniaUIConfigDataAsset* GetUIConfigDataAsset() const { return UIConfigAsset; }

	/** Time threshold for "expiring soon" warning (can be overridden by data asset) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|StatusEffect")
	float ExpiringSoonThreshold = 3.0f;

	/** UI configuration data asset (overrides hardcoded values) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|StatusEffect|Config")
	TObjectPtr<UHarmoniaUIConfigDataAsset> UIConfigAsset;

	// ============================================================================
	// Events
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|StatusEffect")
	FOnStatusEffectAdded OnEffectAdded;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|StatusEffect")
	FOnStatusEffectRemoved OnEffectRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|StatusEffect")
	FOnStatusEffectUpdated OnEffectUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|StatusEffect")
	FOnStatusEffectStackChanged OnStackChanged;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|StatusEffect")
	FOnStatusEffectExpiring OnEffectExpiring;

protected:
	/** Update all effect timers */
	void UpdateEffectTimers(float DeltaTime);

	/** Handle GAS effect applied */
	void OnGameplayEffectApplied(UAbilitySystemComponent* ASC, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);

	/** Handle GAS effect removed */
	void OnGameplayEffectRemoved(const FActiveGameplayEffect& RemovedEffect);

	/** Handle GAS effect stack changed */
	void OnGameplayEffectStackChanged(FActiveGameplayEffectHandle Handle, int32 NewStackCount, int32 PreviousStackCount);

	/** Create UI data from gameplay effect */
	FHarmoniaStatusEffectUIData CreateUIDataFromEffect(const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle);

	/** Get border color based on effect type */
	FLinearColor GetBorderColorForType(EHarmoniaStatusEffectType Type) const;

	/** Format magnitude text */
	FText FormatMagnitudeText(const FGameplayEffectSpec& Spec) const;

	/** Clean up expired custom effects */
	void CleanupExpiredEffects();

private:
	/** Tracked Ability System Component */
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> TrackedASC;

	/** All active effects for UI display */
	UPROPERTY()
	TArray<FHarmoniaStatusEffectUIData> ActiveEffects;

	/** Effect configurations (tag -> config) */
	TMap<FGameplayTag, FHarmoniaStatusEffectConfig> EffectConfigs;

	/** Delegate handles for cleanup */
	FDelegateHandle OnEffectAppliedHandle;
	FDelegateHandle OnEffectRemovedHandle;
	FDelegateHandle OnStackChangedHandle;

	/** Already warned about expiring (to avoid spam) */
	TSet<FGuid> WarnedExpiringEffects;
};
