// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "RootMotionGuideUtilityBase.h"
#include "RemoveAnimationTrack.generated.h"

/**
* Default value to set when removing animation tracks
*/
UENUM()
enum class ERemoveAnimationTrackDefaultValueType : uint8
{
	// Sets the animation first key value to default.
	AnimFirstFrame,

	// Set the transform of the reference pose to the default value.
	RefPose,
};


/**
 * Removes animation key values for the bone.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API URemoveAnimationTrack : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
	
public:

	// AnimSequence to remove the animation track
	UPROPERTY(EditAnywhere, Category = "RemoveAnimationTrack")
	TObjectPtr<class UAnimSequence> AnimSequence;
	
	// Bone name to remove animation track
	UPROPERTY(EditAnywhere, Category = "RemoveAnimationTrack")
	FName RemoveBoneName;

	// Removes the animation tracks of child bones.
	UPROPERTY(EditAnywhere, Category = "RemoveAnimationTrack")
	bool bRemoveAnimationTrackOfAllChildBones;
	
	// Default value to set when removing animation tracks
	UPROPERTY(EditAnywhere, Category = "RemoveAnimationTrack")
	ERemoveAnimationTrackDefaultValueType DefaultValueType;

	// Remove animation track
	FReply RemoveAnimationTrack();
};
