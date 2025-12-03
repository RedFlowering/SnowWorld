// Copyright 2025 Snow Game Studio.

#include "HarmoniaStoryQuestIntegration.h"
#include "Core/HarmoniaCoreBFL.h"
#include "Components/HarmoniaQuestComponent.h"
#include "Components/HarmoniaDialogueComponent.h"
#include "Cutscene/HarmoniaCutsceneManager.h"
#include "Journal/HarmoniaJournalSubsystem.h"
#include "Dialogue/HarmoniaDialogueTree.h"
#include "LevelSequence.h"
#include "Kismet/GameplayStatics.h"
#include "HarmoniaStoryLog.h"

void UHarmoniaStoryQuestIntegration::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHarmoniaStoryQuestIntegration::Deinitialize()
{
	if (BoundQuestComponent)
	{
		BoundQuestComponent->OnQuestStarted.RemoveDynamic(this, &UHarmoniaStoryQuestIntegration::OnQuestStarted);
		BoundQuestComponent->OnQuestCompleted.RemoveDynamic(this, &UHarmoniaStoryQuestIntegration::OnQuestCompleted);
		BoundQuestComponent->OnQuestEventTriggered.RemoveDynamic(this, &UHarmoniaStoryQuestIntegration::OnQuestEventTriggered);
	}
	Super::Deinitialize();
}

void UHarmoniaStoryQuestIntegration::Tick(float DeltaTime)
{
	// Auto-bind if not bound or bound component is invalid
	if (!BoundQuestComponent || !BoundQuestComponent->IsValidLowLevel())
	{
		UWorld* World = GetWorld();
		if (World)
		{
			APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
			if (PC && PC->GetPawn())
			{
				UHarmoniaQuestComponent* QuestComp = PC->GetPawn()->FindComponentByClass<UHarmoniaQuestComponent>();
				if (QuestComp && QuestComp != BoundQuestComponent)
				{
					BindToQuestComponent(QuestComp);
				}
			}
		}
	}
}

bool UHarmoniaStoryQuestIntegration::IsTickable() const
{
	return !IsTemplate();
}

TStatId UHarmoniaStoryQuestIntegration::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHarmoniaStoryQuestIntegration, STATGROUP_Tickables);
}

void UHarmoniaStoryQuestIntegration::BindToQuestComponent(UHarmoniaQuestComponent* QuestComponent)
{
	if (!QuestComponent)
	{
		return;
	}

	if (BoundQuestComponent)
	{
		// Unbind previous
		BoundQuestComponent->OnQuestStarted.RemoveDynamic(this, &UHarmoniaStoryQuestIntegration::OnQuestStarted);
		BoundQuestComponent->OnQuestCompleted.RemoveDynamic(this, &UHarmoniaStoryQuestIntegration::OnQuestCompleted);
		BoundQuestComponent->OnQuestEventTriggered.RemoveDynamic(this, &UHarmoniaStoryQuestIntegration::OnQuestEventTriggered);
	}

	BoundQuestComponent = QuestComponent;

	BoundQuestComponent->OnQuestStarted.AddDynamic(this, &UHarmoniaStoryQuestIntegration::OnQuestStarted);
	BoundQuestComponent->OnQuestCompleted.AddDynamic(this, &UHarmoniaStoryQuestIntegration::OnQuestCompleted);
	BoundQuestComponent->OnQuestEventTriggered.AddDynamic(this, &UHarmoniaStoryQuestIntegration::OnQuestEventTriggered);

	UE_LOG(LogHarmoniaStory, Log, TEXT("Story Integration bound to Quest Component"));
}

void UHarmoniaStoryQuestIntegration::OnQuestStarted(FHarmoniaID QuestId, const FHarmoniaQuestData& QuestData)
{
	// Check for start dialogue
	if (!QuestData.Dialogues.StartDialogueTree.IsNull())
	{
		UHarmoniaDialogueTree* DialogueTree = Cast<UHarmoniaDialogueTree>(QuestData.Dialogues.StartDialogueTree.LoadSynchronous());
		if (DialogueTree)
		{
			// Find Dialogue Component on Player
			APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			if (PC && PC->GetPawn())
			{
				UHarmoniaDialogueComponent* DialogueComp = PC->GetPawn()->FindComponentByClass<UHarmoniaDialogueComponent>();
				if (DialogueComp)
				{
					DialogueComp->StartDialogue(DialogueTree);
					UE_LOG(LogHarmoniaStory, Log, TEXT("Started quest dialogue for %s"), *QuestId.ToString());
				}
				else
				{
					UE_LOG(LogHarmoniaStory, Warning, TEXT("Player has no Dialogue Component"));
				}
			}
		}
	}
}

void UHarmoniaStoryQuestIntegration::OnQuestCompleted(FHarmoniaID QuestId, const FHarmoniaQuestData& QuestData, const TArray<FQuestReward>& Rewards)
{
	// Check for Journal rewards
	UHarmoniaJournalSubsystem* JournalSubsystem = UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaJournalSubsystem>(this);
	if (JournalSubsystem)
	{
		for (const FQuestReward& Reward : Rewards)
		{
			if (Reward.JournalEntryToUnlock.IsValid())
			{
				JournalSubsystem->UnlockEntry(Reward.JournalEntryToUnlock);
			}
		}
	}
}

void UHarmoniaStoryQuestIntegration::OnQuestEventTriggered(FHarmoniaID QuestId, const FQuestEvent& Event)
{
	if (Event.EventType == EQuestEventType::PlayCutscene)
	{
		if (!Event.CutsceneToPlay.IsNull())
		{
			ULevelSequence* Sequence = Event.CutsceneToPlay.LoadSynchronous();
			if (Sequence)
			{
				UHarmoniaCutsceneManager* CutsceneManager = UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaCutsceneManager>(this);
				if (CutsceneManager)
				{
					FCutsceneSettings Settings;
					// Could expose settings in FQuestEvent if needed, using defaults for now
					CutsceneManager->PlayCutscene(Sequence, Settings);
					UE_LOG(LogHarmoniaStory, Log, TEXT("Playing quest cutscene for %s"), *QuestId.ToString());
				}
			}
		}
	}
}
