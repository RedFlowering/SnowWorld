// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UI/LifeContent/HarmoniaLifeContentWidget.h"
#include "Definitions/HarmoniaGatheringSystemDefinitions.h"
#include "HarmoniaGatheringWidget.generated.h"

class UHarmoniaGatheringComponent;
class UImage;
class UTextBlock;
class UProgressBar;

/**
 * Gathering UI Widget
 * Displays gathering progress, equipped tool, and resource information
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaGatheringWidget : public UHarmoniaLifeContentWidget
{
	GENERATED_BODY()

public:
	UHarmoniaGatheringWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

public:
	// ====================================
	// Setup
	// ====================================

	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Gathering")
	void SetGatheringComponent(UHarmoniaGatheringComponent* InComponent);

	virtual void RefreshDisplay() override;

	// ====================================
	// Tool Display
	// ====================================

	/** Update equipped tool display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Gathering")
	void UpdateToolDisplay(EGatheringToolType ToolType, int32 ToolTier, float Durability, float MaxDurability);

	// ====================================
	// Resource Display
	// ====================================

	/** Show target resource info */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Gathering")
	void ShowResourceInfo(const FGatheringResourceData& ResourceData);

	/** Hide resource info */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Gathering")
	void HideResourceInfo();

	/** Show gathered result */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Gathering")
	void ShowGatheredResult(const FName& ResourceName, int32 Amount, bool bCritical);

protected:
	// ====================================
	// Tool Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ToolIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ToolNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ToolTierText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ToolDurabilityBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ToolDurabilityText;

	// ====================================
	// Resource Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> ResourceInfoPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ResourceIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ResourceNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ResourceTypeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ResourceRarityText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RequiredLevelText;

	// ====================================
	// Result Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> GatheredResultPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GatheredResultText;

	// ====================================
	// References
	// ====================================

	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|UI|Gathering")
	TObjectPtr<UHarmoniaGatheringComponent> GatheringComponent;

private:
	FText GetToolTypeName(EGatheringToolType ToolType) const;
	FText GetResourceTypeName(EGatheringResourceType ResourceType) const;
	FText GetRarityText(EGatheringRarity Rarity) const;
	FLinearColor GetRarityColor(EGatheringRarity Rarity) const;
};
