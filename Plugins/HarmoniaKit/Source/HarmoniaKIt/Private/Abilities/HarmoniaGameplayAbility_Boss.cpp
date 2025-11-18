// Copyright 2024 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Boss.h"
#include "Monsters/HarmoniaBossMonster.h"
#include "Character/LyraHealthComponent.h"
#include "AbilitySystemComponent.h"

UHarmoniaGameplayAbility_Boss::UHarmoniaGameplayAbility_Boss(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MinHealthPercent = 0.0f;
	MaxHealthPercent = 0.0f;
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

bool UHarmoniaGameplayAbility_Boss::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	AHarmoniaBossMonster* Boss = Cast<AHarmoniaBossMonster>(ActorInfo->AvatarActor.Get());
	if (!Boss)
	{
		return true; // Allow if not a boss
	}

	// Check phase restriction
	if (ValidPhases.Num() > 0)
	{
		int32 CurrentPhase = Boss->GetCurrentPhase();
		if (!ValidPhases.Contains(CurrentPhase))
		{
			return false;
		}
	}

	// Check health restrictions
	if (MinHealthPercent > 0.0f || MaxHealthPercent > 0.0f)
	{
		ULyraHealthComponent* HealthComp = ULyraHealthComponent::FindHealthComponent(Boss);
		if (HealthComp)
		{
			float HealthPercent = HealthComp->GetHealthNormalized();

			if (MinHealthPercent > 0.0f && HealthPercent < MinHealthPercent)
			{
				return false;
			}

			if (MaxHealthPercent > 0.0f && HealthPercent > MaxHealthPercent)
			{
				return false;
			}
		}
	}

	return true;
}
