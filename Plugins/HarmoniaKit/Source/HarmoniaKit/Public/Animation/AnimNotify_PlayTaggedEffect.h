// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AnimNotify_PlayTaggedEffect.generated.h"

/**
 * Play Tagged Effect Notify
 * Spawns VFX and plays SFX using effect data from DataTable
 *
 * Usage:
 * 1. Create DataTable with FHarmoniaAnimationEffectData
 * 2. Configure DataTable path in HarmoniaEffectCacheSubsystem
 * 3. Place notify in animation and set EffectTag
 * 4. Effect will be spawned using cached data
 *
 * Benefits:
 * - Centralized effect management
 * - Easy bulk updates (change DataTable, affects all animations)
 * - Memory efficient (single copy per effect)
 * - Consistent effects across animations
 *
 * Example Tags:
 * - Effect.Hit.Slash.Light
 * - Effect.Hit.Slash.Heavy
 * - Effect.Hit.Blunt.Light
 * - Effect.Impact.Metal
 * - Effect.Impact.Wood
 */
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "Play Tagged Effect"))
class HARMONIAKIT_API UAnimNotify_PlayTaggedEffect : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_PlayTaggedEffect();

	//~UAnimNotify interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	//~End of UAnimNotify interface

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
	 * Effect tag - references effect data in DataTable
	 * Example: "Effect.Hit.Slash.Light"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect", meta = (Categories = "Effect"))
	FGameplayTag EffectTag;

	// ============================================================================
	// Override Options (Optional)
	// ============================================================================

	/**
	 * Override socket name from DataTable
	 * Leave empty to use DataTable value
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides")
	FName OverrideSocketName = NAME_None;

	/**
	 * Override location offset from DataTable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides")
	bool bOverrideLocationOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides", meta = (EditCondition = "bOverrideLocationOffset"))
	FVector LocationOffsetOverride = FVector::ZeroVector;

	/**
	 * Override rotation offset from DataTable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides")
	bool bOverrideRotationOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides", meta = (EditCondition = "bOverrideRotationOffset"))
	FRotator RotationOffsetOverride = FRotator::ZeroRotator;

	/**
	 * Override scale from DataTable
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides")
	bool bOverrideScale = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides", meta = (EditCondition = "bOverrideScale"))
	FVector ScaleOverride = FVector(1.0f);

	/**
	 * Scale multiplier (applied on top of DataTable scale)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|Overrides", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float ScaleMultiplier = 1.0f;

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
	 * If false, will only warn and not spawn effect
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Effect")
	bool bUseLazyLoading = true;

private:
	/** Get spawn location and rotation */
	void GetSpawnTransform(
		USkeletalMeshComponent* MeshComp,
		const struct FHarmoniaAnimationEffectData* EffectData,
		FVector& OutLocation,
		FRotator& OutRotation
	) const;

	/** Get last hit result from attack component */
	bool GetLastHitResult(AActor* Owner, FVector& OutLocation, FVector& OutNormal) const;

	/** Spawn visual effect */
	void SpawnVisualEffect(
		UWorld* World,
		USkeletalMeshComponent* MeshComp,
		const struct FHarmoniaAnimationEffectData* EffectData,
		const FVector& SpawnLocation,
		const FRotator& SpawnRotation
	) const;

	/** Play sound effect */
	void PlaySoundEffect(
		UWorld* World,
		USkeletalMeshComponent* MeshComp,
		const struct FHarmoniaAnimationEffectData* EffectData,
		const FVector& SpawnLocation,
		const FRotator& SpawnRotation
	) const;
};
