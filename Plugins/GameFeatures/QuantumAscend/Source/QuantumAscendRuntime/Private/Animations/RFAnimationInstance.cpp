// Copyright 2025 RedFlowering.

#include "Animations/RFAnimationInstance.h"
#include "Character/RFCharacter.h"

URFAnimationInstance::URFAnimationInstance()
{
	RootMotionMode = ERootMotionMode::IgnoreRootMotion;
	bUseMainInstanceMontageEvaluationData = true;
}

void URFAnimationInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (GetOwningActor())
	{
		OwnerCharacter = Cast<ARFCharacter>(GetOwningActor());
	}

#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		// Use default objects for editor preview.

		if (!IsValid(OwnerCharacter))
		{
			OwnerCharacter = GetMutableDefault<ARFCharacter>();
		}
	}
#endif
}

void URFAnimationInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}

void URFAnimationInstance::NativeUpdateAnimation(float DeltaSeconds)
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
