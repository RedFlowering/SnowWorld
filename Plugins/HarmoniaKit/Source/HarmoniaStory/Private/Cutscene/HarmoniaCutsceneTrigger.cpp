// Copyright 2025 Snow Game Studio.

#include "Cutscene/HarmoniaCutsceneTrigger.h"
#include "Core/HarmoniaCoreBFL.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AHarmoniaCutsceneTrigger::AHarmoniaCutsceneTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;
	
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}

void AHarmoniaCutsceneTrigger::BeginPlay()
{
	Super::BeginPlay();
	
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AHarmoniaCutsceneTrigger::OnOverlapBegin);
}

void AHarmoniaCutsceneTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasTriggered && bTriggerOnce)
	{
		return;
	}

	// Check if it's a player character
	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()) && OtherActor->GetInstigatorController() && OtherActor->GetInstigatorController()->IsPlayerController())
	{
		UHarmoniaCutsceneManager* CutsceneManager = UHarmoniaCoreBFL::GetGameInstanceSubsystem<UHarmoniaCutsceneManager>(this);
		if (CutsceneManager && CutsceneSequence)
		{
			CutsceneManager->PlayCutscene(CutsceneSequence, Settings);
			bHasTriggered = true;
		}
	}
}
