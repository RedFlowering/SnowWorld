// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RootMotionGuideUtilityBase.h"

#include "AddRootMotion.generated.h"

/**
 * Adds additional root motion over a given frame.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UAddRootMotion : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
	
public:
	/**
	* AnimSequence to be modified
	*/
	UPROPERTY(EditAnywhere, Category = "AddRootMotion")
	TObjectPtr<class UAnimSequence> AnimSequence;

	/**
	* Start frame to which root motion is added
	*/
	UPROPERTY(EditAnywhere, Category = "AddRootMotion", meta = (DisplayName = "Start Frame"))
	int AddRootMotionStartFrame;

	/**
	* End frame to which root motion is to be added
	*/
	UPROPERTY(EditAnywhere, Category = "AddRootMotion", meta = (DisplayName = "End Frame"))
	int AddRootMotionEndFrame;

	/**
	* Only the root bone applies.
	* Turning on this option can produce incorrect results. Please use it carefully.
	*/
	UPROPERTY(EditAnywhere, Category = "AddRootMotion", meta = (DisplayName = "Apply Only Root Bone"), AdvancedDisplay)
	bool bApplyOnlyRootBone;

	/**
	* The root motion to be added.
	* It is divided into Start Frame and End Frame and applied.
	*/
	UPROPERTY(EditAnywhere, Category = "AddRootMotion")
	FVector AddRootMotionValue;

	/**
	* Gravitational acceleration to be applied
	*/
	UPROPERTY(EditAnywhere, Category = "AddRootMotion")
	float GravityZ;
	
	/**
	* Adds additional root motion over a given frame.
	*/
	FReply AddRootMotion();
};
