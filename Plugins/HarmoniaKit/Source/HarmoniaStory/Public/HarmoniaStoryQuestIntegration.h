// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Definitions/HarmoniaQuestSystemDefinitions.h"
#include "HarmoniaStoryQuestIntegration.generated.h"

class UHarmoniaQuestComponent;

/**
 * Subsystem to integrate Storytelling features with Quest System
 */
UCLASS()
class HARMONIASTORY_API UHarmoniaStoryQuestIntegration : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	//~ Begin FTickableGameObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	//~ End FTickableGameObject Interface

	/** Bind to a quest component (e.g. local player's) */
	UFUNCTION(BlueprintCallable, Category = "Story|Integration")
	void BindToQuestComponent(UHarmoniaQuestComponent* QuestComponent);

protected:
	UPROPERTY()
	UHarmoniaQuestComponent* BoundQuestComponent;

	UFUNCTION()
	void OnQuestStarted(FHarmoniaID QuestId, const FHarmoniaQuestData& QuestData);

	UFUNCTION()
	void OnQuestCompleted(FHarmoniaID QuestId, const FHarmoniaQuestData& QuestData, const TArray<FQuestReward>& Rewards);

	UFUNCTION()
	void OnQuestEventTriggered(FHarmoniaID QuestId, const FQuestEvent& Event);
};
