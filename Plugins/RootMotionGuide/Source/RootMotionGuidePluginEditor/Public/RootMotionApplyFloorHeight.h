// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RootMotionGuideUtilityBase.h"
#include "RootMotionApplyFloorHeight.generated.h"

/**
 * Prevents the root bone's position from going below FloorHeight during StartFrame and EndFrame.
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API URootMotionApplyFloorHeight : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:
	
	/**
	* AnimSequence to be adjusted
	*/
	UPROPERTY(EditAnywhere, Category = "ApplyFloorHeight")
	TObjectPtr<class UAnimSequence> AnimSequence;

	/**
	* Start frame to apply floor height
	*/
	UPROPERTY(EditAnywhere, Category = "ApplyFloorHeight", meta = (DisplayName = "Start Frame"))
	int ApplyFloorHeightStartFrame;

	/**
	* End frame to apply floor height
	*/
	UPROPERTY(EditAnywhere, Category = "ApplyFloorHeight", meta = (DisplayName = "End Frame"))
	int ApplyFloorHeightEndFrame;

	UPROPERTY(EditAnywhere, Category = "ApplyFloorHeight")
	float FloorHeight;

	/**
	* Prevents the root bone's position from going below FloorHeight during StartFrame and EndFrame.
	*/
	FReply ApplyFloorHeight();
};
