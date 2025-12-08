// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Animation/AnimTypes.h"
#include "CommonAnimTypes.h"	// for 4.17 ~
#include "RootMotionGuideUtilityBase.h"
#include "Animation/Skeleton.h"

#include "SyncBoneLocation.generated.h"

/**
 * Sync the location of 'Sync Bone' to the Source Bone location of the Source AnimSequence.
 * This tool is used to synchronize the retargeted animation with the original animation.
 * This internally uses Two Bone IK.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API USyncBoneLocation : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:
	/**
	* AnimSequence to be modified
	*/
	UPROPERTY(EditAnywhere, Category = "SyncBoneLocation")
	TObjectPtr<class UAnimSequence> AnimSequence;

	/**
	* Frame to start applying IK
	*/
	UPROPERTY(EditAnywhere, Category = "SyncBoneLocation")
	int StartFrame;

	/**
	* Last frame to apply IK
	*/
	UPROPERTY(EditAnywhere, Category = "SyncBoneLocation")
	int EndFrame;

	/**
	* Frame length to blend in
	*/
	UPROPERTY(EditAnywhere, Category = "SyncBoneLocation")
	int BlendInFrames;

	/**
	* Frame length to blend out
	*/
	UPROPERTY(EditAnywhere, Category = "SyncBoneLocation")
	int BlendOutFrames;

	/**
	* Bone name to sync
	*/
	UPROPERTY(EditAnywhere, Category = "SyncBoneLocation")
	FName SyncBoneName;

	/**
	* Source AnimSequence
	*/
	UPROPERTY(EditAnywhere, Category = "SyncBoneLocation")
	TObjectPtr<class UAnimSequence> SourceAnimSequence;

	/**
	* Bone name to be IK target
	*/
	UPROPERTY(EditAnywhere, Category = "SyncBoneLocation")
	FName SourceBoneName;


	/** Whether or not to apply twist on the chain of joints. This clears the twist value along the TwistAxis */
	UPROPERTY(EditAnywhere, Category = IK, AdvancedDisplay)
	bool bAllowTwist = true;

	/** Specify which axis it's aligned. Used when removing twist */
	UPROPERTY(EditAnywhere, Category = IK, AdvancedDisplay, meta = (editcondition = "!bAllowTwist"))
	FAxis TwistAxis;

	FReply SyncBoneLocation();
	
private:
	/**
	* Convert a FTransform in a specified bone space to ComponentSpace.
	*/
	void ConvertBoneSpaceTransformToCS(const FReferenceSkeleton& RefSkeleton, TArray< TArray< FTransform > > AnimationDataInCS, FTransform& InOutBoneSpaceTM, int32 BoneIndex, int32 Key, EBoneControlSpace Space);

	void RemoveTwist(const FTransform& InParentTransform, FTransform& InOutTransform, const FTransform& OriginalLocalTransform, const FVector& InAlignVector);
	

	FVector GetJointTargetPosition(const FReferenceSkeleton& SrcRefSkeleton, TArray< TArray< FTransform > > SrcAnimationDataInCS, int32 SrcEndBoneIndex, const FReferenceSkeleton& RefSkeleton, TArray< TArray< FTransform > > AnimationDataInCS, int32 EndBoneIndex, int32 Key);
};
