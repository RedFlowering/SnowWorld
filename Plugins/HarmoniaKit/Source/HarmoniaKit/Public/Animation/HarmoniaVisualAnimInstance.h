// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "HarmoniaVisualAnimInstance.generated.h"

class ACharacter;
class UAnimInstance;

/**
 * HarmoniaVisualAnimInstance
 * 
 * AnimInstance for visual/cosmetic child meshes.
 * References the parent character's main AnimInstance for data synchronization.
 * 
 * Usage:
 * - Assign to visual mesh (child actor or attached skeletal mesh)
 * - Automatically references parent character's AnimInstance
 * - Can copy relevant animation state from parent
 */
UCLASS(Blueprintable)
class HARMONIAKIT_API UHarmoniaVisualAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UHarmoniaVisualAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// ============================================================================
	// Parent Reference
	// ============================================================================

	/** Get the parent character (owner of the main mesh) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia Visual AnimInstance")
	ACharacter* GetParentCharacter() const { return ParentCharacter.Get(); }

	/** Get the parent AnimInstance (main mesh's AnimInstance) */
	UFUNCTION(BlueprintCallable, Category = "Harmonia Visual AnimInstance", Meta = (BlueprintThreadSafe))
	UAnimInstance* GetParentAnimInstanceUnsafe() const { return ParentAnimInstance.Get(); }

protected:
	/** Cached parent character */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia Visual AnimInstance")
	TWeakObjectPtr<ACharacter> ParentCharacter;

	/** Cached parent AnimInstance */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia Visual AnimInstance")
	TWeakObjectPtr<UAnimInstance> ParentAnimInstance;

	// ============================================================================
	// Synced State (copy from parent for visual mesh)
	// ============================================================================

	/** Dodge direction X (synced from parent) */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia Visual AnimInstance|Dodge")
	float DodgeDirectionX = 0.0f;

	/** Dodge direction Y (synced from parent) */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia Visual AnimInstance|Dodge")
	float DodgeDirectionY = 0.0f;

	/** Is currently dodging (synced from parent) */
	UPROPERTY(BlueprintReadOnly, Category = "Harmonia Visual AnimInstance|Dodge")
	bool bIsDodging = false;

private:
	/** Sync dodge state from parent AnimInstance */
	void SyncDodgeStateFromParent();
};
