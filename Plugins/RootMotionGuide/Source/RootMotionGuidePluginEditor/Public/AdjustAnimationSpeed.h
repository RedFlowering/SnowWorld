// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RootMotionGuideUtilityBase.h"
#include "AdjustAnimationSpeed.generated.h"

/**
 * Adjust the animation speed of some segments.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UAdjustAnimationSpeed : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:

	/**
	* AnimSequence to be modified speed
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustAnimationSpeed")
	TObjectPtr<class UAnimSequence> AnimSequence;
	
	
	/**
	* Frame to start speed adjustment
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustAnimationSpeed")
	int StartFrame;

	/**
	* Frame to finish speed adjustment
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustAnimationSpeed")
	int EndFrame;

	/**
	* Animation speed to be applied
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustAnimationSpeed")
	float Speed = 1.0f;

	/**
	* Frame length to blend in
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustAnimationSpeed")
	int BlendInFrames;

	/**
	* Frame length to blend out
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustAnimationSpeed")
	int BlendOutFrames;

	/**
	* Adjust the animation speed of some segments.
	*/
	FReply AdjustAnimationSpeed();
};
