// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RootMotionGuideUtilityBase.h"

#include "TrimAnimation.generated.h"

/**
 * Trims the AnimSequence.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UTrimAnimation : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:

	/**
	* AnimSequence to apply trimming
	*/
	UPROPERTY(EditAnywhere, Category = "TrimAnimation")
	TObjectPtr<class UAnimSequence> AnimSequence;

	/**
	* Start frame of animation
	*/
	UPROPERTY(EditAnywhere, Category = "TrimAnimation")
	int StartFrame;

	/**
	* End frame of animation
	*/
	UPROPERTY(EditAnywhere, Category = "TrimAnimation")
	int EndFrame;

	/**
	* Trims the AnimSequence.
	*/
	FReply TrimAnimation();
};
