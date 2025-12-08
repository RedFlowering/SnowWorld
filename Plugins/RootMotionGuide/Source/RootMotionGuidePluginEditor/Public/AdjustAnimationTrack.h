// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RootMotionGuideUtilityBase.h"

#include "AdjustAnimationTrack.generated.h"

/**
 * Adjusts the position key value of the animation track.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UAdjustAnimationTrack : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
	
public:
	/**
	* AnimSequence to be adjusted
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustAnimationTrack")
	TObjectPtr<class UAnimSequence> AnimSequence;

	/**
	* The name of the bone to which the adjustment applies.
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustAnimationTrack")
	FName BoneName;

	/**
	* Adjustment location to be applied
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustAnimationTrack")
	FVector AdjustLocation;

	/**
	* Adjustment rotation to be applied
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustAnimationTrack")
	FRotator AdjustRotation;
	
	/**
	* Adjusts the position key value of the animation track.
	*/
	FReply AdjustAnimationTrack();
};
