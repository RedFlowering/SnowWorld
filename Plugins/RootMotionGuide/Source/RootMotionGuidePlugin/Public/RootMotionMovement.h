// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RootMotionGuide.h"
#include "RootMotionMovement.generated.h"


/**
* Handles movement of the character's root motion.
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOTMOTIONGUIDEPLUGIN_API URootMotionMovement : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URootMotionMovement();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

public:
	/*
	* Called when the root motion animation starts.
	*/
	UFUNCTION(BlueprintCallable, Category = "RootMotionMovement")
	void BeginRootMotion(bool& bResult, ARootMotionGuide*& RootMotionGuide);

	/**
	* Called when the root motion animation ends.
	*/
	UFUNCTION(BlueprintCallable, Category = "RootMotionMovement")
	void EndRootMotion();

	/**
	* Called when starting root motion animation on network play.
	*/
	UFUNCTION(BlueprintCallable, Category = "RootMotionMovement")
	void BeginRootMotionNetwork(FString RootMotionGuideName, bool& bResult, ARootMotionGuide*& RootMotionGuide);

	/**
	* Called from UpdateCustomMovement.
	*/
	UFUNCTION(BlueprintCallable, Category = "RootMotionMovement")
	void UpdateRootMotion(float DeltaTime);
		
	/**
	* Returns whether the root motion animation is playing
	*/
	UFUNCTION(BlueprintPure, Category = "RootMotionMovement")
	bool IsPlayingRootMotion() const;

	/**
	* Whether you are playing root motion animation with the RootMotionGuide.
	*/
	UFUNCTION(BlueprintPure, Category = "RootMotionMovement")
	bool IsPlayingWith(ARootMotionGuide* RootMotionGuide) const;

	/**
	* Whether you are playing root motion animation with the CurrentOverlappedRootMotionGuide.
	*/
	UFUNCTION(BlueprintPure, Category = "RootMotionMovement")
	bool IsPlayingWithCurrentOverlappedRootMotionGuide() const;

	/**
	* Called when the root motion guide overlaps
	* If IsAutoTrigger is true, play the root motion guide.
	*/
	UFUNCTION(BlueprintCallable, Category = "RootMotionMovement")
	void BeginOverlap(ARootMotionGuide* RootMotionGuide, bool& bIsAutoTrigger);

	/**
	* Called at the end of overlap of the root motion guide.
	*/
	UFUNCTION(BlueprintCallable, Category = "RootMotionMovement")
	void EndOverlap(ARootMotionGuide* RootMotionGuide);

	/**
	* Set RootMotionGuide to CurrentOverlappedRootMotionGuide.
	*/
	UFUNCTION(BlueprintCallable, Category = "RootMotionMovement")
	void SetCurrentOverlappedRootMotionGuide(ARootMotionGuide* RootMotionGuide);

	/**
	* Whether there is an overlapped root motion guide
	*/
	UFUNCTION(BlueprintPure, Category = "RootMotionMovement")
	bool HasOverlappedRootMotionGuide() const;

	/**
	* Find the playable root motion guide of the TriggerType that matches the MovementDirection. 
	* If the directions are the same, returns the root motion guide of the nearest position.
	*/
	UFUNCTION(BlueprintPure, Category = "RootMotionMovement")
	void FindRootMotionGuide(FVector MovementDirection, ERootMotionGuideTriggerType TriggerType, float MinimumDotValue, bool& bFound, ARootMotionGuide*& RootMotionGuide) const;

	/**
	* Find the root motion guide that is playable in the DelayedAutoPlayRootMotionGuideArray.
	*/
	UFUNCTION(BlueprintPure, Category = "RootMotionMovement")
	void FindRootMotionGuideToAutoPlay(bool& bFound, ARootMotionGuide*& RootMotionGuide) const;

	/**
	* Root motion guide currently playing
	*/
	UPROPERTY(BlueprintReadOnly, Category = "RootMotionMovement")
	TObjectPtr<ARootMotionGuide> CurrentRootMotionGuide;

	/**
	* Currently overlapped root motion guide
	*/
	UPROPERTY(BlueprintReadOnly, Category = "RootMotionMovement")
	TObjectPtr<ARootMotionGuide> CurrentOverlappedRootMotionGuide;

	/**
	* Overlapped root motion guide list
	*/
	UPROPERTY(BlueprintReadOnly, Category = "RootMotionMovement")
	TArray< TObjectPtr<ARootMotionGuide> > OverlappedRootMotionGuideArray;

	/**
	* Root motion guide list with delayed automatic play
	*/
	UPROPERTY(BlueprintReadOnly, Category = "RootMotionMovement")
	TArray< TObjectPtr<ARootMotionGuide> > DelayedAutoPlayRootMotionGuideArray;
private:
	FTransform DefaultMeshTransform;

	FTransform MeshTransformAtRootMotionStart;

	float AnimSequenceTime;
	float RootMotionBlendTime;
	float CurrentMontageTime;
	float WaitTimeForNextRootMotion;

	EMovementMode RootMotionNextMovementMode;

	FTransform TargetTransform;
	UAnimSequence* AnimSequence;
	FTransform CharacterMeshRotation;
	bool bOrientRotationToMovementOrig;
	bool bIgnoreClientMovementErrorChecksAndCorrectionOrig;
	ENetworkSmoothingMode NetworkSmoothingModeOrig;


	void UpdateRootMotionInternal(ACharacter* Character, float DeltaTime);
};
