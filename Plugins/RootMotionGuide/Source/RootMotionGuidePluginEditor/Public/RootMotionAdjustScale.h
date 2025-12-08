// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RootMotionGuideUtilityBase.h"

#include "RootMotionAdjustScale.generated.h"

/**
 * Adjusts the scale of the root motion.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API URootMotionAdjustScale : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:
	URootMotionAdjustScale();

	/**
	* AnimSequence to which the adjustment will be applied
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustRootMotionScale")
	TObjectPtr<class UAnimSequence> AnimSequence;

	/**
	* Start frame to which root motion scale adjustment is applied
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustRootMotionScale", meta = (DisplayName = "Start Frame"))
	int AdjustScaleStartFrame;

	/**
	* End frame to which root motion scale adjustment is applied
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustRootMotionScale", meta = (DisplayName = "End Frame"))
	int AdjustScaleEndFrame;

	/**
	* Root motion scale value to be applied
	*/
	UPROPERTY(EditAnywhere, Category = "AdjustRootMotionScale")
	FVector AdjustScale;

	/**
	* Adjusts the scale of the root motion.
	*/
	FReply AdjustRootMotionScale();
};
