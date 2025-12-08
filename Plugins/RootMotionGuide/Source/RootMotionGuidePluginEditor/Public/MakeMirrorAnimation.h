// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/MirrorDataTable.h"
#include "RootMotionGuideUtilityBase.h"
#include "MakeMirrorAnimation.generated.h"

/**
 * 
 */
UCLASS()
class ROOTMOTIONGUIDEPLUGINEDITOR_API UMakeMirrorAnimation : public URootMotionGuideUtilityBase
{
	GENERATED_BODY()
public:

	/**
	* AnimSequence to be modified
	*/
	UPROPERTY(EditAnywhere, Category = "MakeMirrorAnimation")
	TObjectPtr<class UAnimSequence> AnimSequence;

	/*
	* MirrorDataTable for AnimSequence
	*/
	UPROPERTY(EditAnywhere, Category = "MakeMirrorAnimation")
	TObjectPtr<UMirrorDataTable> MirrorDataTable;


	/**
	* Apply Mirror animation.
	*/
	FReply MakeMirrorAnimation();
};
