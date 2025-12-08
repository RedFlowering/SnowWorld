// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RootMotionGuideUtilityBase.h"

#include "RotateAnimationTrack.generated.h"

/**
 * Rotates the animation track value by the parent coordinate system.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API URotateAnimationTrack : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
	
public:

	/**
	* AnimSequence to be adjusted
	*/
	UPROPERTY(EditAnywhere, Category = "RotateAnimationTrack")	
	TObjectPtr<class UAnimSequence> AnimSequence;


	/**
	* The name of the bone to which the adjustment applies.
	*/
	UPROPERTY(EditAnywhere, Category = "RotateAnimationTrack")
	FName BoneName;

	
	/**
	* Adjustment rotation to be applied (By parent coordinate system)
	*/
	UPROPERTY(EditAnywhere, Category = "RotateAnimationTrack")
	FRotator AdjustRotation;
	
	/**
	* Rotates the animation track value by the parent coordinate system.
	*/
	FReply RotateAnimationTrack();
};
