// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HarmoniaCutsceneManager.generated.h"

class ULevelSequence;
class ULevelSequencePlayer;
class ALevelSequenceActor;

/**
 * Settings for cutscene playback
 */
USTRUCT(BlueprintType)
struct FCutsceneSettings
{
	GENERATED_BODY()

	/** Whether to hide the HUD during playback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutscene")
	bool bHideHUD = true;

	/** Whether to disable player input during playback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutscene")
	bool bDisableInput = true;

	/** Whether the cutscene can be skipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutscene")
	bool bCanSkip = true;

	FCutsceneSettings()
		: bHideHUD(true)
		, bDisableInput(true)
		, bCanSkip(true)
	{}
};

/**
 * Delegate for cutscene events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCutsceneStarted, ULevelSequence*, Sequence);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCutsceneEnded, ULevelSequence*, Sequence);

/**
 * Subsystem to manage cinematic playback
 */
UCLASS()
class HARMONIASTORY_API UHarmoniaCutsceneManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Play a level sequence as a cutscene */
	UFUNCTION(BlueprintCallable, Category = "Cutscene")
	void PlayCutscene(ULevelSequence* Sequence, FCutsceneSettings Settings);

	/** Stop the currently playing cutscene */
	UFUNCTION(BlueprintCallable, Category = "Cutscene")
	void StopCutscene();

	/** Skip the currently playing cutscene */
	UFUNCTION(BlueprintCallable, Category = "Cutscene")
	void SkipCutscene();

	/** Check if a cutscene is currently playing */
	UFUNCTION(BlueprintPure, Category = "Cutscene")
	bool IsCutscenePlaying() const { return CurrentSequenceActor != nullptr; }

public:
	UPROPERTY(BlueprintAssignable, Category = "Cutscene|Events")
	FOnCutsceneStarted OnCutsceneStarted;

	UPROPERTY(BlueprintAssignable, Category = "Cutscene|Events")
	FOnCutsceneEnded OnCutsceneEnded;

protected:
	UPROPERTY()
	ALevelSequenceActor* CurrentSequenceActor;

	UPROPERTY()
	ULevelSequencePlayer* CurrentSequencePlayer;

	FCutsceneSettings CurrentSettings;

	UFUNCTION()
	void OnSequenceFinished();

	void SetPlayerInputEnabled(bool bEnabled);
	void SetHUDVisible(bool bVisible);
};
