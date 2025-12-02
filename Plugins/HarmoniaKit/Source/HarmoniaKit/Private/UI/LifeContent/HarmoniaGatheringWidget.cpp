// Copyright 2025 Snow Game Studio.

#include "UI/LifeContent/HarmoniaGatheringWidget.h"
#include "Components/HarmoniaGatheringComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

UHarmoniaGatheringWidget::UHarmoniaGatheringWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UHarmoniaGatheringWidget::NativeConstruct()
{
	Super::NativeConstruct();
	HideResourceInfo();
}

void UHarmoniaGatheringWidget::SetGatheringComponent(UHarmoniaGatheringComponent* InComponent)
{
	GatheringComponent = InComponent;
	SetLifeContentComponent(InComponent);
}

void UHarmoniaGatheringWidget::RefreshDisplay()
{
	Super::RefreshDisplay();
}

void UHarmoniaGatheringWidget::UpdateToolDisplay(EGatheringToolType ToolType, int32 ToolTier, float Durability, float MaxDurability)
{
	if (ToolNameText)
	{
		ToolNameText->SetText(GetToolTypeName(ToolType));
	}

	if (ToolTierText)
	{
		ToolTierText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaGathering", "ToolTier", "?±Í∏â {0}"),
			FText::AsNumber(ToolTier)));
	}

	if (ToolDurabilityBar)
	{
		const float DurabilityPercent = MaxDurability > 0.0f ? Durability / MaxDurability : 0.0f;
		ToolDurabilityBar->SetPercent(DurabilityPercent);

		// Change color based on durability
		if (DurabilityPercent > 0.5f)
		{
			ToolDurabilityBar->SetFillColorAndOpacity(FLinearColor::Green);
		}
		else if (DurabilityPercent > 0.25f)
		{
			ToolDurabilityBar->SetFillColorAndOpacity(FLinearColor::Yellow);
		}
		else
		{
			ToolDurabilityBar->SetFillColorAndOpacity(FLinearColor::Red);
		}
	}

	if (ToolDurabilityText)
	{
		ToolDurabilityText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaGathering", "DurabilityFormat", "{0}/{1}"),
			FText::AsNumber(FMath::RoundToInt(Durability)),
			FText::AsNumber(FMath::RoundToInt(MaxDurability))));
	}
}

void UHarmoniaGatheringWidget::ShowResourceInfo(const FGatheringResourceData& ResourceData)
{
	if (ResourceInfoPanel)
	{
		ResourceInfoPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (ResourceNameText)
	{
		ResourceNameText->SetText(FText::FromName(ResourceData.ResourceName));
	}

	if (ResourceTypeText)
	{
		ResourceTypeText->SetText(GetResourceTypeName(ResourceData.ResourceType));
	}

	if (ResourceRarityText)
	{
		ResourceRarityText->SetText(GetRarityText(ResourceData.Rarity));
		ResourceRarityText->SetColorAndOpacity(GetRarityColor(ResourceData.Rarity));
	}

	if (RequiredLevelText)
	{
		RequiredLevelText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaGathering", "RequiredLevel", "?ÑÏöî ?àÎ≤®: {0}"),
			FText::AsNumber(ResourceData.MinGatheringLevel)));
	}
}

void UHarmoniaGatheringWidget::HideResourceInfo()
{
	if (ResourceInfoPanel)
	{
		ResourceInfoPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UHarmoniaGatheringWidget::ShowGatheredResult(const FName& ResourceName, int32 Amount, bool bCritical)
{
	if (GatheredResultPanel)
	{
		GatheredResultPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (GatheredResultText)
	{
		if (bCritical)
		{
			GatheredResultText->SetText(FText::Format(
				NSLOCTEXT("HarmoniaGathering", "CriticalGather", "???¨Î¶¨?∞Ïª¨! {0} x{1} ??),
				FText::FromName(ResourceName),
				FText::AsNumber(Amount)));
			GatheredResultText->SetColorAndOpacity(FLinearColor(1.0f, 0.84f, 0.0f)); // Gold
		}
		else
		{
			GatheredResultText->SetText(FText::Format(
				NSLOCTEXT("HarmoniaGathering", "NormalGather", "{0} x{1}"),
				FText::FromName(ResourceName),
				FText::AsNumber(Amount)));
			GatheredResultText->SetColorAndOpacity(FLinearColor::White);
		}
	}
}

FText UHarmoniaGatheringWidget::GetToolTypeName(EGatheringToolType ToolType) const
{
	switch (ToolType)
	{
	case EGatheringToolType::None:
		return NSLOCTEXT("HarmoniaGathering", "ToolNone", "?ÜÏùå");
	case EGatheringToolType::Pickaxe:
		return NSLOCTEXT("HarmoniaGathering", "ToolPickaxe", "Í≥°Í¥≠??);
	case EGatheringToolType::Axe:
		return NSLOCTEXT("HarmoniaGathering", "ToolAxe", "?ÑÎÅº");
	case EGatheringToolType::Sickle:
		return NSLOCTEXT("HarmoniaGathering", "ToolSickle", "??);
	case EGatheringToolType::Hammer:
		return NSLOCTEXT("HarmoniaGathering", "ToolHammer", "ÎßùÏπò");
	default:
		return FText::GetEmpty();
	}
}

FText UHarmoniaGatheringWidget::GetResourceTypeName(EGatheringResourceType ResourceType) const
{
	switch (ResourceType)
	{
	case EGatheringResourceType::Mineral:
		return NSLOCTEXT("HarmoniaGathering", "TypeMineral", "Í¥ëÎ¨º");
	case EGatheringResourceType::Herb:
		return NSLOCTEXT("HarmoniaGathering", "TypeHerb", "?ΩÏ¥à");
	case EGatheringResourceType::Wood:
		return NSLOCTEXT("HarmoniaGathering", "TypeWood", "Î™©Ïû¨");
	case EGatheringResourceType::Fiber:
		return NSLOCTEXT("HarmoniaGathering", "TypeFiber", "?¨Ïú†");
	case EGatheringResourceType::Stone:
		return NSLOCTEXT("HarmoniaGathering", "TypeStone", "?ùÏû¨");
	case EGatheringResourceType::Crystal:
		return NSLOCTEXT("HarmoniaGathering", "TypeCrystal", "?¨Î¶¨?§ÌÉà");
	case EGatheringResourceType::Flower:
		return NSLOCTEXT("HarmoniaGathering", "TypeFlower", "ÍΩ?);
	case EGatheringResourceType::Mushroom:
		return NSLOCTEXT("HarmoniaGathering", "TypeMushroom", "Î≤ÑÏÑØ");
	default:
		return FText::GetEmpty();
	}
}

FText UHarmoniaGatheringWidget::GetRarityText(EGatheringRarity Rarity) const
{
	switch (Rarity)
	{
	case EGatheringRarity::Common:
		return NSLOCTEXT("HarmoniaGathering", "RarityCommon", "?ºÎ∞ò");
	case EGatheringRarity::Uncommon:
		return NSLOCTEXT("HarmoniaGathering", "RarityUncommon", "Í≥†Í∏â");
	case EGatheringRarity::Rare:
		return NSLOCTEXT("HarmoniaGathering", "RarityRare", "?¨Í?");
	case EGatheringRarity::Epic:
		return NSLOCTEXT("HarmoniaGathering", "RarityEpic", "?ÅÏõÖ");
	case EGatheringRarity::Legendary:
		return NSLOCTEXT("HarmoniaGathering", "RarityLegendary", "?ÑÏÑ§");
	default:
		return FText::GetEmpty();
	}
}

FLinearColor UHarmoniaGatheringWidget::GetRarityColor(EGatheringRarity Rarity) const
{
	switch (Rarity)
	{
	case EGatheringRarity::Common:
		return FLinearColor::White;
	case EGatheringRarity::Uncommon:
		return FLinearColor::Green;
	case EGatheringRarity::Rare:
		return FLinearColor(0.0f, 0.5f, 1.0f);
	case EGatheringRarity::Epic:
		return FLinearColor(0.5f, 0.0f, 0.5f);
	case EGatheringRarity::Legendary:
		return FLinearColor(1.0f, 0.5f, 0.0f);
	default:
		return FLinearColor::White;
	}
}
