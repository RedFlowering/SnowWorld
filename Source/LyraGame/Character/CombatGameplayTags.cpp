// Copyright 2024 Snow Game Studio.

#include "CombatGameplayTags.h"

namespace CombatGameplayTags
{
	// Lock-on targeting tags
	UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_LockOn, "Ability.LockOn");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Status_LockedOn, "Status.LockedOn");

	// Dodge/Roll tags
	UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Dodge, "Ability.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Dodging, "Status.Dodging");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Status_Invincible, "Status.Invincible");

	// Input tags
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_LockOn, "InputTag.LockOn");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_Dodge, "InputTag.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_SwitchTargetLeft, "InputTag.SwitchTargetLeft");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Input_SwitchTargetRight, "InputTag.SwitchTargetRight");
}
