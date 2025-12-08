// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RootMotionGuideUtilityBase.h"
#include "AppendAnimation.generated.h"

/**
 * Add a next animation to this animation.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UAppendAnimation : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:

	/**
	* AnimSequence to be added
	*/
	UPROPERTY(EditAnywhere, Category = "AppendAnimation")
	TObjectPtr<class UAnimSequence> AnimSequence;

	/**
	* The AnimSequence to add
	*/
	UPROPERTY(EditAnywhere, Category = "AppendAnimation")
	TObjectPtr<class UAnimSequence> NextAnimSequence;

	/**
	* Animation time to start adding
	*/
	UPROPERTY(EditAnywhere, Category = "AppendAnimation")
	float NextStartTime;

	/**
	* Frame length to blend
	*/
	UPROPERTY(EditAnywhere, Category = "AppendAnimation")
	int NextBlendFrames;


	/**
	* Add a next animation to this animation.
	*/
	FReply AppendAnimation();
};
