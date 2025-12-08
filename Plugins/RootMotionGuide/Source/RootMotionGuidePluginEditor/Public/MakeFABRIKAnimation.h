// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Animation/AnimTypes.h"
#include "CommonAnimTypes.h"	// for 4.17 ~
#include "RootMotionGuideUtilityBase.h"
#include "Animation/Skeleton.h"

#include "MakeFABRIKAnimation.generated.h"

/**
 * Apply FABRIK animation.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UMakeFABRIKAnimation : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:
	/**
	* AnimSequence to be modified
	*/
	UPROPERTY(EditAnywhere, Category = "MakeFABRIKAnimation")
	TObjectPtr<class UAnimSequence> AnimSequence;

	/**
	* Frame to start applying IK
	*/
	UPROPERTY(EditAnywhere, Category = "MakeFABRIKAnimation")
	int StartFrame;

	/**
	* Last frame to apply IK
	*/
	UPROPERTY(EditAnywhere, Category = "MakeFABRIKAnimation")
	int EndFrame;

	/**
	* Frame length to blend in
	*/
	UPROPERTY(EditAnywhere, Category = "MakeFABRIKAnimation")
	int BlendInFrames;

	/**
	* Frame length to blend out
	*/
	UPROPERTY(EditAnywhere, Category = "MakeFABRIKAnimation")
	int BlendOutFrames;

	/** Coordinates for target location of tip bone - if EffectorLocationSpace is bone, this is the offset from Target Bone to use as target location*/
	UPROPERTY(EditAnywhere, Category = EndEffector)
	FTransform EffectorTransform = FTransform::Identity;

	/** Reference frame of Effector Transform. */
	UPROPERTY(EditAnywhere, Category = EndEffector)
	TEnumAsByte<enum EBoneControlSpace> EffectorTransformSpace = BCS_ComponentSpace;

	/** If EffectorTransformSpace is a bone, this is the bone to use. **/
	UPROPERTY(EditAnywhere, Category = EndEffector)
	FName EffectorTransformBoneName;

	UPROPERTY(EditAnywhere, Category = EndEffector)
	TEnumAsByte<enum EBoneRotationSource> EffectorRotationSource = BRS_KeepLocalSpaceRotation;

	/** Name of tip bone */
	UPROPERTY(EditAnywhere, Category = Solver)
	FName IKTipBoneName;

	/** Name of the root bone*/
	UPROPERTY(EditAnywhere, Category = Solver)
	FName IKRootBoneName;

	/** Tolerance for final tip location delta from EffectorLocation*/
	UPROPERTY(EditAnywhere, Category = Solver)
	float Precision = 1.0f;

	/** Maximum number of iterations allowed, to control performance. */
	UPROPERTY(EditAnywhere, Category = Solver)
	int32 MaxIterations = 10;

	/**
	* Apply FABRIK animation.
	*/
	FReply MakeFABRIKAnimation();
	
private:

	/** Transient structure for FABRIK node evaluation */
	struct FABRIKChainLink
	{
	public:
		FVector Position;

		float Length;

		int32 BoneIndex;

		int32 TransformIndex;

		TArray< int32 > ChildZeroLengthTransformIndices;

		FABRIKChainLink()
			:Position(FVector::ZeroVector)
			,Length(0.0f)
			,BoneIndex(INDEX_NONE)
			,TransformIndex(INDEX_NONE)
		{

		}

		FABRIKChainLink(const FVector& InPosition, const float& InLength, const int32& InBoneIndex, const int32& InTransformIndex)
			:Position(InPosition)
			, Length(InLength)
			, BoneIndex(InBoneIndex)
			, TransformIndex(InTransformIndex)
		{

		}
	};
	
	/**
	* Convert a FTransform in a specified bone space to ComponentSpace.
	*/
	void ConvertBoneSpaceTransformToCS(const FReferenceSkeleton& RefSkeleton, TArray< TArray< FTransform > > AnimationDataInCS, FTransform& InOutBoneSpaceTM, int32 BoneIndex, int32 Key, EBoneControlSpace Space);

	FVector GetCurrentLocation(const TArray< TArray< FTransform > >& AnimationDataInCS, const int32& BoneIndex, const int32& Key);
};
