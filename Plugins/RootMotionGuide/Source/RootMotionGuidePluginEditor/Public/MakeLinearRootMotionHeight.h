// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RootMotionGuideUtilityBase.h"

#include "MakeLinearRootMotionHeight.generated.h"

/**
 * Changes the value of the z-axis position of the root motion to linear interpolation for a certain frame.
 * It is used to correct the character bouncing when landing.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UMakeLinearRootMotionHeight : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:
	/**
	* AnimSequence to be adjusted
	*/
	UPROPERTY(EditAnywhere, Category = "MakeLinearRootMotionHeight")
	TObjectPtr<class UAnimSequence> AnimSequence;
	
	/**
	* Start frame to apply linear interpolation
	*/
	UPROPERTY(EditAnywhere, Category = "MakeLinearRootMotionHeight", meta = (DisplayName = "Start Frame"))
	int LinearRootMotionHeightStartFrame;

	/**
	* End frame to apply linear interpolation
	*/
	UPROPERTY(EditAnywhere, Category = "MakeLinearRootMotionHeight", meta = (DisplayName = "End Frame"))
	int LinearRootMotionHeightEndFrame;

	/**
	* Changes the value of the z-axis position of the root motion to linear interpolation for a certain frame.
	*/
	FReply MakeLinearRootMotionHeight();
};
