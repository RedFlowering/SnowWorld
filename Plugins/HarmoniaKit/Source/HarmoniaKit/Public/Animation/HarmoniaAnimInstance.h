// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "AlsAnimationInstance.h"
#include "Animation/IHarmoniaDodgeAnimInterface.h"
#include "HarmoniaAnimInstance.generated.h"

/**
 * HarmoniaAnimInstance
 * 
 * AnimInstance that extends ALS with Harmonia-specific features.
 * Implements IHarmoniaDodgeAnimInterface for BlendSpace dodge support.
 * 
 * Use this as the base class for your main character's AnimBP when using
 * both ALS and HarmoniaKit together.
 */
UCLASS(Blueprintable)
class HARMONIAKIT_API UHarmoniaAnimInstance : public UAlsAnimationInstance, public IHarmoniaDodgeAnimInterface
{
	GENERATED_BODY()

public:
	UHarmoniaAnimInstance();

	// ============================================================================
	// IHarmoniaDodgeAnimInterface Implementation
	// ============================================================================

	virtual void SetDodgeDirection_Implementation(float DirectionX, float DirectionY) override;
	virtual void ClearDodgeDirection_Implementation() override;

protected:
	// ============================================================================
	// Dodge State (for BlendSpace)
	// ============================================================================

	/** Dodge direction X: -1 (Left) to +1 (Right) */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dodge")
	float DodgeDirectionX = 0.0f;

	/** Dodge direction Y: -1 (Backward) to +1 (Forward) */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dodge")
	float DodgeDirectionY = 0.0f;

	/** Is currently dodging? */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|Dodge")
	bool bIsDodging = false;
};
