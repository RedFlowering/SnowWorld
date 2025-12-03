// Copyright 2025 Snow Game Studio.

#include "UI/HarmoniaRepairWidget.h"
#include "Interfaces/IRepairStation.h"

void UHarmoniaRepairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind to repair events
	if (EnhancementComponent)
	{
		EnhancementComponent->OnItemRepaired.AddDynamic(this, &UHarmoniaRepairWidget::HandleItemRepaired);
	}
}

void UHarmoniaRepairWidget::NativeDestruct()
{
	Super::NativeDestruct();

	// Unbind from repair events
	if (EnhancementComponent)
	{
		EnhancementComponent->OnItemRepaired.RemoveDynamic(this, &UHarmoniaRepairWidget::HandleItemRepaired);
	}
}

void UHarmoniaRepairWidget::InitializeWidget(UHarmoniaEnhancementSystemComponent* InEnhancementComponent)
{
	EnhancementComponent = InEnhancementComponent;

	if (EnhancementComponent && !EnhancementComponent->OnItemRepaired.Contains(this, TEXT("HandleItemRepaired")))
	{
		EnhancementComponent->OnItemRepaired.AddDynamic(this, &UHarmoniaRepairWidget::HandleItemRepaired);
	}
}

void UHarmoniaRepairWidget::SetRepairStation(AActor* InRepairStation)
{
	RepairStation = InRepairStation;
}

void UHarmoniaRepairWidget::SetItemToRepair(FGuid ItemGUID)
{
	SelectedItemGUID = ItemGUID;
	OnItemSelectionChanged(ItemGUID);
}

bool UHarmoniaRepairWidget::GetItemData(FEnhancedItemData& OutData) const
{
	if (!EnhancementComponent)
	{
		return false;
	}

	return EnhancementComponent->GetEnhancementData(SelectedItemGUID, OutData);
}

bool UHarmoniaRepairWidget::CanRepair(FString& OutReason) const
{
	if (!EnhancementComponent)
	{
		OutReason = TEXT("Enhancement component not set");
		return false;
	}

	if (!SelectedItemGUID.IsValid())
	{
		OutReason = TEXT("No item selected");
		return false;
	}

	return EnhancementComponent->CanRepairItem(SelectedItemGUID, OutReason);
}

int32 UHarmoniaRepairWidget::GetRepairCost(bool bFullRepair) const
{
	if (!EnhancementComponent || !SelectedItemGUID.IsValid())
	{
		return 0;
	}

	if (RepairStation)
	{
		return EnhancementComponent->GetRepairCostAtStation(SelectedItemGUID, RepairStation, bFullRepair);
	}

	return EnhancementComponent->GetRepairCost(SelectedItemGUID, bFullRepair, 0.0f);
}

FText UHarmoniaRepairWidget::GetRepairCostText(bool bFullRepair) const
{
	int32 Cost = GetRepairCost(bFullRepair);
	return FText::Format(FText::FromString(TEXT("{0} Gold")), FText::AsNumber(Cost));
}

void UHarmoniaRepairWidget::RepairItem(bool bFullRepair)
{
	if (!EnhancementComponent || !SelectedItemGUID.IsValid())
	{
		return;
	}

	OnRepairStarted();

	if (RepairStation)
	{
		EnhancementComponent->RepairItemAtStation(SelectedItemGUID, RepairStation, bFullRepair);
	}
	else
	{
		EnhancementComponent->RepairItem(SelectedItemGUID, bFullRepair, 0.0f);
	}
}

void UHarmoniaRepairWidget::RepairItemWithKit(FHarmoniaID RepairKitId)
{
	if (!EnhancementComponent || !SelectedItemGUID.IsValid())
	{
		return;
	}

	OnRepairStarted();
	EnhancementComponent->RepairItemWithKit(SelectedItemGUID, RepairKitId);
}

float UHarmoniaRepairWidget::GetDurabilityPercent() const
{
	FEnhancedItemData ItemData;
	if (!GetItemData(ItemData))
	{
		return 1.0f;
	}

	return ItemData.GetDurabilityPercent();
}

FText UHarmoniaRepairWidget::GetDurabilityText() const
{
	FEnhancedItemData ItemData;
	if (!GetItemData(ItemData))
	{
		return FText::FromString(TEXT("N/A"));
	}

	return FText::Format(
		FText::FromString(TEXT("{0} / {1}")),
		FText::AsNumber(FMath::RoundToInt(ItemData.CurrentDurability)),
		FText::AsNumber(FMath::RoundToInt(ItemData.MaxDurability))
	);
}

FLinearColor UHarmoniaRepairWidget::GetDurabilityColor() const
{
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

bool UHarmoniaRepairWidget::ShouldShowWarning() const
{
	if (!EnhancementComponent || !SelectedItemGUID.IsValid())
	{
		return false;
	}

	return EnhancementComponent->ShouldShowDurabilityWarning(SelectedItemGUID, WarningThreshold);
}

FText UHarmoniaRepairWidget::GetRepairStationName() const
{
	if (!RepairStation)
	{
		return FText::FromString(TEXT("No Station"));
	}

	if (RepairStation->Implements<URepairStation>())
	{
		IRepairStation* Station = Cast<IRepairStation>(RepairStation);
		if (Station)
		{
			return Station->Execute_GetStationDisplayName(RepairStation);
		}
	}

	return FText::FromString(RepairStation->GetName());
}

float UHarmoniaRepairWidget::GetRepairStationDiscount() const
{
	if (!RepairStation || !RepairStation->Implements<URepairStation>())
	{
		return 0.0f;
	}

	IRepairStation* Station = Cast<IRepairStation>(RepairStation);
	if (!Station)
	{
		return 0.0f;
	}

	FRepairStationData StationData = Station->Execute_GetRepairStationData(RepairStation);
	return (1.0f - StationData.CostDiscountMultiplier) * 100.0f; // Convert to percentage
}

void UHarmoniaRepairWidget::HandleItemRepaired(FGuid ItemGUID, float NewDurability)
{
	if (ItemGUID == SelectedItemGUID)
	{
		OnRepairCompleted(NewDurability);
	}
}
