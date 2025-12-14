// Copyright 2025 Snow Game Studio.

#include "Player/HarmoniaPlayerState.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaPlayerState)

AHarmoniaPlayerState::AHarmoniaPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("HealthSet"))
	                        .DoNotCreateDefaultSubobject(TEXT("CombatSet")))
{
	// DoNotCreateDefaultSubobject may be ignored if marked as required, but that's OK
	// HarmoniaAttributeSet will be added via AbilitySet and that's the one we use
}

const UHarmoniaAttributeSet* AHarmoniaPlayerState::GetHarmoniaAttributeSet() const
{
	if (ULyraAbilitySystemComponent* ASC = GetLyraAbilitySystemComponent())
	{
		return ASC->GetSet<UHarmoniaAttributeSet>();
	}
	return nullptr;
}
