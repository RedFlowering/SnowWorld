// Copyright 2025 Snow Game Studio.

#include "UI/HarmoniaDurabilityBarWidget.h"

void UHarmoniaDurabilityBarWidget::SetItem(UHarmoniaEnhancementSystemComponent* InEnhancementComponent, FGuid InItemGUID)
{
	EnhancementComponent = InEnhancementComponent;
	ItemGUID = InItemGUID;

	if (EnhancementComponent && ItemGUID.IsValid())
	{
		FEnhancedItemData ItemData;
		if (EnhancementComponent->GetEnhancementData(ItemGUID, ItemData))
		{
			LastDurability = ItemData.CurrentDurability;
			bWasBroken = ItemData.IsBroken();
		}
	}
}

void UHarmoniaDurabilityBarWidget::ClearItem()
{
	EnhancementComponent = nullptr;
	ItemGUID.Invalidate();
	LastDurability = 100.0f;
	bWasBroken = false;
}

float UHarmoniaDurabilityBarWidget::GetDurabilityPercent() const
{
	if (!EnhancementComponent || !ItemGUID.IsValid())
	{
		return 1.0f;
	}

	FEnhancedItemData ItemData;
	if (!EnhancementComponent->GetEnhancementData(ItemGUID, ItemData))
	{
		return 1.0f;
	}

	return ItemData.GetDurabilityPercent();
}

FText UHarmoniaDurabilityBarWidget::GetDurabilityText() const
{
	if (!EnhancementComponent || !ItemGUID.IsValid())
	{
		return FText::FromString(TEXT("N/A"));
	}

	FEnhancedItemData ItemData;
	if (!EnhancementComponent->GetEnhancementData(ItemGUID, ItemData))
	{
		return FText::FromString(TEXT("N/A"));
	}

	return FText::Format(
		FText::FromString(TEXT("{0}/{1}")),
		FText::AsNumber(FMath::RoundToInt(ItemData.CurrentDurability)),
		FText::AsNumber(FMath::RoundToInt(ItemData.MaxDurability))
	);
}

FLinearColor UHarmoniaDurabilityBarWidget::GetDurabilityBarColor() const
{
	if (IsBroken())
	{
		return BrokenColor;
	}

	float Percent = GetDurabilityPercent();

	if (Percent > 0.5f)
	{
		return HighDurabilityColor;
	}
	else if (Percent > 0.25f)
	{
		return MediumDurabilityColor;
	}
	else
	{
		return LowDurabilityColor;
	}
}

bool UHarmoniaDurabilityBarWidget::ShouldShowWarningIcon() const
{
	if (!EnhancementComponent || !ItemGUID.IsValid())
	{
		return false;
	}

	return EnhancementComponent->ShouldShowDurabilityWarning(ItemGUID, WarningThreshold);
}

bool UHarmoniaDurabilityBarWidget::IsBroken() const
{
	if (!EnhancementComponent || !ItemGUID.IsValid())
	{
		return false;
	}

	FEnhancedItemData ItemData;
	if (!EnhancementComponent->GetEnhancementData(ItemGUID, ItemData))
	{
		return false;
	}

	return ItemData.IsBroken();
}

float UHarmoniaDurabilityBarWidget::GetPenaltyPercent() const
{
	if (!EnhancementComponent || !ItemGUID.IsValid())
	{
		return 0.0f;
	}

	float Multiplier = EnhancementComponent->GetDurabilityPenaltyMultiplier(ItemGUID);
	float Penalty = 1.0f - Multiplier;
	return Penalty * 100.0f;
}

bool UHarmoniaDurabilityBarWidget::HasPenalty() const
{
	return GetPenaltyPercent() > 0.01f; // More than 0.01% penalty
}

FText UHarmoniaDurabilityBarWidget::GetPenaltyTooltipText() const
{
	if (!HasPenalty())
	{
		return FText::FromString(TEXT("No penalty"));
	}

	float PenaltyPercent = GetPenaltyPercent();
	return FText::Format(
		FText::FromString(TEXT("Performance reduced by {0}% due to low durability")),
		FText::AsNumber(FMath::RoundToInt(PenaltyPercent))
	);
}

void UHarmoniaDurabilityBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!EnhancementComponent || !ItemGUID.IsValid())
	{
		return;
	}

	// Update at specified frequency
	TimeSinceLastUpdate += InDeltaTime;
	if (TimeSinceLastUpdate < UpdateFrequency)
	{
		return;
	}

	TimeSinceLastUpdate = 0.0f;

	// Check for changes
	FEnhancedItemData ItemData;
	if (!EnhancementComponent->GetEnhancementData(ItemGUID, ItemData))
	{
		return;
	}

	// Check if durability changed
	if (!FMath::IsNearlyEqual(ItemData.CurrentDurability, LastDurability, 0.1f))
	{
		float OldDurability = LastDurability;
		LastDurability = ItemData.CurrentDurability;

		OnDurabilityChanged(ItemData.GetDurabilityPercent());

		// Check if repaired
		if (ItemData.CurrentDurability > OldDurability)
		{
			OnItemRepaired(ItemData.CurrentDurability);
		}
	}

	// Check if broken status changed
	bool bIsCurrentlyBroken = ItemData.IsBroken();
	if (bIsCurrentlyBroken && !bWasBroken)
	{
		OnItemBroken();
	}
	bWasBroken = bIsCurrentlyBroken;
}
