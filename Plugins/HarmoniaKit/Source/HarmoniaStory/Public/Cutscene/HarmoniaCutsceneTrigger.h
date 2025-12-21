// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cutscene/HarmoniaCutsceneManager.h"
#include "HarmoniaCutsceneTrigger.generated.h"

class UBoxComponent;
class ULevelSequence;

/**
 * Actor that triggers a cutscene when overlapped
 */
UCLASS()
class HARMONIASTORY_API AHarmoniaCutsceneTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	AHarmoniaCutsceneTrigger();

protected:
	virtual void BeginPlay() override;

public:
	/** Trigger volume */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cutscene")
	UBoxComponent* TriggerBox;

	/** Sequence to play */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutscene")
	ULevelSequence* CutsceneSequence;

	/** Cutscene settings */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutscene")
	FCutsceneSettings Settings;

	/** Whether to trigger only once */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutscene")
	bool bTriggerOnce = true;

protected:
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool bHasTriggered = false;
};
