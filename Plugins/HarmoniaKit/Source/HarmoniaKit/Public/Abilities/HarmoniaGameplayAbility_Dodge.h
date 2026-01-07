// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "HarmoniaGameplayAbility_Dodge.generated.h"

class UHarmoniaMeleeCombatComponent;
class UBlendSpace;
class ULyraAnimInstance;

/**
 * Dodge roll type based on equipment load
 */
UENUM(BlueprintType)
enum class EDodgeRollType : uint8
{
	Light		UMETA(DisplayName = "Light Roll"),		// 0-30% load
	Medium		UMETA(DisplayName = "Medium Roll"),		// 30-70% load
	Heavy		UMETA(DisplayName = "Heavy Roll")		// 70%+ load
};

/**
 * Dodge Gameplay Ability
 * Roll/dash to avoid attacks with 8-directional BlendSpace support
 *
 * Features:
 * - 8-directional movement via BlendSpace (with fallback montage)
 * - I-frames (invincibility frames)
 * - Stamina consumption
 * - Equipment load affects roll type (Light/Medium/Heavy)
 * - Custom velocity-based movement (not Impulse)
 *
 * @see Docs/HarmoniaKit_Complete_Documentation.md Section 17.3.1 for tag configuration
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API UHarmoniaGameplayAbility_Dodge : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UHarmoniaGameplayAbility_Dodge(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~UGameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	//~End of UGameplayAbility interface

	/** Animation callbacks */
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	UFUNCTION()
	void OnMontageInterrupted();

protected:
	// ============================================================================
	// Animation Configuration
	// ============================================================================

	/** Dodge montage (uses Motion Warping for directional movement) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Animation")
	TObjectPtr<UAnimMontage> DodgeMontage;

	/** Motion Warping target name (set in montage's MotionWarping notify) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Animation")
	FName DodgeWarpTargetName = FName("DodgeTarget");

	/** Whether to use Motion Warping for dodge direction (requires MotionWarpingComponent) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Animation")
	bool bUseMotionWarping = true;

	// ============================================================================
	// Light Roll (0-30% Equipment Load)
	// ============================================================================

	/** Light roll distance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Light Roll")
	float LightRollDistance = 500.0f;

	/** Light roll duration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Light Roll")
	float LightRollDuration = 0.5f;

	/** Light roll I-frame start time */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Light Roll")
	float LightRollIFrameStartTime = 0.1f;

	/** Light roll I-frame duration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Light Roll")
	float LightRollIFrameDuration = 0.4f;

	/** Light roll stamina cost */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Light Roll")
	float LightRollStaminaCost = 15.0f;

	// ============================================================================
	// Medium Roll (30-70% Equipment Load)
	// ============================================================================

	/** Medium roll distance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Medium Roll")
	float MediumRollDistance = 400.0f;

	/** Medium roll duration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Medium Roll")
	float MediumRollDuration = 0.6f;

	/** Medium roll I-frame start time */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Medium Roll")
	float MediumRollIFrameStartTime = 0.1f;

	/** Medium roll I-frame duration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Medium Roll")
	float MediumRollIFrameDuration = 0.3f;

	/** Medium roll stamina cost */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Medium Roll")
	float MediumRollStaminaCost = 20.0f;

	// ============================================================================
	// Heavy Roll (70%+ Equipment Load)
	// ============================================================================

	/** Heavy roll distance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Heavy Roll")
	float HeavyRollDistance = 300.0f;

	/** Heavy roll duration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Heavy Roll")
	float HeavyRollDuration = 0.8f;

	/** Heavy roll I-frame start time */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Heavy Roll")
	float HeavyRollIFrameStartTime = 0.15f;

	/** Heavy roll I-frame duration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Heavy Roll")
	float HeavyRollIFrameDuration = 0.2f;

	/** Heavy roll stamina cost */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge|Heavy Roll")
	float HeavyRollStaminaCost = 30.0f;

	/** Cached melee combat component */
	UPROPERTY()
	TObjectPtr<UHarmoniaMeleeCombatComponent> MeleeCombatComponent;

private:
	UHarmoniaMeleeCombatComponent* GetMeleeCombatComponent() const;

	/** Determine dodge roll type based on equipment load */
	EDodgeRollType DetermineRollType() const;

	/** Get dodge parameters for current roll type */
	void GetDodgeParameters(float& OutDistance, float& OutDuration, float& OutIFrameStart, float& OutIFrameDuration, float& OutStaminaCost) const;

	/** Calculate dodge direction based on player input (returns normalized local direction) */
	FVector2D CalculateDodgeDirection(const ACharacter* Character) const;

	/** Calculate world-space dodge velocity based on direction and distance */
	FVector CalculateDodgeVelocity(const ACharacter* Character, const FVector2D& LocalDirection) const;

	/** Apply custom dodge velocity to movement component */
	void ApplyDodgeVelocity(ACharacter* Character, const FVector& Velocity);

	/** Clear dodge velocity and restore normal movement */
	void ClearDodgeVelocity();

	/** Setup Motion Warping target for directional dodge */
	void SetupMotionWarpingTarget(ACharacter* Character, const FVector2D& Direction);

	/** Start i-frames */
	void StartIFrames();

	/** End i-frames */
	void EndIFrames();

	/** Dodge movement tick handler */
	void OnDodgeMovementTick();

	/** End dodge movement */
	void OnDodgeMovementEnd();

	FTimerHandle IFrameStartTimerHandle;
	FTimerHandle IFrameEndTimerHandle;
	FTimerHandle DodgeMovementTimerHandle;
	FTimerHandle DodgeEndTimerHandle;

	/** Current dodge parameters (set during activation) */
	float CurrentDodgeDistance;
	float CurrentDodgeDuration;
	float CurrentIFrameStartTime;
	float CurrentIFrameDuration;
	float CurrentStaminaCost;

	/** Current dodge direction (local space: X = Right, Y = Forward) */
	FVector2D CurrentDodgeDirection;

	/** Current dodge velocity (world space) */
	FVector CurrentDodgeVelocity;

	/** Cached character for dodge movement */
	TWeakObjectPtr<ACharacter> CachedCharacter;
};
