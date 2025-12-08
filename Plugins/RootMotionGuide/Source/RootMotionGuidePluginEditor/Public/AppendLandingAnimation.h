// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RootMotionGuideUtilityBase.h"

#include "AppendLandingAnimation.generated.h"

/**
 * Add landing animation.
 * AnimSequence = AnimSequence + FallingAnimSequence (Loop) + LandingAnimSequence
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UAppendLandingAnimation : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
	
public:
	/**
	* AnimSequence to add landing animation
	*/
	UPROPERTY(EditAnywhere, Category = "AppendLandingAnimation")
	TObjectPtr<class UAnimSequence> AnimSequence;

	/**
	* Gravity value to apply when falling
	*/
	UPROPERTY(EditAnywhere, Category = "AppendLandingAnimation")
	float GravityZ = -980.0f;

	/**
	* AnimSequence to apply when going down
	*/
	UPROPERTY(EditAnywhere, Category = "AppendLandingAnimation")
	TObjectPtr<class UAnimSequence> FallingAnimSequence;

	/**
	* Frame length to fall
	*/
	UPROPERTY(EditAnywhere, Category = "AppendLandingAnimation")
	int FallingFrames;

	/**
	* Blending frame when falling animation
	*/
	UPROPERTY(EditAnywhere, Category = "AppendLandingAnimation")
	int FallingBlendFrames;

	/**
	* Landing AnimSequence
	*/
	UPROPERTY(EditAnywhere, Category = "AppendLandingAnimation")
	TObjectPtr<class UAnimSequence> LandingAnimSequence;

	/**
	* Time to start landing animation
	*/
	UPROPERTY(EditAnywhere, Category = "AppendLandingAnimation")
	float LandingStartTime;

	/**
	* Landing animation blending frame
	*/
	UPROPERTY(EditAnywhere, Category = "AppendLandingAnimation")
	int LandingBlendFrames;
	
	/**
	* Add landing animation.
	*/
	FReply AppendLandingAnimation();
};
