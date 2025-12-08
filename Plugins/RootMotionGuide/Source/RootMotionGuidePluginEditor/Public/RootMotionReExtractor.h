// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RootMotionGuideUtilityBase.h"
#include "RootMotionReExtractor.generated.h"


/**
* The axis of the source bone to be used to extract the root motion
*/
UENUM()
enum class ERootMotionReExtractorAxisType : uint8
{
	X,
	Y,
	Z,
};

/**
* How the RootMotionTargetRotationAtEnd value is applied
*/
UENUM()
enum class ERootMotionReExtractorTargetRotationType : uint8
{
	/** Add the RootMotionTargetRotationAtEnd value to the final rotation. */
	Offset,

	/** Makes the RootMotionTargetRotationAtEnd value the final rotation value. */
	Absolute,

	/** Ignores the rotation of the Source Bone and rotates to the RootMotionTargetRotationAtEnd value. */
	Override,

	/** Get the rotation from the source bone, but project it so that it only rotates in the vertical axis, offset may be added. */
	SourceRotation,
};



/**
 * Extracts the root motion animation from the SourceBone.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API URootMotionReExtractor : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:
	URootMotionReExtractor();

	/**
	* AnimSequence with root motion extracted
	*/
	UPROPERTY(EditAnywhere, Category = "ExtractRootMotionFromSource")
	TObjectPtr<class UAnimSequence> AnimSequence;

	/**
	* The bone name that contains the root motion data.
	* Must be a child of the root bone.
	*/
	UPROPERTY(EditAnywhere, Category = "ExtractRootMotionFromSource")
	FName RootMotionSourceBoneName;

	/**
	* The axis of the bone for obtaining rotation information when extracting the root motion data
	*/
	UPROPERTY(EditAnywhere, Category = "ExtractRootMotionFromSource")
	ERootMotionReExtractorAxisType RootMotionSourceBoneForwardAxis;


	/**
	* Source bone height from root bone in start frame
	*/
	UPROPERTY(EditAnywhere, Category = "ExtractRootMotionFromSource")
	float RootMotionSourceHeightAtStart;

	/**
	* Source bone height from root bone at end frame
	*/
	UPROPERTY(EditAnywhere, Category = "ExtractRootMotionFromSource")
	float RootMotionSourceHeightAtEnd;

	/**
	* Number of Frames over which to average the rotation and translation of the source bone
	*/
	UPROPERTY(EditAnywhere, Category = "ExtractRootMotionFromSource", meta = (ClampMin = "0"))
	int RootMotionSamplingInterval;

	/**
	* How to adjust the rotation of the root bone of the last frame
	* Offset,			// Add the RootMotionTargetRotationAtEnd value to the final rotation.
	* Absolute,			// Makes the RootMotionTargetRotationAtEnd value the final rotation value.
	* Override,			// Ignores the rotation of the Source Bone and rotates to the RootMotionTargetRotationAtEnd value.
	* SourceRotation,	// Get the rotation from the source bone, but project it so that it only rotates in the vertical axis, offset may be added.
	*/
	UPROPERTY(EditAnywhere, Category = "ExtractRootMotionFromSource")
	ERootMotionReExtractorTargetRotationType TargetRotationType;

	/**
	* The value to be applied to the rotation adjustment of the root bone of the last frame.
	*/
	UPROPERTY(EditAnywhere, Category = "ExtractRootMotionFromSource")
	float RootMotionTargetRotationAtEnd;

	/**
	* Force root motion extraction.
	* Existing root motion will be erased.
	*/
	UPROPERTY(EditAnywhere, Category = "ExtractRootMotionFromSource", AdvancedDisplay)
	bool bForceRootMotionExtraction;

	/**
	* Returns whether the root bone animation track needs to be added
	*/
	bool NeedToAddRootBoneAnimationTrack() const;

	/**
	* Returns Visibility of AddRootBoneAnimationTrackButton
	*/
	EVisibility GetVisibilityAddRootBoneAnimationTrackButton() const;


	/**
	* Returns the Visibility of the ExtractRootMotionButton
	*/
	EVisibility GetVisibilityExtractRootMotionButton() const;

	/**
	* Add root bone animation track
	*/
	FReply	AddRootBoneAnimationTrack();

	/**
	* Extract root motion data from source bone
	*/
	FReply	ExtractRootMotionFromSource();

	/**
	* Whether root motion data can be extracted
	*/
	bool CanExtractRootMotion() const;

	/**
	* Calculates the rotation value of the root bone in the rotation of the source bone.
	*/
	FRotator GetRootRotation(const FQuat& RootSourceRot, ERootMotionReExtractorAxisType ForwardAxis);

	static void AddContextMenuItem();

private:
	static FDelegateHandle ContentBrowserExtenderDelegateHandle;
	static void RemoveContextMenuItem();
	static void ExecuteExtractRootMotion(TArray<FAssetData> AssetDataArray);
	
	
	
};
