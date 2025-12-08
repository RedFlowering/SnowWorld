// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RootMotionGuideUtilityBase.h"
#include "BlendAnimationLastPose.generated.h"

/**
 * Blend the last pose of the AnimSequence to the start pose of NextAnimSequence.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UBlendAnimationLastPose : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:

	/**
	* AnimSequence to blend
	*/
	UPROPERTY(EditAnywhere, Category = "BlendAnimationLastPose")
	TObjectPtr<class UAnimSequence> AnimSequence;
	
	/**
	* AnimSequence to be blended with the last pose
	*/
	UPROPERTY(EditAnywhere, Category = "BlendAnimationLastPose")
	TObjectPtr<class UAnimSequence> NextAnimSequence;
	
	/**
	* Frame to start blending
	*/
	UPROPERTY(EditAnywhere, Category = "BlendAnimationLastPose")
	int BlendStartFrame;

	/**
	* Frame to end blending
	*/
	UPROPERTY(EditAnywhere, Category = "BlendAnimationLastPose")
	int BlendEndFrame;

	/**
	* Blend the last pose of the AnimSequence to the start pose of NextAnimSequence.
	*/
	FReply BlendAnimationLastPose();
};
