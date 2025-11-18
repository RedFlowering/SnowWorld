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

	// ============================================================
	// Hit Reaction System Tags
	// ============================================================

	// State Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_HitReaction);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_HitStunned);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Invincible);

	// Event Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_HitReaction);

	// ============================================================
	// Lock-On System Tags
	// ============================================================

	// Input Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_LockOn);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_SwitchTargetLeft);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_SwitchTargetRight);

	// State Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_LockOn_Active);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_LockOn_HasTarget);

	// Character Type Tags (for target filtering)
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Type_Enemy);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Type_Ally);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Type_Neutral);

	// ============================================================
	// Dodge System Tags (Already implemented)
	// ============================================================

	// Input Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Dodge);

	// State Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dodging);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Dodging);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Attacking);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Blocking);

	// ============================================================
	// Difficulty System Tags
	// ============================================================

	// Difficulty Pact Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Difficulty_Pact_GlassCannon);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Difficulty_Pact_IronWill);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Difficulty_Pact_SwiftFoes);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Difficulty_Pact_EliteUprising);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Difficulty_Pact_Permadeath);

	// New Game Plus Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Difficulty_NewGamePlus);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Difficulty_NewGamePlus_Active);

	// Dynamic Difficulty Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Difficulty_Dynamic_Enabled);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Difficulty_Dynamic_Adjusting);

	// Event Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Difficulty_PactActivated);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Difficulty_PactDeactivated);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Difficulty_DynamicAdjusted);

	// ============================================================
	// Animation Effect System Tags
	// ============================================================

	// Effect Root Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Hit);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Trail);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Impact);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Projectile);

	// Hit Effect Examples (Slash)
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Hit_Slash);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Hit_Slash_Light);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Hit_Slash_Medium);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Hit_Slash_Heavy);

	// Hit Effect Examples (Blunt)
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Hit_Blunt);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Hit_Blunt_Light);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Hit_Blunt_Heavy);

	// Hit Effect Examples (Pierce)
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Hit_Pierce);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Hit_Pierce_Light);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Hit_Pierce_Heavy);

	// Weapon Trail Examples
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Trail_Sword);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Trail_Sword_Normal);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Trail_Sword_Fire);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Trail_Sword_Ice);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Trail_Axe);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Trail_Axe_Heavy);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Trail_Dagger);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Trail_Dagger_Fast);

	// Impact Effect Examples
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Impact_Metal);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Impact_Wood);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Impact_Stone);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Impact_Flesh);
}
