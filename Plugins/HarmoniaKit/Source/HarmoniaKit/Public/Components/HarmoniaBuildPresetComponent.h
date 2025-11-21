// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaEquipmentSystemDefinitions.h"
#include "GameplayTagContainer.h"
#include "HarmoniaBuildPresetComponent.generated.h"

/**
 * Data structure for a build preset
 */
USTRUCT(BlueprintType)
struct FBuildPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Preset")
	FName PresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Preset")
	TArray<FEquippedItem> Equipment;

	// Placeholder for abilities - in a real scenario, this would save active ability configurations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Preset")
	FGameplayTagContainer AbilityTags;

	FBuildPreset() : PresetName(NAME_None) {}
};

/**
 * Component to manage build presets (Equipment + Abilities)
 */
UCLASS(ClassGroup = (Harmonia), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaBuildPresetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaBuildPresetComponent();

protected:
	virtual void BeginPlay() override;

public:
	/**
	 * Save current state as a new preset
	 */
	UFUNCTION(BlueprintCallable, Category = "Build Preset")
	void SavePreset(FName PresetName);

	/**
	 * Load and apply a preset
	 */
	UFUNCTION(BlueprintCallable, Category = "Build Preset")
	void LoadPreset(FName PresetName);

	/**
	 * Get all saved presets
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Build Preset")
	const TArray<FBuildPreset>& GetPresets() const { return SavedPresets; }

	/**
	 * Delete a preset
	 */
	UFUNCTION(BlueprintCallable, Category = "Build Preset")
	void DeletePreset(FName PresetName);

protected:
	/** Saved presets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Preset")
	TArray<FBuildPreset> SavedPresets;

	/** Apply equipment from preset */
	void ApplyEquipment(const TArray<FEquippedItem>& Equipment);

	/** Apply abilities from preset */
	void ApplyAbilities(const FGameplayTagContainer& AbilityTags);
};
