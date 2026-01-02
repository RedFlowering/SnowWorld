// Copyright 2024 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Boss.h"
#include "Monsters/HarmoniaBossMonster.h"

UHarmoniaGameplayAbility_Boss::UHarmoniaGameplayAbility_Boss(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

AHarmoniaBossMonster* UHarmoniaGameplayAbility_Boss::GetBossCharacter() const
{
	return Cast<AHarmoniaBossMonster>(GetAvatarActorFromActorInfo());
}

int32 UHarmoniaGameplayAbility_Boss::GetBossPhase() const
{
	AHarmoniaBossMonster* Boss = GetBossCharacter();
	return Boss ? Boss->GetCurrentPhase() : 0;
}

