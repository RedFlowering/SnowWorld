// Copyright 2025 Snow Game Studio.

#include "AI/HarmoniaJumpPoint.h"
#include "SenseStimulusComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/ArrowComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

AHarmoniaJumpPoint::AHarmoniaJumpPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// SenseSystem Stimulus (Sense 탐지용)
	StimulusComponent = CreateDefaultSubobject<USenseStimulusComponent>(TEXT("StimulusComponent"));

#if WITH_EDITORONLY_DATA
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		SpriteComponent->SetupAttachment(RootComponent);
		SpriteComponent->SetRelativeScale3D(FVector(0.5f));
	}

	LandingArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("LandingArrow"));
	if (LandingArrow)
	{
		LandingArrow->SetupAttachment(RootComponent);
		LandingArrow->SetArrowColor(FLinearColor::Green);
		LandingArrow->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	}
#endif
}

#if WITH_EDITOR
void AHarmoniaJumpPoint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (LandingArrow)
	{
		LandingArrow->SetRelativeLocation(LandingOffset);
	}
}
#endif

bool AHarmoniaJumpPoint::IsValidForActor(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	// 이미 점유 중이면 불가
	if (bIsOccupied && OccupyingActor.IsValid() && OccupyingActor.Get() != Actor)
	{
		return false;
	}

	// 태그 검사
	if (!AllowedTags.IsEmpty())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
		{
			if (!ASC->HasAnyMatchingGameplayTags(AllowedTags))
			{
				return false;
			}
		}
	}

	return true;
}

bool AHarmoniaJumpPoint::ClaimPoint(AActor* Claimer)
{
	if (!Claimer)
	{
		return false;
	}

	if (bIsOccupied && OccupyingActor.IsValid() && OccupyingActor.Get() != Claimer)
	{
		return false;
	}

	bIsOccupied = true;
	OccupyingActor = Claimer;
	return true;
}

void AHarmoniaJumpPoint::ReleasePoint(AActor* Claimer)
{
	if (OccupyingActor.Get() == Claimer)
	{
		bIsOccupied = false;
		OccupyingActor.Reset();
	}
}
