// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RootMotionGuideUtilityBase.h"

#include "AdjustHoldRootMotion.generated.h"

/**
 * Adjust the root motion to hold the position of the hold bone during a certain frame.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UAdjustHoldRootMotion : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:
	/**
	* AnimSequence to be adjusted
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustHoldRootMotion")
	TObjectPtr<class UAnimSequence> AnimSequence;
	
	/**
	* Name of bone to pin position
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustHoldRootMotion", meta = (DisplayName = "Hold Bone Name"))
	FName HoldRootMotionHoldBoneName;

	/**
	* Whether to fix the x-axis position value of the bone
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustHoldRootMotion", meta = (DisplayName = "Lock X"))
	bool bHoldRootMotionLockX;

	/**
	* Whether to fix the y-axis position value of the bone
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustHoldRootMotion", meta = (DisplayName = "Lock Y"))
	bool bHoldRootMotionLockY;

	/**
	* Whether to fix the z-axis position value of the bone
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustHoldRootMotion", meta = (DisplayName = "Lock Z"))
	bool bHoldRootMotionLockZ;

	/**
	* A start frame to fix the position of the bone
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustHoldRootMotion", meta = (DisplayName = "Start Frame"))
	int HoldRootMotionStartFrame;

	/**
	* An end frame to fix the position of the bone
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustHoldRootMotion", meta = (DisplayName = "End Frame"))
	int HoldRootMotionEndFrame;


	/**
	* Adjust the root motion to hold the position of the hold bone during a certain frame.
	*/
	FReply AdjustHoldRootMotion();
};
