// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "RootMotionGuideUtilityBase.h"
#include "ReverseAnimation.generated.h"

/**
 * Change the animation in reverse.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UReverseAnimation : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
	
public:

	// AnimSequence to reverse
	UPROPERTY(EditAnywhere, Category = "ReverseAnimation")
	TObjectPtr<class UAnimSequence> AnimSequence;

	// Change the animation in reverse.
	FReply ReverseAnimation();
};
