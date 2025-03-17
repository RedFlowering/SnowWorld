// Copyright 2025 RedFlowering.

#pragma once

#include "NativeGameplayTags.h"

namespace RFGameplayTags
{
	QUANTUMASCENDRUNTIME_API	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

	// Ability Tag
	QUANTUMASCENDRUNTIME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_GrapplingHook_Pulling);
	QUANTUMASCENDRUNTIME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_GrapplingHook_Cancel);
	QUANTUMASCENDRUNTIME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Climbing);
};
