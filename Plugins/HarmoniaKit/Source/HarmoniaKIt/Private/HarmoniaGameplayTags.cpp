// Copyright 2025 Snow Game Studio.

#include "HarmoniaGameplayTags.h"
#include "GameplayTagsManager.h"
#include "Engine/EngineTypes.h"

namespace HarmoniaGameplayTags
{
	// ============================================================
	// Input Tags
	// ============================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Attack_Melee, "InputTag.Attack.Melee", "Melee attack input tag");

	// ============================================================
	// Ability Tags - Combo States
	// ============================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Attack_Melee_Combo1, "Ability.Attack.Melee.Combo1", "First combo attack");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Attack_Melee_Combo2, "Ability.Attack.Melee.Combo2", "Second combo attack");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Attack_Melee_Combo3, "Ability.Attack.Melee.Combo3", "Third combo attack");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Attack_Melee_Combo4, "Ability.Attack.Melee.Combo4", "Fourth combo attack");

	// ============================================================
	// State Tags
	// ============================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_Attacking, "State.Combat.Attacking", "Character is performing an attack");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Combat_ComboWindow, "State.Combat.ComboWindow", "Combo window is active");

	// ============================================================
	// Event Tags
	// ============================================================
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Attack_ComboNext, "GameplayEvent.Attack.ComboNext", "Trigger next combo in sequence");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Attack_ComboReset, "GameplayEvent.Attack.ComboReset", "Reset combo sequence");

	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					UE_LOG(LogTemp, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}
}
