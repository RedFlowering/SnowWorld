// Copyright 2025 RedFlowering.
#include "Tags/RFGameplayTags.h"

#include "Engine/EngineTypes.h"
#include "GameplayTagsManager.h"
#include "LyraLogChannels.h"

namespace RFGameplayTags
{
	// Ability 
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_GrapplingHook_Pulling, "InputTag.Pulling", "Pulling input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_GrapplingHook_Cancel, "InputTag.CancelGrappling", "GrapplingHook Cancel input.");

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
					UE_LOG(LogLyra, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}
}


