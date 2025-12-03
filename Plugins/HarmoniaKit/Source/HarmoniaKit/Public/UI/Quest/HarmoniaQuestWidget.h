// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Definitions/HarmoniaQuestSystemDefinitions.h"
#include "HarmoniaQuestWidget.generated.h"

class UHarmoniaQuestComponent;
class UHarmoniaQuestSubsystem;
class UImage;
class UTextBlock;
class UProgressBar;
class UScrollBox;
class UButton;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIQuestSelected, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIQuestAccepted, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIQuestAbandoned, FName, QuestID);

/**
 * Quest UI Widget
 * Displays quest list, quest details, objectives, and tracking
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaQuestWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UHarmoniaQuestWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// ====================================
	// Setup
	// ====================================

	/** Set the quest component to display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void SetQuestComponent(UHarmoniaQuestComponent* InComponent);

	/** Refresh the entire quest display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void RefreshDisplay();

	// ====================================
	// Quest List
	// ====================================

	/** Refresh quest list by category */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void RefreshQuestList(EQuestType QuestType);

	/** Refresh all active quests */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void RefreshActiveQuests();

	/** Filter quests by state */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void FilterByState(EQuestState State);

	// ====================================
	// Quest Details
	// ====================================

	/** Show quest details */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void ShowQuestDetails(FName QuestID, const FText& QuestName, const FText& Description,
						  EQuestType QuestType, EQuestState QuestState);

	/** Hide quest details */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void HideQuestDetails();

	/** Update quest objectives display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void UpdateObjectives(const TArray<FText>& ObjectiveTexts, const TArray<float>& ObjectiveProgress,
						  const TArray<bool>& ObjectiveCompleted);

	// ====================================
	// Quest Tracking
	// ====================================

	/** Set tracked quest for HUD display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void SetTrackedQuest(FName QuestID);

	/** Update tracked quest display */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void UpdateTrackedQuestDisplay(const FText& QuestName, const FText& CurrentObjective, float Progress);

	/** Show tracking panel */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void ShowTrackingPanel();

	/** Hide tracking panel */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void HideTrackingPanel();

	// ====================================
	// Rewards Display
	// ====================================

	/** Show quest rewards */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void ShowRewards(int32 Experience, int32 Gold, const TArray<FName>& ItemRewards);

	// ====================================
	// Notifications
	// ====================================

	/** Show quest accepted notification */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void ShowQuestAcceptedNotification(const FText& QuestName);

	/** Show quest completed notification */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void ShowQuestCompletedNotification(const FText& QuestName);

	/** Show objective updated notification */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|UI|Quest")
	void ShowObjectiveUpdatedNotification(const FText& ObjectiveText, int32 Current, int32 Required);

	// ====================================
	// Events
	// ====================================

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|UI|Quest")
	FOnUIQuestSelected OnQuestSelected;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|UI|Quest")
	FOnUIQuestAccepted OnQuestAccepted;

	UPROPERTY(BlueprintAssignable, Category = "Harmonia|UI|Quest")
	FOnUIQuestAbandoned OnQuestAbandoned;

protected:
	// ====================================
	// Quest List Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> QuestListScrollBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ActiveQuestsBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> CompletedQuestsBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harmonia|UI|Quest")
	TSubclassOf<UUserWidget> QuestEntryWidgetClass;

	// ====================================
	// Category Buttons
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> MainQuestTabButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SideQuestTabButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> DailyQuestTabButton;

	// ====================================
	// Quest Details Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> QuestDetailsPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuestNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuestDescriptionText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuestTypeText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuestStateText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ObjectivesBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> AcceptQuestButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> AbandonQuestButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> TrackQuestButton;

	// ====================================
	// Rewards Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> RewardsPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ExperienceRewardText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GoldRewardText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ItemRewardsBox;

	// ====================================
	// Tracking Widgets (HUD)
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> TrackingPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TrackedQuestNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TrackedObjectiveText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> TrackedProgressBar;

	// ====================================
	// Notification Widgets
	// ====================================

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> NotificationPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NotificationText;

	// ====================================
	// References
	// ====================================

	UPROPERTY(BlueprintReadOnly, Category = "Harmonia|UI|Quest")
	TObjectPtr<UHarmoniaQuestComponent> QuestComponent;

	// ====================================
	// State
	// ====================================

	UPROPERTY()
	FName CurrentSelectedQuest;

	UPROPERTY()
	FName CurrentTrackedQuest;

	UPROPERTY()
	EQuestType CurrentQuestTypeFilter = EQuestType::Main;

private:
	FText GetQuestTypeText(EQuestType Type) const;
	FText GetQuestStateText(EQuestState State) const;
	FLinearColor GetQuestTypeColor(EQuestType Type) const;
	FLinearColor GetQuestStateColor(EQuestState State) const;

	void HideNotificationAfterDelay();
	FTimerHandle NotificationTimerHandle;
};
