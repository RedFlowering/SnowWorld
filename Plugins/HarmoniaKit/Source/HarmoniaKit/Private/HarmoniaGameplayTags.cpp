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

	// ============================================================
	// Damage System Tags
	// ============================================================

	// Damage Types
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Type_Physical, "Damage.Type.Physical", "Physical damage type");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Type_Fire, "Damage.Type.Fire", "Fire damage type");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Type_Ice, "Damage.Type.Ice", "Ice damage type");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Type_Lightning, "Damage.Type.Lightning", "Lightning damage type");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Type_Poison, "Damage.Type.Poison", "Poison damage type");

	// Damage Modifiers
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Critical, "Damage.Critical", "Critical hit damage");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Blocked, "Damage.Blocked", "Damage was blocked");

	// Hit Reactions
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReaction_Light, "HitReaction.Light", "Light hit reaction");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReaction_Medium, "HitReaction.Medium", "Medium hit reaction");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReaction_Heavy, "HitReaction.Heavy", "Heavy hit reaction");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReaction_Knockback, "HitReaction.Knockback", "Knockback hit reaction");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(HitReaction_Stun, "HitReaction.Stun", "Stun hit reaction");

	// Gameplay Cues - Hit Effects
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_Hit_Physical, "GameplayCue.Hit.Physical", "Physical hit effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_Hit_Critical, "GameplayCue.Hit.Critical", "Critical hit effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_Hit_Blocked, "GameplayCue.Hit.Blocked", "Blocked hit effect");

	// Data Tags (for SetByCaller magnitudes)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Damage, "Data.Damage", "Damage magnitude for SetByCaller");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Healing, "Data.Healing", "Healing magnitude for SetByCaller");

	// ============================================================
	// Hit Reaction System Tags
	// ============================================================

	// State Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_HitReaction, "State.HitReaction", "Character is in hit reaction");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_HitStunned, "State.HitStunned", "Character is hit stunned");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Invincible, "State.Invincible", "Character is invincible");

	// Event Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_HitReaction, "GameplayEvent.HitReaction", "Trigger hit reaction");

	// ============================================================
	// Lock-On System Tags
	// ============================================================

	// Input Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_LockOn, "InputTag.LockOn", "Lock-on toggle input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_SwitchTargetLeft, "InputTag.SwitchTargetLeft", "Switch lock-on target left");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_SwitchTargetRight, "InputTag.SwitchTargetRight", "Switch lock-on target right");

	// State Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_LockOn_Active, "State.LockOn.Active", "Lock-on is currently active");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_LockOn_HasTarget, "State.LockOn.HasTarget", "Lock-on has a valid target");

	// Character Type Tags (for target filtering)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Type_Enemy, "Character.Type.Enemy", "Character is an enemy");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Type_Ally, "Character.Type.Ally", "Character is an ally");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Type_Neutral, "Character.Type.Neutral", "Character is neutral");

	// ============================================================
	// Dodge System Tags (Already implemented)
	// ============================================================

	// Input Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Dodge, "InputTag.Dodge", "Dodge input");

	// State Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dodging, "State.Dodging", "Character is dodging");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Dodging, "Character.State.Dodging", "Character state dodging");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Attacking, "Character.State.Attacking", "Character state attacking");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_State_Blocking, "Character.State.Blocking", "Character state blocking");

	// ============================================================
	// Difficulty System Tags
	// ============================================================

	// Difficulty Pact Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Difficulty_Pact_GlassCannon, "Difficulty.Pact.GlassCannon", "Glass Cannon difficulty pact active");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Difficulty_Pact_IronWill, "Difficulty.Pact.IronWill", "Iron Will difficulty pact active");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Difficulty_Pact_SwiftFoes, "Difficulty.Pact.SwiftFoes", "Swift Foes difficulty pact active");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Difficulty_Pact_EliteUprising, "Difficulty.Pact.EliteUprising", "Elite Uprising difficulty pact active");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Difficulty_Pact_Permadeath, "Difficulty.Pact.Permadeath", "Permadeath difficulty pact active");

	// New Game Plus Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Difficulty_NewGamePlus, "Difficulty.NewGamePlus", "New Game Plus unlocked");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Difficulty_NewGamePlus_Active, "Difficulty.NewGamePlus.Active", "Currently in New Game Plus");

	// Dynamic Difficulty Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Difficulty_Dynamic_Enabled, "Difficulty.Dynamic.Enabled", "Dynamic difficulty is enabled");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Difficulty_Dynamic_Adjusting, "Difficulty.Dynamic.Adjusting", "Dynamic difficulty is adjusting");

	// Event Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Difficulty_PactActivated, "GameplayEvent.Difficulty.PactActivated", "Difficulty pact was activated");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Difficulty_PactDeactivated, "GameplayEvent.Difficulty.PactDeactivated", "Difficulty pact was deactivated");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Difficulty_DynamicAdjusted, "GameplayEvent.Difficulty.DynamicAdjusted", "Dynamic difficulty was adjusted");

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
