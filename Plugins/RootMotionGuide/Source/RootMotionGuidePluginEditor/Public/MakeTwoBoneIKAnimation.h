// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Animation/AnimTypes.h"
#include "CommonAnimTypes.h"	// for 4.17 ~
#include "RootMotionGuideUtilityBase.h"
#include "Animation/Skeleton.h"

#include "MakeTwoBoneIKAnimation.generated.h"

/**
 * Apply Two Bone IK.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UMakeTwoBoneIKAnimation : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:
	/**
	* AnimSequence to be modified
	*/
	UPROPERTY(EditAnywhere, Category = "MakeTwoBoneIKAnimation")
	TObjectPtr<class UAnimSequence> AnimSequence;

	/**
	* Frame to start applying IK
	*/
	UPROPERTY(EditAnywhere, Category = "MakeTwoBoneIKAnimation")
	int StartFrame;

	/**
	* Last frame to apply IK
	*/
	UPROPERTY(EditAnywhere, Category = "MakeTwoBoneIKAnimation")
	int EndFrame;

	/**
	* Frame length to blend in
	*/
	UPROPERTY(EditAnywhere, Category = "MakeTwoBoneIKAnimation")
	int BlendInFrames;

	/**
	* Frame length to blend out
	*/
	UPROPERTY(EditAnywhere, Category = "MakeTwoBoneIKAnimation")
	int BlendOutFrames;

	/** Name of bone to control. This is the main bone chain to modify from. **/
	UPROPERTY(EditAnywhere, Category = IK)
	FName IKBoneName;
	
	/** Effector Location. Target Location to reach. */
	UPROPERTY(EditAnywhere, Category = EndEffector)
	FVector EffectorLocation;

	/** Joint Target Location. Location used to orient Joint bone. **/
	UPROPERTY(EditAnywhere, Category = JointTarget)
	FVector JointTargetLocation;

	/** If EffectorLocationSpace is a bone, this is the bone to use. **/
	UPROPERTY(EditAnywhere, Category = EndEffector)
	FName EffectorSpaceBoneName;

	/** Set end bone to use End Effector rotation */
	UPROPERTY(EditAnywhere, Category = EndEffector)
	bool bTakeRotationFromEffectorSpace;

	/** Keep local rotation of end bone */
	UPROPERTY(EditAnywhere, Category = EndEffector)
	bool bMaintainEffectorRelRot;

	/** Should stretching be allowed, to be prevent over extension */
	UPROPERTY(EditAnywhere, Category = IK)
	bool bAllowStretching;

	/** Limits to use if stretching is allowed. This value determines when to start stretch. For example, 0.9 means once it reaches 90% of the whole length of the limb, it will start apply. */
	UPROPERTY(EditAnywhere, Category = IK, meta = (editcondition = "bAllowStretching", ClampMin = "0.0", UIMin = "0.0"))
	float StartStretchRatio = 1.0f;

	/** Limits to use if stretching is allowed. This value determins what is the max stretch scale. For example, 1.5 means it will stretch until 150 % of the whole length of the limb.*/
	UPROPERTY(EditAnywhere, Category = IK, meta = (editcondition = "bAllowStretching", ClampMin = "0.0", UIMin = "0.0"))
	float MaxStretchScale = 1.2f;

	/** Reference frame of Effector Location. */
	UPROPERTY(EditAnywhere, Category = EndEffector)
	TEnumAsByte<enum EBoneControlSpace> EffectorLocationSpace = EBoneControlSpace::BCS_ComponentSpace;
	
	/** Reference frame of Joint Target Location. */
	UPROPERTY(EditAnywhere, Category = JointTarget)
	TEnumAsByte<enum EBoneControlSpace> JointTargetLocationSpace = EBoneControlSpace::BCS_ComponentSpace;

	/** If JointTargetSpaceBoneName is a bone, this is the bone to use. **/
	UPROPERTY(EditAnywhere, Category = JointTarget)
	FName JointTargetSpaceBoneName;

	/** Whether or not to apply twist on the chain of joints. This clears the twist value along the TwistAxis */
	UPROPERTY(EditAnywhere, Category = IK)
	bool bAllowTwist = true;

	/** Specify which axis it's aligned. Used when removing twist */
	UPROPERTY(EditAnywhere, Category = IK, meta = (editcondition = "!bAllowTwist"))
	FAxis TwistAxis;

	/**
	* Apply Two Bone IK.
	*/
	FReply MakeTwoBoneIKAnimation();


private:

	/**
	* Convert a FTransform in a specified bone space to ComponentSpace.
	*/
	void ConvertBoneSpaceTransformToCS(const FReferenceSkeleton& RefSkeleton, TArray< TArray< FTransform > > AnimationDataInCS, FTransform& InOutBoneSpaceTM, int32 BoneIndex, int32 Key, EBoneControlSpace Space);

	void RemoveTwist(const FTransform& InParentTransform, FTransform& InOutTransform, const FTransform& OriginalLocalTransform, const FVector& InAlignVector);

};
