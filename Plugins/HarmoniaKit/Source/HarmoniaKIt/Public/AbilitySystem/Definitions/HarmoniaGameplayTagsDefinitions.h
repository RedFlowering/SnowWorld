// Copyright RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "HarmoniaGameplayTagsDefinitions.generated.h"

/**
 * DataTable row structure for Gameplay Tags configuration
 * Used for organizing and managing gameplay tags in HarmoniaKit
 */
USTRUCT(BlueprintType)
struct FHarmoniaGameplayTagData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// Unique identifier for this tag entry
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag")
	FName TagId;

	// The actual gameplay tag
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag", meta = (Categories = "Harmonia"))
	FGameplayTag Tag;

	// Display name for UI
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag")
	FText DisplayName;

	// Description of what this tag is used for
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag")
	FText Description;

	// Category for organization (e.g., "Ability", "Combat", "State")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag")
	FName Category;

	FHarmoniaGameplayTagData()
		: TagId(NAME_None)
		, Tag(FGameplayTag())
		, DisplayName(FText::GetEmpty())
		, Description(FText::GetEmpty())
		, Category(NAME_None)
	{
	}
};

/**
 * DataTable row structure for Combo Attack configuration
 * Defines combo attack sequences and their properties
 */
USTRUCT(BlueprintType)
struct FHarmoniaComboAttackData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// Combo index (0, 1, 2, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	int32 ComboIndex;

	// Tag for this combo step
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo", meta = (Categories = "Harmonia.Ability.Attack.Combo"))
	FGameplayTag ComboTag;

	// Animation montage to play for this combo
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	TSoftObjectPtr<UAnimMontage> AttackMontage;

	// Damage multiplier for this combo step
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float DamageMultiplier;

	// Duration in seconds to wait for next combo input before resetting
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float ComboWindowDuration;

	// Time in the animation when the next combo can be triggered (normalized 0-1)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float ComboInputStartTime;

	// Time in the animation when the combo input window closes (normalized 0-1)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combo")
	float ComboInputEndTime;

	FHarmoniaComboAttackData()
		: ComboIndex(0)
		, ComboTag(FGameplayTag())
		, AttackMontage(nullptr)
		, DamageMultiplier(1.0f)
		, ComboWindowDuration(1.5f)
		, ComboInputStartTime(0.5f)
		, ComboInputEndTime(0.9f)
	{
	}
};
