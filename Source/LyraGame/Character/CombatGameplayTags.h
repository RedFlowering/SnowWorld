// Copyright 2024 Snow Game Studio.

#pragma once

#include "NativeGameplayTags.h"

namespace CombatGameplayTags
{
	// Lock-on targeting tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_LockOn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_LockedOn);

	// Dodge/Roll tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Dodge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Dodging);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Status_Invincible);

	// Input tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_LockOn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Dodge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_SwitchTargetLeft);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_SwitchTargetRight);
}
