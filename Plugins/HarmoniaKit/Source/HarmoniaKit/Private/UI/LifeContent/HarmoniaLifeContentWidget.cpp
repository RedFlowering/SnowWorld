// Copyright 2025 Snow Game Studio.

#include "UI/LifeContent/HarmoniaLifeContentWidget.h"
#include "Components/HarmoniaBaseLifeContentComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

UHarmoniaLifeContentWidget::UHarmoniaLifeContentWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UHarmoniaLifeContentWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UHarmoniaLifeContentWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bUpdateActivityProgressOnTick && LifeContentComponent && LifeContentComponent->IsActivityActive())
	{
		UpdateActivityStatus();
	}
}

void UHarmoniaLifeContentWidget::SetLifeContentComponent(UHarmoniaBaseLifeContentComponent* InComponent)
{
	// Unbind from old component
	if (LifeContentComponent)
	{
		LifeContentComponent->OnLevelUp.RemoveDynamic(this, &UHarmoniaLifeContentWidget::OnLevelUp);
		LifeContentComponent->OnActivityStarted.RemoveDynamic(this, &UHarmoniaLifeContentWidget::OnActivityStarted);
		LifeContentComponent->OnActivityCancelled.RemoveDynamic(this, &UHarmoniaLifeContentWidget::OnActivityCancelled);
		LifeContentComponent->OnActivityCompleted.RemoveDynamic(this, &UHarmoniaLifeContentWidget::OnActivityCompleted);
	}

	LifeContentComponent = InComponent;

	// Bind to new component
	if (LifeContentComponent)
	{
		LifeContentComponent->OnLevelUp.AddDynamic(this, &UHarmoniaLifeContentWidget::OnLevelUp);
		LifeContentComponent->OnActivityStarted.AddDynamic(this, &UHarmoniaLifeContentWidget::OnActivityStarted);
		LifeContentComponent->OnActivityCancelled.AddDynamic(this, &UHarmoniaLifeContentWidget::OnActivityCancelled);
		LifeContentComponent->OnActivityCompleted.AddDynamic(this, &UHarmoniaLifeContentWidget::OnActivityCompleted);
	}

	RefreshDisplay();
}

void UHarmoniaLifeContentWidget::RefreshDisplay()
{
	UpdateLevelDisplay();
	UpdateExperienceBar();
	UpdateActivityStatus();
}

void UHarmoniaLifeContentWidget::UpdateLevelDisplay()
{
	if (LevelText && LifeContentComponent)
	{
		LevelText->SetText(FText::AsNumber(LifeContentComponent->GetLevel()));
	}
}

void UHarmoniaLifeContentWidget::UpdateExperienceBar()
{
	if (!LifeContentComponent)
	{
		return;
	}

	const int32 CurrentExp = LifeContentComponent->GetCurrentExperience();
	const int32 RequiredExp = LifeContentComponent->GetExperienceForNextLevel();
	const float Progress = RequiredExp > 0 ? static_cast<float>(CurrentExp) / static_cast<float>(RequiredExp) : 0.0f;

	if (ExperienceBar)
	{
		ExperienceBar->SetPercent(Progress);
	}

	if (ExperienceText)
	{
		ExperienceText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaLifeContent", "ExpFormat", "{0} / {1}"),
			FText::AsNumber(CurrentExp),
			FText::AsNumber(RequiredExp)));
	}
}

void UHarmoniaLifeContentWidget::UpdateActivityStatus()
{
	if (!LifeContentComponent)
	{
		return;
	}

	const bool bIsActive = LifeContentComponent->IsActivityActive();
	const float Progress = LifeContentComponent->GetActivityProgress();

	if (ActivityProgressBar)
	{
		ActivityProgressBar->SetPercent(Progress);
		ActivityProgressBar->SetVisibility(bIsActive ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	if (ActivityStatusText)
	{
		if (bIsActive)
		{
			ActivityStatusText->SetText(FText::Format(
				NSLOCTEXT("HarmoniaLifeContent", "ActivityProgress", "ì§„í–‰ ì¤?.. {0}%"),
				FText::AsNumber(FMath::RoundToInt(Progress * 100.0f))));
		}
		else
		{
			ActivityStatusText->SetText(NSLOCTEXT("HarmoniaLifeContent", "ActivityReady", "ì¤€ë¹??„ë£Œ"));
		}
	}
}

void UHarmoniaLifeContentWidget::OnLevelUp(int32 NewLevel, int32 SkillPoints)
{
	UpdateLevelDisplay();
	UpdateExperienceBar();
}

void UHarmoniaLifeContentWidget::OnActivityStarted()
{
	UpdateActivityStatus();
}

void UHarmoniaLifeContentWidget::OnActivityCancelled()
{
	UpdateActivityStatus();
}

void UHarmoniaLifeContentWidget::OnActivityCompleted()
{
	UpdateActivityStatus();
	UpdateExperienceBar();
}
