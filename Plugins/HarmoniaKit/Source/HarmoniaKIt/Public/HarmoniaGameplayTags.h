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

	// ============================================================
	// Damage System Tags
	// ============================================================

	// Damage Types
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Type_Physical);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Type_Fire);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Type_Ice);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Type_Lightning);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Type_Poison);

	// Damage Modifiers
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Critical);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Blocked);

	// Hit Reactions
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReaction_Light);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReaction_Medium);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReaction_Heavy);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReaction_Knockback);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(HitReaction_Stun);

	// Gameplay Cues - Hit Effects
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Hit_Physical);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Hit_Critical);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Hit_Blocked);

	// Data Tags (for SetByCaller magnitudes)
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Healing);
}
