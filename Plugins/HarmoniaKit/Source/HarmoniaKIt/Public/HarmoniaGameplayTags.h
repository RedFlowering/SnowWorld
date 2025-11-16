// Copyright 2025 Snow Game Studio.

#pragma once

#include "NativeGameplayTags.h"

namespace HarmoniaGameplayTags
{
	HARMONIAKIT_API FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

	// ============================================================
	// Combo Attack System Tags
	// ============================================================

	// Input Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Attack_Melee);

	// Ability Tags - Combo States
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Melee_Combo1);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Melee_Combo2);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Melee_Combo3);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Melee_Combo4);

	// State Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_Attacking);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_ComboWindow);

	// Event Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Attack_ComboNext);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Attack_ComboReset);
}
