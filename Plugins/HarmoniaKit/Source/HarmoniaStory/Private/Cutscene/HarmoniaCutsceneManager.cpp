// Copyright 2025 Snow Game Studio.

#include "Cutscene/HarmoniaCutsceneManager.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "HarmoniaStoryLog.h"

void UHarmoniaCutsceneManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHarmoniaCutsceneManager::Deinitialize()
{
	Super::Deinitialize();
}

void UHarmoniaCutsceneManager::PlayCutscene(ULevelSequence* Sequence, FCutsceneSettings Settings)
{
	if (!Sequence)
	{
		UE_LOG(LogHarmoniaStory, Warning, TEXT("PlayCutscene called with null sequence"));
		return;
	}

	if (IsCutscenePlaying())
	{
		UE_LOG(LogHarmoniaStory, Warning, TEXT("Cutscene already playing, stopping current one"));
		StopCutscene();
	}

	CurrentSettings = Settings;

	FMovieSceneSequencePlaybackSettings PlaybackSettings;
	PlaybackSettings.bAutoPlay = true;
	PlaybackSettings.bHideHud = Settings.bHideHUD;
	PlaybackSettings.bDisableCameraCuts = false;
	PlaybackSettings.bPauseAtEnd = false;

	CurrentSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
		GetWorld(), Sequence, PlaybackSettings, CurrentSequenceActor);

	if (CurrentSequencePlayer)
	{
		CurrentSequencePlayer->OnFinished.AddDynamic(this, &UHarmoniaCutsceneManager::OnSequenceFinished);
		CurrentSequencePlayer->Play();

		if (Settings.bDisableInput)
		{
			SetPlayerInputEnabled(false);
		}

		// HUD hiding is handled by PlaybackSettings.bHideHud usually, but we can enforce it if needed
		// For now relying on Sequence Player settings

		OnCutsceneStarted.Broadcast(Sequence);
	}
}

void UHarmoniaCutsceneManager::StopCutscene()
{
	if (CurrentSequencePlayer)
	{
		CurrentSequencePlayer->Stop();
		OnSequenceFinished(); // Force cleanup
	}
}

void UHarmoniaCutsceneManager::SkipCutscene()
{
	if (IsCutscenePlaying() && CurrentSettings.bCanSkip)
	{
		StopCutscene();
	}
}

void UHarmoniaCutsceneManager::OnSequenceFinished()
{
	ULevelSequence* FinishedSequence = nullptr;
	if (CurrentSequencePlayer)
	{
		FinishedSequence = Cast<ULevelSequence>(CurrentSequencePlayer->GetSequence());
		CurrentSequencePlayer->OnFinished.RemoveDynamic(this, &UHarmoniaCutsceneManager::OnSequenceFinished);
	}

	CurrentSequencePlayer = nullptr;
	CurrentSequenceActor = nullptr;

	if (CurrentSettings.bDisableInput)
	{
		SetPlayerInputEnabled(true);
	}

	OnCutsceneEnded.Broadcast(FinishedSequence);
}

void UHarmoniaCutsceneManager::SetPlayerInputEnabled(bool bEnabled)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		if (bEnabled)
		{
			PC->EnableInput(PC);
		}
		else
		{
			PC->DisableInput(PC);
		}
	}
}

void UHarmoniaCutsceneManager::SetHUDVisible(bool bVisible)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->GetHUD())
	{
		PC->GetHUD()->SetActorHiddenInGame(!bVisible);
	}
}
