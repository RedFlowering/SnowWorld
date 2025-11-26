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

	// Ability Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_LockOn);

	// State Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_LockOn_Active);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_LockOn_HasTarget);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_LockedOn);

	// Character Type Tags (for target filtering)
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Type_Enemy);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Type_Ally);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Type_Neutral);

	// ============================================================
	// Dodge System Tags (Already implemented)
	// ============================================================

	// Input Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Dodge);

	// Ability Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Dodge);

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

	// ============================================================
	// Animation System Tags
	// ============================================================

	// Animation Root Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Monster);

	// Player Animation Examples (Attack)
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_Attack);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_Attack_Sword);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_Attack_Sword_Light);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_Attack_Sword_Heavy);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_Attack_Sword_Combo1);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_Attack_Sword_Combo2);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_Attack_Sword_Combo3);

	// Player Animation Examples (Dodge)
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_Dodge);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_Dodge_Forward);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_Dodge_Backward);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_Dodge_Left);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_Dodge_Right);

	// Player Animation Examples (HitReaction)
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_HitReaction);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_HitReaction_Light);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Player_HitReaction_Heavy);

	// Monster Animation Examples
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Monster_Goblin);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Monster_Goblin_Attack);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Anim_Monster_Goblin_Death);

	// ============================================================
	// Sound System Tags
	// ============================================================

	// Sound Root Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_SFX);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_Music);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_Voice);

	// SFX Examples (Hit)
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_SFX_Hit);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_SFX_Hit_Metal);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_SFX_Hit_Wood);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_SFX_Hit_Stone);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_SFX_Hit_Flesh);

	// SFX Examples (Weapon)
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_SFX_Weapon);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_SFX_Weapon_Sword);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_SFX_Weapon_Sword_Swing);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_SFX_Weapon_Sword_Hit);

	// SFX Examples (Footstep)
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_SFX_Footstep);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_SFX_Footstep_Stone);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_SFX_Footstep_Wood);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_SFX_Footstep_Grass);

	// Music Examples
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_Music_Combat);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_Music_Combat_Intense);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_Music_Ambient);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_Music_Ambient_Peaceful);

	// Voice Examples
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_Voice_Player);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_Voice_Player_Grunt);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sound_Voice_Player_Death);

	// ============================================================
	// Mount System Tags
	// ============================================================

	// Input Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Mount);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Dismount);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_MountSpecialAbility);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_MountSprint);

	// State Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Mounted);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Mounting);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dismounting);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Mount_Sprinting);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Mount_Flying);

	// Mount Type Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mount_Type_Ground);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mount_Type_Flying);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mount_Type_Amphibious);

	// Event Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Mount_Mounted);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Mount_Dismounted);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Mount_ChargeAttack);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Mount_Trample);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Mount_AerialStrike);

	// ============================================================
	// Fast Travel System Tags
	// ============================================================

	// Input Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_FastTravel);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_OpenFastTravelMap);

	// State Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_FastTraveling);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_CanFastTravel);

	// Waypoint Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Waypoint_Discovered);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Waypoint_Active);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Waypoint_Locked);

	// Event Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Waypoint_Discovered);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_FastTravel_Started);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_FastTravel_Completed);

	// ============================================================
	// Climbing and Parkour System Tags
	// ============================================================

	// Input Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Climb);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Vault);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Hang);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Mantle);

	// State Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Climbing);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Hanging);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_WallRunning);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Vaulting);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Mantling);

	// Surface Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Surface_Climbable);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Surface_Climbable_Rough);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Surface_Climbable_Smooth);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Surface_Climbable_Ledge);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Surface_Climbable_Ladder);

	// Event Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Climbing_Started);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Climbing_Stopped);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Parkour_Vault);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Parkour_WallJump);

	// ============================================================
	// Swimming and Diving System Tags
	// ============================================================

	// Input Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Dive);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Surface);

	// State Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Swimming);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Diving);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Underwater);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_LowOxygen);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Drowning);

	// Water Environment Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Water_Environment_Fresh);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Water_Environment_Salt);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Water_Environment_Toxic);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Water_Environment_Lava);

	// Event Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Swimming_Started);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Swimming_Stopped);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Diving_Started);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Oxygen_Depleted);

	// ============================================================
	// General Movement System Tags
	// ============================================================

	// Ability Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Movement_Sprint);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Movement_Dash);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Movement_Slide);

	// Movement Restriction Tags
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Restricted_NoMount);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Restricted_NoClimb);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Restricted_NoSwim);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Restricted_NoFastTravel);

	// ============================================================
	// Stat System Tags (for SetByCaller and Stat Allocation)
	// ============================================================

	// Primary Stats
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Primary_Vitality);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Primary_Endurance);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Primary_Strength);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Primary_Dexterity);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Primary_Intelligence);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Primary_Faith);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Primary_Luck);

	// Combat Stats
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Combat_AttackPower);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Combat_Defense);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Combat_CriticalChance);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Combat_CriticalDamage);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Combat_Poise);

	// Resource Stats
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Resource_MaxHealth);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Resource_MaxStamina);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Resource_MaxMana);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Resource_StaminaRegen);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Resource_ManaRegen);

	// Movement Stats
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Movement_Speed);
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Movement_AttackSpeed);

	// Equipment Stats
	HARMONIAKIT_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stat_Equipment_MaxLoad);
}
