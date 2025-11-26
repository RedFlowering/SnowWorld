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

	// Ability Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_LockOn, "Ability.LockOn", "Lock-on targeting ability");

	// State Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_LockOn_Active, "State.LockOn.Active", "Lock-on is currently active");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_LockOn_HasTarget, "State.LockOn.HasTarget", "Lock-on has a valid target");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_LockedOn, "Status.LockedOn", "Character is locked on to a target");

	// Character Type Tags (for target filtering)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Type_Enemy, "Character.Type.Enemy", "Character is an enemy");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Type_Ally, "Character.Type.Ally", "Character is an ally");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Character_Type_Neutral, "Character.Type.Neutral", "Character is neutral");

	// ============================================================
	// Dodge System Tags (Already implemented)
	// ============================================================

	// Input Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Dodge, "InputTag.Dodge", "Dodge input");

	// Ability Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Dodge, "Ability.Dodge", "Dodge roll ability");

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

	// ============================================================
	// Animation Effect System Tags
	// ============================================================

	// Effect Root Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect, "Effect", "Root tag for all animation effects");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Hit, "Effect.Hit", "Hit effects category");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Trail, "Effect.Trail", "Weapon trail effects category");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Impact, "Effect.Impact", "Impact effects category");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Projectile, "Effect.Projectile", "Projectile effects category");

	// Hit Effect Examples (Slash)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Hit_Slash, "Effect.Hit.Slash", "Slash hit effects");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Hit_Slash_Light, "Effect.Hit.Slash.Light", "Light slash hit effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Hit_Slash_Medium, "Effect.Hit.Slash.Medium", "Medium slash hit effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Hit_Slash_Heavy, "Effect.Hit.Slash.Heavy", "Heavy slash hit effect");

	// Hit Effect Examples (Blunt)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Hit_Blunt, "Effect.Hit.Blunt", "Blunt hit effects");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Hit_Blunt_Light, "Effect.Hit.Blunt.Light", "Light blunt hit effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Hit_Blunt_Heavy, "Effect.Hit.Blunt.Heavy", "Heavy blunt hit effect");

	// Hit Effect Examples (Pierce)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Hit_Pierce, "Effect.Hit.Pierce", "Pierce hit effects");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Hit_Pierce_Light, "Effect.Hit.Pierce.Light", "Light pierce hit effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Hit_Pierce_Heavy, "Effect.Hit.Pierce.Heavy", "Heavy pierce hit effect");

	// Weapon Trail Examples
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Trail_Sword, "Effect.Trail.Sword", "Sword trail effects");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Trail_Sword_Normal, "Effect.Trail.Sword.Normal", "Normal sword trail effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Trail_Sword_Fire, "Effect.Trail.Sword.Fire", "Fire sword trail effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Trail_Sword_Ice, "Effect.Trail.Sword.Ice", "Ice sword trail effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Trail_Axe, "Effect.Trail.Axe", "Axe trail effects");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Trail_Axe_Heavy, "Effect.Trail.Axe.Heavy", "Heavy axe trail effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Trail_Dagger, "Effect.Trail.Dagger", "Dagger trail effects");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Trail_Dagger_Fast, "Effect.Trail.Dagger.Fast", "Fast dagger trail effect");

	// Impact Effect Examples
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Impact_Metal, "Effect.Impact.Metal", "Metal impact effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Impact_Wood, "Effect.Impact.Wood", "Wood impact effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Impact_Stone, "Effect.Impact.Stone", "Stone impact effect");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Effect_Impact_Flesh, "Effect.Impact.Flesh", "Flesh impact effect");

	// ============================================================
	// Animation System Tags
	// ============================================================

	// Animation Root Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim, "Anim", "Root tag for all animations");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player, "Anim.Player", "Player animations");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Monster, "Anim.Monster", "Monster animations");

	// Player Animation Examples (Attack)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_Attack, "Anim.Player.Attack", "Player attack animations");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_Attack_Sword, "Anim.Player.Attack.Sword", "Player sword attack animations");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_Attack_Sword_Light, "Anim.Player.Attack.Sword.Light", "Player light sword attack");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_Attack_Sword_Heavy, "Anim.Player.Attack.Sword.Heavy", "Player heavy sword attack");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_Attack_Sword_Combo1, "Anim.Player.Attack.Sword.Combo1", "Player sword combo 1");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_Attack_Sword_Combo2, "Anim.Player.Attack.Sword.Combo2", "Player sword combo 2");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_Attack_Sword_Combo3, "Anim.Player.Attack.Sword.Combo3", "Player sword combo 3");

	// Player Animation Examples (Dodge)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_Dodge, "Anim.Player.Dodge", "Player dodge animations");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_Dodge_Forward, "Anim.Player.Dodge.Forward", "Player forward dodge");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_Dodge_Backward, "Anim.Player.Dodge.Backward", "Player backward dodge");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_Dodge_Left, "Anim.Player.Dodge.Left", "Player left dodge");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_Dodge_Right, "Anim.Player.Dodge.Right", "Player right dodge");

	// Player Animation Examples (HitReaction)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_HitReaction, "Anim.Player.HitReaction", "Player hit reaction animations");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_HitReaction_Light, "Anim.Player.HitReaction.Light", "Player light hit reaction");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Player_HitReaction_Heavy, "Anim.Player.HitReaction.Heavy", "Player heavy hit reaction");

	// Monster Animation Examples
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Monster_Goblin, "Anim.Monster.Goblin", "Goblin animations");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Monster_Goblin_Attack, "Anim.Monster.Goblin.Attack", "Goblin attack animation");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Anim_Monster_Goblin_Death, "Anim.Monster.Goblin.Death", "Goblin death animation");

	// ============================================================
	// Sound System Tags
	// ============================================================

	// Sound Root Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound, "Sound", "Root tag for all sounds");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_SFX, "Sound.SFX", "Sound effects");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_Music, "Sound.Music", "Music");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_Voice, "Sound.Voice", "Voice/dialogue");

	// SFX Examples (Hit)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_SFX_Hit, "Sound.SFX.Hit", "Hit sound effects");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_SFX_Hit_Metal, "Sound.SFX.Hit.Metal", "Metal hit sound");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_SFX_Hit_Wood, "Sound.SFX.Hit.Wood", "Wood hit sound");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_SFX_Hit_Stone, "Sound.SFX.Hit.Stone", "Stone hit sound");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_SFX_Hit_Flesh, "Sound.SFX.Hit.Flesh", "Flesh hit sound");

	// SFX Examples (Weapon)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_SFX_Weapon, "Sound.SFX.Weapon", "Weapon sound effects");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_SFX_Weapon_Sword, "Sound.SFX.Weapon.Sword", "Sword sound effects");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_SFX_Weapon_Sword_Swing, "Sound.SFX.Weapon.Sword.Swing", "Sword swing sound");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_SFX_Weapon_Sword_Hit, "Sound.SFX.Weapon.Sword.Hit", "Sword hit sound");

	// SFX Examples (Footstep)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_SFX_Footstep, "Sound.SFX.Footstep", "Footstep sound effects");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_SFX_Footstep_Stone, "Sound.SFX.Footstep.Stone", "Stone footstep sound");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_SFX_Footstep_Wood, "Sound.SFX.Footstep.Wood", "Wood footstep sound");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_SFX_Footstep_Grass, "Sound.SFX.Footstep.Grass", "Grass footstep sound");

	// Music Examples
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_Music_Combat, "Sound.Music.Combat", "Combat music");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_Music_Combat_Intense, "Sound.Music.Combat.Intense", "Intense combat music");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_Music_Ambient, "Sound.Music.Ambient", "Ambient music");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_Music_Ambient_Peaceful, "Sound.Music.Ambient.Peaceful", "Peaceful ambient music");

	// Voice Examples
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_Voice_Player, "Sound.Voice.Player", "Player voice");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_Voice_Player_Grunt, "Sound.Voice.Player.Grunt", "Player grunt sound");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Sound_Voice_Player_Death, "Sound.Voice.Player.Death", "Player death sound");

	// ============================================================
	// Mount System Tags
	// ============================================================

	// Input Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Mount, "InputTag.Mount", "Mount input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Dismount, "InputTag.Dismount", "Dismount input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_MountSpecialAbility, "InputTag.MountSpecialAbility", "Mount special ability input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_MountSprint, "InputTag.MountSprint", "Mount sprint input");

	// State Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Mounted, "State.Mounted", "Character is mounted");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Mounting, "State.Mounting", "Character is mounting");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Dismounting, "State.Dismounting", "Character is dismounting");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Mount_Sprinting, "State.Mount.Sprinting", "Mount is sprinting");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Mount_Flying, "State.Mount.Flying", "Mount is flying");

	// Mount Type Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Mount_Type_Ground, "Mount.Type.Ground", "Ground mount");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Mount_Type_Flying, "Mount.Type.Flying", "Flying mount");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Mount_Type_Amphibious, "Mount.Type.Amphibious", "Amphibious mount");

	// Event Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Mount_Mounted, "GameplayEvent.Mount.Mounted", "Successfully mounted");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Mount_Dismounted, "GameplayEvent.Mount.Dismounted", "Successfully dismounted");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Mount_ChargeAttack, "GameplayEvent.Mount.ChargeAttack", "Mount charge attack triggered");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Mount_Trample, "GameplayEvent.Mount.Trample", "Mount trample triggered");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Mount_AerialStrike, "GameplayEvent.Mount.AerialStrike", "Mount aerial strike triggered");

	// ============================================================
	// Fast Travel System Tags
	// ============================================================

	// Input Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_FastTravel, "InputTag.FastTravel", "Fast travel input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_OpenFastTravelMap, "InputTag.OpenFastTravelMap", "Open fast travel map");

	// State Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_FastTraveling, "State.FastTraveling", "Character is fast traveling");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_CanFastTravel, "State.CanFastTravel", "Character can fast travel");

	// Waypoint Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Waypoint_Discovered, "Waypoint.Discovered", "Waypoint is discovered");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Waypoint_Active, "Waypoint.Active", "Waypoint is active");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Waypoint_Locked, "Waypoint.Locked", "Waypoint is locked");

	// Event Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Waypoint_Discovered, "GameplayEvent.Waypoint.Discovered", "Waypoint discovered");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_FastTravel_Started, "GameplayEvent.FastTravel.Started", "Fast travel started");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_FastTravel_Completed, "GameplayEvent.FastTravel.Completed", "Fast travel completed");

	// ============================================================
	// Climbing and Parkour System Tags
	// ============================================================

	// Input Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Climb, "InputTag.Climb", "Climb input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Vault, "InputTag.Vault", "Vault input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Hang, "InputTag.Hang", "Hang input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Mantle, "InputTag.Mantle", "Mantle input");

	// State Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Climbing, "State.Climbing", "Character is climbing");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Hanging, "State.Hanging", "Character is hanging");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_WallRunning, "State.WallRunning", "Character is wall running");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Vaulting, "State.Vaulting", "Character is vaulting");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Mantling, "State.Mantling", "Character is mantling");

	// Surface Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Surface_Climbable, "Surface.Climbable", "Surface is climbable");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Surface_Climbable_Rough, "Surface.Climbable.Rough", "Rough climbable surface");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Surface_Climbable_Smooth, "Surface.Climbable.Smooth", "Smooth climbable surface");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Surface_Climbable_Ledge, "Surface.Climbable.Ledge", "Ledge surface");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Surface_Climbable_Ladder, "Surface.Climbable.Ladder", "Ladder surface");

	// Event Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Climbing_Started, "GameplayEvent.Climbing.Started", "Climbing started");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Climbing_Stopped, "GameplayEvent.Climbing.Stopped", "Climbing stopped");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Parkour_Vault, "GameplayEvent.Parkour.Vault", "Vault performed");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Parkour_WallJump, "GameplayEvent.Parkour.WallJump", "Wall jump performed");

	// ============================================================
	// Swimming and Diving System Tags
	// ============================================================

	// Input Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Dive, "InputTag.Dive", "Dive input");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Surface, "InputTag.Surface", "Surface input");

	// State Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Swimming, "State.Swimming", "Character is swimming");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Diving, "State.Diving", "Character is diving");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Underwater, "State.Underwater", "Character is underwater");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_LowOxygen, "State.LowOxygen", "Low oxygen warning");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Drowning, "State.Drowning", "Character is drowning");

	// Water Environment Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Water_Environment_Fresh, "Water.Environment.Fresh", "Freshwater environment");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Water_Environment_Salt, "Water.Environment.Salt", "Saltwater environment");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Water_Environment_Toxic, "Water.Environment.Toxic", "Toxic water environment");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Water_Environment_Lava, "Water.Environment.Lava", "Lava environment");

	// Event Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Swimming_Started, "GameplayEvent.Swimming.Started", "Swimming started");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Swimming_Stopped, "GameplayEvent.Swimming.Stopped", "Swimming stopped");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Diving_Started, "GameplayEvent.Diving.Started", "Diving started");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Oxygen_Depleted, "GameplayEvent.Oxygen.Depleted", "Oxygen depleted");

	// ============================================================
	// General Movement System Tags
	// ============================================================

	// Ability Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Movement_Sprint, "Ability.Movement.Sprint", "Sprint ability");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Movement_Dash, "Ability.Movement.Dash", "Dash ability");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Movement_Slide, "Ability.Movement.Slide", "Slide ability");

	// Movement Restriction Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Restricted_NoMount, "Movement.Restricted.NoMount", "Cannot mount in this area");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Restricted_NoClimb, "Movement.Restricted.NoClimb", "Cannot climb in this area");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Restricted_NoSwim, "Movement.Restricted.NoSwim", "Cannot swim in this area");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Restricted_NoFastTravel, "Movement.Restricted.NoFastTravel", "Cannot fast travel from this area");

	// ============================================================
	// Stat System Tags
	// ============================================================

	// Primary Stats
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Primary_Vitality, "Stat.Primary.Vitality", "Vitality stat - increases MaxHealth and defense");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Primary_Endurance, "Stat.Primary.Endurance", "Endurance stat - increases MaxStamina and equipment load");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Primary_Strength, "Stat.Primary.Strength", "Strength stat - increases physical damage");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Primary_Dexterity, "Stat.Primary.Dexterity", "Dexterity stat - increases attack speed and critical chance");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Primary_Intelligence, "Stat.Primary.Intelligence", "Intelligence stat - increases magic damage and max mana");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Primary_Faith, "Stat.Primary.Faith", "Faith stat - increases healing and miracle power");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Primary_Luck, "Stat.Primary.Luck", "Luck stat - increases critical chance and item discovery");

	// Combat Stats
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Combat_AttackPower, "Stat.Combat.AttackPower", "Attack power stat");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Combat_Defense, "Stat.Combat.Defense", "Defense stat");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Combat_CriticalChance, "Stat.Combat.CriticalChance", "Critical hit chance stat");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Combat_CriticalDamage, "Stat.Combat.CriticalDamage", "Critical hit damage multiplier stat");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Combat_Poise, "Stat.Combat.Poise", "Poise stat - resistance to stagger");

	// Resource Stats
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Resource_MaxHealth, "Stat.Resource.MaxHealth", "Maximum health stat");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Resource_MaxStamina, "Stat.Resource.MaxStamina", "Maximum stamina stat");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Resource_MaxMana, "Stat.Resource.MaxMana", "Maximum mana stat");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Resource_StaminaRegen, "Stat.Resource.StaminaRegen", "Stamina regeneration rate stat");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Resource_ManaRegen, "Stat.Resource.ManaRegen", "Mana regeneration rate stat");

	// Movement Stats
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Movement_Speed, "Stat.Movement.Speed", "Movement speed stat");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Movement_AttackSpeed, "Stat.Movement.AttackSpeed", "Attack speed stat");

	// Equipment Stats
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Stat_Equipment_MaxLoad, "Stat.Equipment.MaxLoad", "Maximum equipment load stat");

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
