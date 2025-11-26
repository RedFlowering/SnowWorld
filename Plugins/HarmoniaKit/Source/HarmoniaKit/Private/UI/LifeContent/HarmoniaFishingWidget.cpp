// Copyright 2025 Snow Game Studio.

#include "UI/LifeContent/HarmoniaFishingWidget.h"
#include "Components/HarmoniaFishingComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"

UHarmoniaFishingWidget::UHarmoniaFishingWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UHarmoniaFishingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Hide minigame and caught fish panels initially
	HideMinigame();
	HideCaughtFish();
}

void UHarmoniaFishingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UHarmoniaFishingWidget::SetFishingComponent(UHarmoniaFishingComponent* InComponent)
{
	FishingComponent = InComponent;
	SetLifeContentComponent(InComponent);
}

void UHarmoniaFishingWidget::RefreshDisplay()
{
	Super::RefreshDisplay();
}

void UHarmoniaFishingWidget::ShowMinigame(EFishingMinigameType MinigameType)
{
	CurrentMinigameType = MinigameType;
	bIsMinigameActive = true;

	if (MinigamePanel)
	{
		MinigamePanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (MinigameInstructionText)
	{
		MinigameInstructionText->SetText(GetMinigameInstruction(MinigameType));
	}

	if (MinigameResultText)
	{
		MinigameResultText->SetVisibility(ESlateVisibility::Hidden);
	}

	// Show appropriate minigame elements based on type
	switch (MinigameType)
	{
	case EFishingMinigameType::TimingBased:
		if (TimingBar) TimingBar->SetVisibility(ESlateVisibility::Visible);
		if (TimingIndicator) TimingIndicator->SetVisibility(ESlateVisibility::Visible);
		if (TensionMeter) TensionMeter->SetVisibility(ESlateVisibility::Hidden);
		break;

	case EFishingMinigameType::BarBalance:
	case EFishingMinigameType::ReelTension:
		if (TimingBar) TimingBar->SetVisibility(ESlateVisibility::Hidden);
		if (TimingIndicator) TimingIndicator->SetVisibility(ESlateVisibility::Hidden);
		if (TensionMeter) TensionMeter->SetVisibility(ESlateVisibility::Visible);
		break;

	case EFishingMinigameType::QuickTimeEvent:
		if (TimingBar) TimingBar->SetVisibility(ESlateVisibility::Hidden);
		if (TimingIndicator) TimingIndicator->SetVisibility(ESlateVisibility::Hidden);
		if (TensionMeter) TensionMeter->SetVisibility(ESlateVisibility::Hidden);
		break;
	}

	OnMinigameInputRequired.Broadcast(MinigameType);
}

void UHarmoniaFishingWidget::HideMinigame()
{
	bIsMinigameActive = false;

	if (MinigamePanel)
	{
		MinigamePanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UHarmoniaFishingWidget::UpdateMinigameProgress(float Progress)
{
	if (MinigameProgressBar)
	{
		MinigameProgressBar->SetPercent(FMath::Clamp(Progress, 0.0f, 1.0f));
	}
}

void UHarmoniaFishingWidget::UpdateTimingIndicator(float Position)
{
	if (TimingIndicator)
	{
		// Move indicator along the timing bar (assuming horizontal layout)
		FWidgetTransform Transform;
		Transform.Translation = FVector2D(Position * 200.0f - 100.0f, 0.0f); // -100 to 100 range
		TimingIndicator->SetRenderTransform(Transform);
	}
}

void UHarmoniaFishingWidget::UpdateTensionMeter(float Tension)
{
	if (TensionMeter)
	{
		TensionMeter->SetPercent(FMath::Clamp(Tension, 0.0f, 1.0f));

		// Change color based on tension (safe zone is 0.3-0.7)
		FLinearColor TensionColor;
		if (Tension < 0.2f || Tension > 0.8f)
		{
			TensionColor = FLinearColor::Red; // Danger
		}
		else if (Tension < 0.3f || Tension > 0.7f)
		{
			TensionColor = FLinearColor::Yellow; // Warning
		}
		else
		{
			TensionColor = FLinearColor::Green; // Safe
		}
		TensionMeter->SetFillColorAndOpacity(TensionColor);
	}
}

void UHarmoniaFishingWidget::ShowMinigameResult(bool bSuccess, bool bPerfect)
{
	if (MinigameResultText)
	{
		MinigameResultText->SetVisibility(ESlateVisibility::Visible);

		if (bPerfect)
		{
			MinigameResultText->SetText(NSLOCTEXT("HarmoniaFishing", "PerfectCatch", "완벽한 낚시!"));
			MinigameResultText->SetColorAndOpacity(FLinearColor(1.0f, 0.84f, 0.0f)); // Gold
		}
		else if (bSuccess)
		{
			MinigameResultText->SetText(NSLOCTEXT("HarmoniaFishing", "GoodCatch", "성공!"));
			MinigameResultText->SetColorAndOpacity(FLinearColor::Green);
		}
		else
		{
			MinigameResultText->SetText(NSLOCTEXT("HarmoniaFishing", "FailedCatch", "놓쳤다..."));
			MinigameResultText->SetColorAndOpacity(FLinearColor::Red);
		}
	}
}

void UHarmoniaFishingWidget::ShowCaughtFish(const FCaughtFish& FishInfo, const FFishData& FishData)
{
	if (CaughtFishPanel)
	{
		CaughtFishPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (CaughtFishNameText)
	{
		CaughtFishNameText->SetText(FText::FromName(FishData.FishName));
	}

	if (CaughtFishRarityText)
	{
		CaughtFishRarityText->SetText(GetRarityText(FishData.Rarity));
		CaughtFishRarityText->SetColorAndOpacity(GetRarityColor(FishData.Rarity));
	}

	if (CaughtFishSizeText)
	{
		CaughtFishSizeText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaFishing", "SizeFormat", "{0} cm"),
			FText::AsNumber(FMath::RoundToInt(FishInfo.Size))));
	}

	if (CaughtFishWeightText)
	{
		FNumberFormattingOptions FormatOptions;
		FormatOptions.SetMaximumFractionalDigits(2);
		FormatOptions.SetUseGrouping(false);
		CaughtFishWeightText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaFishing", "WeightFormat", "{0} kg"),
			FText::AsNumber(FishInfo.Weight, &FormatOptions)));
	}

	if (CaughtFishQualityText)
	{
		CaughtFishQualityText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaFishing", "QualityFormat", "품질: {0}"),
			FText::AsNumber(FMath::RoundToInt(FishInfo.QualityScore))));
	}

	// Load and set fish icon
	if (CaughtFishIcon && !FishData.Icon.IsNull())
	{
		if (UTexture2D* IconTexture = FishData.Icon.LoadSynchronous())
		{
			CaughtFishIcon->SetBrushFromTexture(IconTexture);
		}
	}
}

void UHarmoniaFishingWidget::HideCaughtFish()
{
	if (CaughtFishPanel)
	{
		CaughtFishPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UHarmoniaFishingWidget::UpdateFishingSpotInfo(const UFishingSpotData* SpotData)
{
	if (!SpotData)
	{
		return;
	}

	if (FishingSpotNameText)
	{
		FishingSpotNameText->SetText(SpotData->SpotName);
	}

	if (FishingSpotTypeText)
	{
		FText TypeText;
		switch (SpotData->SpotType)
		{
		case EFishingSpotType::River:
			TypeText = NSLOCTEXT("HarmoniaFishing", "SpotRiver", "강");
			break;
		case EFishingSpotType::Lake:
			TypeText = NSLOCTEXT("HarmoniaFishing", "SpotLake", "호수");
			break;
		case EFishingSpotType::Ocean:
			TypeText = NSLOCTEXT("HarmoniaFishing", "SpotOcean", "바다");
			break;
		case EFishingSpotType::Pond:
			TypeText = NSLOCTEXT("HarmoniaFishing", "SpotPond", "연못");
			break;
		case EFishingSpotType::Special:
			TypeText = NSLOCTEXT("HarmoniaFishing", "SpotSpecial", "특별 낚시터");
			break;
		}
		FishingSpotTypeText->SetText(TypeText);
	}
}

FText UHarmoniaFishingWidget::GetMinigameInstruction(EFishingMinigameType Type) const
{
	switch (Type)
	{
	case EFishingMinigameType::TimingBased:
		return NSLOCTEXT("HarmoniaFishing", "InstructionTiming", "타이밍에 맞춰 버튼을 누르세요!");
	case EFishingMinigameType::BarBalance:
		return NSLOCTEXT("HarmoniaFishing", "InstructionBalance", "균형을 유지하세요!");
	case EFishingMinigameType::QuickTimeEvent:
		return NSLOCTEXT("HarmoniaFishing", "InstructionQTE", "표시되는 버튼을 빠르게 누르세요!");
	case EFishingMinigameType::ReelTension:
		return NSLOCTEXT("HarmoniaFishing", "InstructionTension", "장력을 적절히 유지하세요!");
	default:
		return FText::GetEmpty();
	}
}

FText UHarmoniaFishingWidget::GetRarityText(EFishRarity Rarity) const
{
	switch (Rarity)
	{
	case EFishRarity::Common:
		return NSLOCTEXT("HarmoniaFishing", "RarityCommon", "일반");
	case EFishRarity::Uncommon:
		return NSLOCTEXT("HarmoniaFishing", "RarityUncommon", "고급");
	case EFishRarity::Rare:
		return NSLOCTEXT("HarmoniaFishing", "RarityRare", "희귀");
	case EFishRarity::Epic:
		return NSLOCTEXT("HarmoniaFishing", "RarityEpic", "영웅");
	case EFishRarity::Legendary:
		return NSLOCTEXT("HarmoniaFishing", "RarityLegendary", "전설");
	default:
		return FText::GetEmpty();
	}
}

FLinearColor UHarmoniaFishingWidget::GetRarityColor(EFishRarity Rarity) const
{
	switch (Rarity)
	{
	case EFishRarity::Common:
		return FLinearColor::White;
	case EFishRarity::Uncommon:
		return FLinearColor::Green;
	case EFishRarity::Rare:
		return FLinearColor(0.0f, 0.5f, 1.0f); // Blue
	case EFishRarity::Epic:
		return FLinearColor(0.5f, 0.0f, 0.5f); // Purple
	case EFishRarity::Legendary:
		return FLinearColor(1.0f, 0.5f, 0.0f); // Orange
	default:
		return FLinearColor::White;
	}
}
