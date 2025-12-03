// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "AnimNotifyState_PlayTaggedWeaponTrail.generated.h"

/**
 * Play Tagged Weapon Trail Notify State
 * Spawns and manages weapon trail VFX using effect data from DataTable
 *
 * Usage:
 * 1. Create DataTable with FHarmoniaAnimationEffectData
 * 2. Configure trail-specific settings (TrailStartSocketName, TrailEndSocketName)
 * 3. Place notify state over attack swing duration
 * 4. Trail appears during state, disappears when state ends
 *
 * Benefits:
 * - Centralized trail management
 * - Easy bulk updates (change DataTable, affects all animations)
 * - Consistent trails across animations
 * - Memory efficient
 *
 * Example Tags:
 * - Effect.Trail.Sword.Normal
 * - Effect.Trail.Sword.Fire
 * - Effect.Trail.Axe.Heavy
 * - Effect.Trail.Dagger.Fast
 */
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Play Tagged Weapon Trail"))
class HARMONIAKIT_API UAnimNotifyState_PlayTaggedWeaponTrail : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_PlayTaggedWeaponTrail();

	//~UAnimNotifyState interface
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	//~End of UAnimNotifyState interface

#if WITH_EDITOR
	//~UObject interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~End of UObject interface
#endif

protected:
	// ============================================================================
	// Effect Configuration
	// ============================================================================

	/**
	 * Effect tag - references trail effect data in DataTable
	 * Example: "Effect.Trail.Sword.Normal"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect", meta = (Categories = "Effect.Trail"))
	FGameplayTag EffectTag;

	// ============================================================================
	// Override Options (Optional)
	// ============================================================================

	/**
	 * Override trail start socket from DataTable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides")
	bool bOverrideTrailSockets = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides", meta = (EditCondition = "bOverrideTrailSockets"))
	FName TrailStartSocketOverride = FName("weapon_trail_start");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides", meta = (EditCondition = "bOverrideTrailSockets"))
	FName TrailEndSocketOverride = FName("weapon_trail_end");

	/**
	 * Override trail width from DataTable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides")
	bool bOverrideTrailWidth = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides", meta = (EditCondition = "bOverrideTrailWidth", ClampMin = "0.1", ClampMax = "5.0"))
	float TrailWidthOverride = 1.0f;

	/**
	 * Width multiplier (applied on top of DataTable width)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float WidthMultiplier = 1.0f;

	/**
	 * Override trail color from DataTable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides")
	bool bOverrideColor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides", meta = (EditCondition = "bOverrideColor"))
	FLinearColor ColorOverride = FLinearColor::White;

	/**
	 * Volume multiplier (applied on top of DataTable volume)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float VolumeMultiplier = 1.0f;

	// ============================================================================
	// Advanced
	// ============================================================================

	/**
	 * Whether to use lazy loading if effect not cached
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Effect")
	bool bUseLazyLoading = true;

private:
	/** Spawned trail component */
	UPROPERTY(Transient)
	TObjectPtr<USceneComponent> SpawnedTrailComponent;

	/** Mesh component that owns this notify */
	UPROPERTY(Transient)
	TWeakObjectPtr<USkeletalMeshComponent> CachedMeshComp;

	/** Spawn trail effect */
	void SpawnTrailEffect(
		UWorld* World,
		USkeletalMeshComponent* MeshComp,
		const struct FHarmoniaAnimationEffectData* EffectData
	);

	/** Play trail sound */
	void PlayTrailSound(
		UWorld* World,
		USkeletalMeshComponent* MeshComp,
		const struct FHarmoniaAnimationEffectData* EffectData
	) const;

	/** Destroy spawned trail */
	void DestroyTrail();
};
