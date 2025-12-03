// Copyright 2025 Snow Game Studio.

#include "UI/Quest/HarmoniaQuestWidget.h"
#include "Components/HarmoniaQuestComponent.h"
#include "System/HarmoniaQuestSubsystem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "TimerManager.h"

UHarmoniaQuestWidget::UHarmoniaQuestWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UHarmoniaQuestWidget::NativeConstruct()
{
	Super::NativeConstruct();

	HideQuestDetails();
	HideTrackingPanel();

	if (NotificationPanel)
	{
		NotificationPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UHarmoniaQuestWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UHarmoniaQuestWidget::SetQuestComponent(UHarmoniaQuestComponent* InComponent)
{
	QuestComponent = InComponent;
	RefreshDisplay();
}

void UHarmoniaQuestWidget::RefreshDisplay()
{
	RefreshActiveQuests();
}

void UHarmoniaQuestWidget::RefreshQuestList(EQuestType QuestType)
{
	CurrentQuestTypeFilter = QuestType;
	// Quest list would be populated here based on QuestComponent's quests
	// Actual implementation would iterate through quests of the specified type
}

void UHarmoniaQuestWidget::RefreshActiveQuests()
{
	// Refresh the list of all active quests
	// Actual implementation would create quest entry widgets for each active quest
}

void UHarmoniaQuestWidget::FilterByState(EQuestState State)
{
	// Filter displayed quests by state
}

void UHarmoniaQuestWidget::ShowQuestDetails(FName QuestID, const FText& QuestName, const FText& Description,
											EQuestType QuestType, EQuestState QuestState)
{
	CurrentSelectedQuest = QuestID;

	if (QuestDetailsPanel)
	{
		QuestDetailsPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (QuestNameText)
	{
		QuestNameText->SetText(QuestName);
	}

	if (QuestDescriptionText)
	{
		QuestDescriptionText->SetText(Description);
	}

	if (QuestTypeText)
	{
		QuestTypeText->SetText(GetQuestTypeText(QuestType));
		QuestTypeText->SetColorAndOpacity(GetQuestTypeColor(QuestType));
	}

	if (QuestStateText)
	{
		QuestStateText->SetText(GetQuestStateText(QuestState));
		QuestStateText->SetColorAndOpacity(GetQuestStateColor(QuestState));
	}

	// Update button visibility based on quest state
	if (AcceptQuestButton)
	{
		AcceptQuestButton->SetVisibility(QuestState == EQuestState::Available ? 
			ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (AbandonQuestButton)
	{
		AbandonQuestButton->SetVisibility(QuestState == EQuestState::InProgress ? 
			ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (TrackQuestButton)
	{
		TrackQuestButton->SetVisibility(QuestState == EQuestState::InProgress ? 
			ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	OnQuestSelected.Broadcast(QuestID);
}

void UHarmoniaQuestWidget::HideQuestDetails()
{
	CurrentSelectedQuest = NAME_None;

	if (QuestDetailsPanel)
	{
		QuestDetailsPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UHarmoniaQuestWidget::UpdateObjectives(const TArray<FText>& ObjectiveTexts, const TArray<float>& ObjectiveProgress,
											const TArray<bool>& ObjectiveCompleted)
{
	if (!ObjectivesBox)
	{
		return;
	}

	ObjectivesBox->ClearChildren();

	for (int32 i = 0; i < ObjectiveTexts.Num(); ++i)
	{
		// Create objective entry widget
		// Actual implementation would use a custom objective entry widget class
		UTextBlock* ObjectiveText = NewObject<UTextBlock>(this);
		if (ObjectiveText)
		{
			FString ProgressString;
			if (i < ObjectiveProgress.Num())
			{
				ProgressString = FString::Printf(TEXT(" (%.0f%%)"), ObjectiveProgress[i] * 100.0f);
			}

			FText DisplayText = FText::Format(
				NSLOCTEXT("HarmoniaQuest", "ObjectiveFormat", "{0}{1}"),
				ObjectiveTexts[i],
				FText::FromString(ProgressString));

			ObjectiveText->SetText(DisplayText);

			if (i < ObjectiveCompleted.Num() && ObjectiveCompleted[i])
			{
				ObjectiveText->SetColorAndOpacity(FLinearColor::Green);
			}
			else
			{
				ObjectiveText->SetColorAndOpacity(FLinearColor::White);
			}

			ObjectivesBox->AddChild(ObjectiveText);
		}
	}
}

void UHarmoniaQuestWidget::SetTrackedQuest(FName QuestID)
{
	CurrentTrackedQuest = QuestID;
	ShowTrackingPanel();
}

void UHarmoniaQuestWidget::UpdateTrackedQuestDisplay(const FText& QuestName, const FText& CurrentObjective, float Progress)
{
	if (TrackedQuestNameText)
	{
		TrackedQuestNameText->SetText(QuestName);
	}

	if (TrackedObjectiveText)
	{
		TrackedObjectiveText->SetText(CurrentObjective);
	}

	if (TrackedProgressBar)
	{
		TrackedProgressBar->SetPercent(FMath::Clamp(Progress, 0.0f, 1.0f));
	}
}

void UHarmoniaQuestWidget::ShowTrackingPanel()
{
	if (TrackingPanel)
	{
		TrackingPanel->SetVisibility(ESlateVisibility::Visible);
	}
}

void UHarmoniaQuestWidget::HideTrackingPanel()
{
	CurrentTrackedQuest = NAME_None;

	if (TrackingPanel)
	{
		TrackingPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UHarmoniaQuestWidget::ShowRewards(int32 Experience, int32 Gold, const TArray<FName>& ItemRewards)
{
	if (RewardsPanel)
	{
		RewardsPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (ExperienceRewardText)
	{
		ExperienceRewardText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaQuest", "ExpReward", "Experience: {0}"),
			FText::AsNumber(Experience)));
	}

	if (GoldRewardText)
	{
		GoldRewardText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaQuest", "GoldReward", "Gold: {0}"),
			FText::AsNumber(Gold)));
	}

	if (ItemRewardsBox)
	{
		ItemRewardsBox->ClearChildren();
		for (const FName& ItemID : ItemRewards)
		{
			UTextBlock* ItemText = NewObject<UTextBlock>(this);
			if (ItemText)
			{
				ItemText->SetText(FText::FromName(ItemID));
				ItemRewardsBox->AddChild(ItemText);
			}
		}
	}
}

void UHarmoniaQuestWidget::ShowQuestAcceptedNotification(const FText& QuestName)
{
	if (NotificationPanel)
	{
		NotificationPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (NotificationText)
	{
		NotificationText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaQuest", "QuestAccepted", "Quest Accepted: {0}"),
			QuestName));
		NotificationText->SetColorAndOpacity(FLinearColor::Yellow);
	}

	HideNotificationAfterDelay();
}

void UHarmoniaQuestWidget::ShowQuestCompletedNotification(const FText& QuestName)
{
	if (NotificationPanel)
	{
		NotificationPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (NotificationText)
	{
		NotificationText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaQuest", "QuestCompleted", "Quest Completed: {0}"),
			QuestName));
		NotificationText->SetColorAndOpacity(FLinearColor::Green);
	}

	HideNotificationAfterDelay();
}

void UHarmoniaQuestWidget::ShowObjectiveUpdatedNotification(const FText& ObjectiveText, int32 Current, int32 Required)
{
	if (NotificationPanel)
	{
		NotificationPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (NotificationText)
	{
		NotificationText->SetText(FText::Format(
			NSLOCTEXT("HarmoniaQuest", "ObjectiveUpdated", "{0} ({1}/{2})"),
			ObjectiveText,
			FText::AsNumber(Current),
			FText::AsNumber(Required)));
		NotificationText->SetColorAndOpacity(FLinearColor::White);
	}

	HideNotificationAfterDelay();
}

void UHarmoniaQuestWidget::HideNotificationAfterDelay()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(NotificationTimerHandle);
		World->GetTimerManager().SetTimer(NotificationTimerHandle, [this]()
		{
			if (NotificationPanel)
			{
				NotificationPanel->SetVisibility(ESlateVisibility::Collapsed);
			}
		}, 3.0f, false);
	}
}

FText UHarmoniaQuestWidget::GetQuestTypeText(EQuestType Type) const
{
	switch (Type)
	{
	case EQuestType::Main:
		return NSLOCTEXT("HarmoniaQuest", "TypeMain", "Main Quest");
	case EQuestType::Side:
		return NSLOCTEXT("HarmoniaQuest", "TypeSide", "Side Quest");
	case EQuestType::Daily:
		return NSLOCTEXT("HarmoniaQuest", "TypeDaily", "Daily Quest");
	case EQuestType::Weekly:
		return NSLOCTEXT("HarmoniaQuest", "TypeWeekly", "Weekly Quest");
	case EQuestType::Repeatable:
		return NSLOCTEXT("HarmoniaQuest", "TypeRepeatable", "Repeatable Quest");
	case EQuestType::Story:
		return NSLOCTEXT("HarmoniaQuest", "TypeStory", "Story Quest");
	case EQuestType::Tutorial:
		return NSLOCTEXT("HarmoniaQuest", "TypeTutorial", "Tutorial");
	case EQuestType::Achievement:
		return NSLOCTEXT("HarmoniaQuest", "TypeAchievement", "Achievement");
	default:
		return FText::GetEmpty();
	}
}

FText UHarmoniaQuestWidget::GetQuestStateText(EQuestState State) const
{
	switch (State)
	{
	case EQuestState::Locked:
		return NSLOCTEXT("HarmoniaQuest", "StateLocked", "Locked");
	case EQuestState::Available:
		return NSLOCTEXT("HarmoniaQuest", "StateAvailable", "Available");
	case EQuestState::InProgress:
		return NSLOCTEXT("HarmoniaQuest", "StateInProgress", "In Progress");
	case EQuestState::ReadyToComplete:
		return NSLOCTEXT("HarmoniaQuest", "StateReadyComplete", "Ready to Complete");
	case EQuestState::Completed:
		return NSLOCTEXT("HarmoniaQuest", "StateCompleted", "Completed");
	case EQuestState::Failed:
		return NSLOCTEXT("HarmoniaQuest", "StateFailed", "Failed");
	default:
		return FText::GetEmpty();
	}
}

FLinearColor UHarmoniaQuestWidget::GetQuestTypeColor(EQuestType Type) const
{
	switch (Type)
	{
	case EQuestType::Main:
		return FLinearColor(1.0f, 0.84f, 0.0f); // Gold
	case EQuestType::Story:
		return FLinearColor(0.5f, 0.0f, 0.5f); // Purple
	case EQuestType::Side:
		return FLinearColor(0.0f, 0.5f, 1.0f); // Blue
	case EQuestType::Daily:
		return FLinearColor::Green;
	case EQuestType::Weekly:
		return FLinearColor(0.0f, 0.8f, 0.8f); // Cyan
	case EQuestType::Repeatable:
		return FLinearColor::White;
	case EQuestType::Tutorial:
		return FLinearColor::Yellow;
	case EQuestType::Achievement:
		return FLinearColor(1.0f, 0.5f, 0.0f); // Orange
	default:
		return FLinearColor::White;
	}
}

FLinearColor UHarmoniaQuestWidget::GetQuestStateColor(EQuestState State) const
{
	switch (State)
	{
	case EQuestState::Locked:
		return FLinearColor::Gray;
	case EQuestState::Available:
		return FLinearColor::Yellow;
	case EQuestState::InProgress:
		return FLinearColor::White;
	case EQuestState::ReadyToComplete:
		return FLinearColor::Green;
	case EQuestState::Completed:
		return FLinearColor(0.5f, 1.0f, 0.5f); // Light green
	case EQuestState::Failed:
		return FLinearColor::Red;
	default:
		return FLinearColor::White;
	}
}
