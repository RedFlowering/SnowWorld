// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Animation/AnimTypes.h"
#include "CommonAnimTypes.h"	// for 4.17 ~
#include "RootMotionGuideUtilityBase.h"
#include "MakeLookAtAnimation.generated.h"


struct FAxis;

UENUM()
enum class EMakeLookAtAnimationInterpolationBlend : uint8
{
	Linear,
	Cubic,
	Sinusoidal,
	EaseInOutExponent2,
	EaseInOutExponent3,
	EaseInOutExponent4,
	EaseInOutExponent5,
};

/**
 * Apply LookAt animation.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UMakeLookAtAnimation : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:
	/**
	* AnimSequence to be modified
	*/
	UPROPERTY(EditAnywhere, Category = "MakeLookAtAnimation")
	TObjectPtr<class UAnimSequence> AnimSequence;

	/**
	* Frame to start applying LookAt
	*/
	UPROPERTY(EditAnywhere, Category = "MakeLookAtAnimation")
	int StartFrame;

	/**
	* Last frame to apply LookAt
	*/
	UPROPERTY(EditAnywhere, Category = "MakeLookAtAnimation")
	int EndFrame;

	/**
	* Frame length to blend in
	*/
	UPROPERTY(EditAnywhere, Category = "MakeLookAtAnimation")
	int BlendInFrames;

	/**
	* Frame length to blend out
	*/
	UPROPERTY(EditAnywhere, Category = "MakeLookAtAnimation")
	int BlendOutFrames;

	/** Name of bone to control. This is the main bone chain to modify from. **/
	UPROPERTY(EditAnywhere, Category = SkeletalControl)
	FName BoneNameToModify;

	/** Target Bone to look at - You can use  LookAtLocation if you need offset from this point. That location will be used in their local space. **/
	UPROPERTY(EditAnywhere, Category = Target)
	FName LookAtBoneName;

	/** Target Offset. It's in world space if LookAtBone is empty or it is based on LookAtBone in their local space*/
	UPROPERTY(EditAnywhere, Category = Target)
	FVector LookAtLocation = FVector(0.0f, 0.0f, 0.0f);

	/** Look at axis, which axis to align to look at point */
	UPROPERTY(EditAnywhere, Category = SkeletalControl)
	FAxis LookAt_Axis = FAxis(FVector(0.0f, 1.0f, 0.0f));

	/** Whether or not to use Look up axis */
	UPROPERTY(EditAnywhere, Category = SkeletalControl)
	bool bUseLookUpAxis;

	/** Look up axis in local space */
	UPROPERTY(EditAnywhere, Category = SkeletalControl)
	FAxis LookUp_Axis = FAxis(FVector(1.0f, 0.0f, 0.0f));

	/** Look at Clamp value in degree - if you're look at axis is Z, only X, Y degree of clamp will be used*/
	UPROPERTY(EditAnywhere, Category = SkeletalControl)
	float LookAtClamp;

	UPROPERTY(EditAnywhere, Category = SkeletalControl)
	EMakeLookAtAnimationInterpolationBlend InterpolationType;

	UPROPERTY(EditAnywhere, Category = SkeletalControl)
	float InterpolationTime;

	UPROPERTY(EditAnywhere, Category = SkeletalControl)
	float InterpolationTriggerThreashold;
	

	/**
	* Apply LookAt animation.
	*/
	FReply MakeLookAtAnimation();

private:

	/** Turn a linear interpolated alpha into the corresponding AlphaBlendType */
	static float AlphaToBlendType(float InAlpha, EMakeLookAtAnimationInterpolationBlend BlendType);
};
