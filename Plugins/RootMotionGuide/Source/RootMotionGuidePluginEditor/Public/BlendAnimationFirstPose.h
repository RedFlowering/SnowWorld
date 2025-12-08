// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "RootMotionGuideUtilityBase.h"
#include "BlendAnimationFirstPose.generated.h"

/**
 * Blend the first pose of the AnimSequence to the last pose of the PrevAnimSequence.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UBlendAnimationFirstPose : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:

	/**
	* AnimSequence to blend
	*/
	UPROPERTY(EditAnywhere, Category = "BlendAnimationFirstPose")
	TObjectPtr<class UAnimSequence> AnimSequence;

	/**
	* AnimSequence to be blended with the first pose
	*/
	UPROPERTY(EditAnywhere, Category = "BlendAnimationFirstPose")
	TObjectPtr<class UAnimSequence> PrevAnimSequence;

	/**
	* Frames to Apply Blending
	*/
	UPROPERTY(EditAnywhere, Category = "BlendAnimationFirstPose")
	int BlendFrames;


	/**
	* Blend the first pose of the AnimSequence to the last pose of the PrevAnimSequence.
	*/
	FReply BlendAnimationFirstPose();
};
