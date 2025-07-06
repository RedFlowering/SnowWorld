// Copyright 2025 Snow Game Studio.

#include "Animations/QAAnimationInstance.h"
#include "Character/BaseCharacter.h"

UQAAnimationInstance::UQAAnimationInstance()
{
	RootMotionMode = ERootMotionMode::IgnoreRootMotion;
	bUseMainInstanceMontageEvaluationData = true;
}

void UQAAnimationInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (GetOwningActor())
	{
		OwnerCharacter = Cast<ABaseCharacter>(GetOwningActor());
	}

#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		// Use default objects for editor preview.

		if (!IsValid(OwnerCharacter))
		{
			OwnerCharacter = GetMutableDefault<ABaseCharacter>();
		}
	}
#endif
}

void UQAAnimationInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

void UQAAnimationInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwnerCharacter)
	{
		RightHandIK = OwnerCharacter->GetRightHandIK();
		LeftHandIK = OwnerCharacter->GetLeftHandIK();

		bUseRightHandIK = OwnerCharacter->GetUseRightHandIK();
		bUseLeftHandIK = OwnerCharacter->GetUseLeftHandIK();
	}
}
