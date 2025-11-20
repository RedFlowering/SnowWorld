// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Definitions/HarmoniaEnhancementSystemDefinitions.h"
#include "IRepairStation.generated.h"

/**
 * Repair station type
 */
UENUM(BlueprintType)
enum class ERepairStationType : uint8
{
	None			UMETA(DisplayName = "None"),
	BasicAnvil		UMETA(DisplayName = "Basic Anvil"),			// Basic repairs, low discount
	Blacksmith		UMETA(DisplayName = "Blacksmith"),			// Standard repairs, medium discount
	MasterForge		UMETA(DisplayName = "Master Forge"),		// Advanced repairs, high discount
	ArcaneFurnace	UMETA(DisplayName = "Arcane Furnace"),		// Magical item repairs, special bonuses
	Traveling		UMETA(DisplayName = "Traveling Repair"),	// Portable, high cost
	MAX				UMETA(Hidden)
};

/**
 * Repair station stats and bonuses
 */
USTRUCT(BlueprintType)
struct FRepairStationData
{
	GENERATED_BODY()

	/** Station type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair Station")
	ERepairStationType StationType = ERepairStationType::None;

	/** Cost discount multiplier (0.0 - 1.0, where 0.5 = 50% off) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair Station", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CostDiscountMultiplier = 1.0f;

	/** Repair quality bonus (adds extra durability %) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair Station")
	float QualityBonus = 0.0f;

	/** Can repair magical items? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair Station")
	bool bCanRepairMagicalItems = true;

	/** Can repair cursed items? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair Station")
	bool bCanRepairCursedItems = false;

	/** Maximum item grade that can be repaired */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair Station")
	EItemGrade MaxRepairableGrade = EItemGrade::Legendary;

	/** Repair speed multiplier (for timed repairs) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair Station")
	float RepairSpeedMultiplier = 1.0f;

	/** Special tags granted during repair */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair Station")
	FGameplayTagContainer GrantedTags;

	FRepairStationData()
		: StationType(ERepairStationType::None)
		, CostDiscountMultiplier(1.0f)
		, QualityBonus(0.0f)
		, bCanRepairMagicalItems(true)
		, bCanRepairCursedItems(false)
		, MaxRepairableGrade(EItemGrade::Legendary)
		, RepairSpeedMultiplier(1.0f)
	{}
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class URepairStation : public UInterface
{
	GENERATED_BODY()
};

/**
 * Repair Station Interface
 * Implement this interface on actors that can repair equipment
 * Examples: Blacksmith NPCs, Anvils, Repair Benches
 */
class HARMONIAKIT_API IRepairStation
{
	GENERATED_BODY()

public:
	/**
	 * Get repair station type
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair Station")
	ERepairStationType GetRepairStationType() const;
	virtual ERepairStationType GetRepairStationType_Implementation() const { return ERepairStationType::BasicAnvil; }

	/**
	 * Get repair station data
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair Station")
	FRepairStationData GetRepairStationData() const;
	virtual FRepairStationData GetRepairStationData_Implementation() const { return FRepairStationData(); }

	/**
	 * Is station available for use?
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair Station")
	bool IsAvailableForRepair(AActor* RequestingActor) const;
	virtual bool IsAvailableForRepair_Implementation(AActor* RequestingActor) const { return true; }

	/**
	 * Can repair specific item?
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair Station")
	bool CanRepairItem(FGuid ItemGUID, FString& OutReason) const;
	virtual bool CanRepairItem_Implementation(FGuid ItemGUID, FString& OutReason) const { return true; }

	/**
	 * Get repair cost with station discount applied
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair Station")
	int32 GetDiscountedRepairCost(FGuid ItemGUID, int32 BaseCost) const;
	virtual int32 GetDiscountedRepairCost_Implementation(FGuid ItemGUID, int32 BaseCost) const { return BaseCost; }

	/**
	 * Called when repair starts at this station
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair Station")
	void OnRepairStarted(AActor* RequestingActor, FGuid ItemGUID);
	virtual void OnRepairStarted_Implementation(AActor* RequestingActor, FGuid ItemGUID) {}

	/**
	 * Called when repair completes at this station
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair Station")
	void OnRepairCompleted(AActor* RequestingActor, FGuid ItemGUID, float DurabilityRestored);
	virtual void OnRepairCompleted_Implementation(AActor* RequestingActor, FGuid ItemGUID, float DurabilityRestored) {}

	/**
	 * Get station display name
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair Station")
	FText GetStationDisplayName() const;
	virtual FText GetStationDisplayName_Implementation() const { return FText::FromString(TEXT("Repair Station")); }
};
