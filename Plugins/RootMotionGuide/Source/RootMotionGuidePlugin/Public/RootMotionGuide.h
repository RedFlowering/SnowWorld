// Copyright 2017 Lee Ju Sik

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RootMotionGuide.generated.h"

class URootMotionMovement;

/**
* Settings for displaying animation tracks
*/
USTRUCT()
struct FRootMotionGuideExtractBone
{
	GENERATED_BODY()
public:

	/**
	* Name of the bone to display the animation track
	*/
	UPROPERTY(EditAnywhere, Category = "RootMotionGuide")
	FName BoneName;

	/**
	* The color of the arrow to be displayed in the animation track.
	*/
	UPROPERTY(EditAnywhere, Category = "RootMotionGuide")
	FColor ArrowColor = FColor::White;
};

/**
* Animation track data
*/
USTRUCT()
struct FRootMotionGuideAnimationTrack
{
	GENERATED_BODY()
public:

	/**
	* Bone name of animation track
	*/
	UPROPERTY(VisibleAnywhere, Category = "RootMotionGuide")
	FName BoneName;

	/**
	* Animation data for component space
	*/
	UPROPERTY(VisibleAnywhere, Category = "RootMotionGuide")
	TArray< FTransform > ComponentSpaceAnimationTrack;
};

/**
* Trigger type of root motion guide
*/
UENUM()
enum class ERootMotionGuideTriggerType : uint8
{
	/** It plays automatically as soon as it overlaps the trigger. */
	AutoPlay,		

	/** It is played by an action event. */
	ActionEvent,

	/** It is played by the Axis event. */
	AxisEvent,
};


/**
* Displays the movement of the root motion and provides information for playing the root motion animation at the correct location.
*/
UCLASS()
class ROOTMOTIONGUIDEPLUGIN_API ARootMotionGuide : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARootMotionGuide();

protected:
	// Actor's BeginPlay override
	virtual void BeginPlay() override;

	/**
	* Initializes the subtrigger.
	* Copy the information of the main trigger to the subtrigger.
	*/
	virtual void InitializeSubTrigger(const ARootMotionGuide* MainTrigger);

	/**
	* Initializes the Blueprint subtrigger.
	* Copy the information of the main trigger to the subtrigger.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "RootMotionGuide")
	void ReceiveInitializeSubTrigger(const ARootMotionGuide* MainTrigger);

	
private:
	bool bIsSubTrigger;

public:	

	/**
	* AnimSequence for displaying root motion
	*/
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "RootMotionGuide")
	TObjectPtr<class UAnimSequence> AnimSequence;

	UPROPERTY(BlueprintReadOnly, Category = "RootMotionGuide")
	TObjectPtr<class UCapsuleComponent> Capsule;

	UPROPERTY()
	TObjectPtr<class UArrowComponent> Arrow;

	UPROPERTY()
	TObjectPtr<class UArrowComponent> MovementDirectionArrow;

	/**
	* Adjust the position of the trigger.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	FVector TriggerOffset;

	/**
	* Displays capsules and arrows in-game for debugging purposes.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	bool bShowCapsuleInGame;

	/**
	* CapsuleHalfHeight value of the character
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	float CapsuleHalfHeight;

	/**
	* CapsuleRadius value of the character
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	float CapsuleRadius;

	/**
	* Trigger type of root motion guide
	* AutoPlay,		(It plays automatically as soon as it overlaps the trigger.)
	* ActionEvent,	(It is played by an action event.)
	* AxisEvent,	(It is played by the Axis event.)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	ERootMotionGuideTriggerType TriggerType;

	/**
	* If TriggerRange is greater than 0.0, RootMotionLocation and character distance must be closer than TriggerRange to play.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	float TriggerRange;

	/**
	* Prevents play of other root motion guides during NonInterruptibleTime.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	float NonInterruptibleTime;

	/**
	* This root motion guide can play even if another root motion guide is playing.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	bool bCanPlayInPlayingOtherRootMotion;

	/**
	* AnimMontage for root motion to be played by the character
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	class UAnimMontage* AnimMontage;

	/**
	* Time to blend the position of the character mesh when playing the root motion guide.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	float RootMotionBlendTime;

	/**
	* MovementDirection is used to find a root motion guide suitable for user input when the TriggerType is ActionEvent or AxisEvent.
	* MovementDirection is indicated by a green arrow.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	FVector MovementDirection;

	/**
	* MininumDotValue is the minimum Dot value that must be satisfied when examining the MovementDirection.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	float MinimumDotValue;

	/**
	* If the value is not None, to play this root motion guide, another root motion guide must be playing and have PrevRootMotionGuideTag in the actor tag.
	* This value is used to construct the root motion guide chain.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	FName PrevRootMotionGuideTag;

	/**
	* Movement mode to be applied after the Montage animation
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	TEnumAsByte<enum EMovementMode> NextMovementMode;

	/**
	* The offset at which the subtrigger will be placed.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	FVector SubTriggerOffset;

	/**
	* The number of subtriggers.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionTrigger")
	int SubTriggerCount;

	/**
	* The relative rotation value of the character mesh.
	*/
	UPROPERTY(EditAnywhere, Category = "RootMotionGuide")
	FRotator CharacterMeshRotation;


#if	WITH_EDITORONLY_DATA
	
	
	/**
	* Bones to display as animation tracks
	*/
	UPROPERTY(EditAnywhere, Category = "RootMotionGuide")
	TArray< FRootMotionGuideExtractBone > ExtractBoneArray;

	/**
	* Animation track data
	*/
	UPROPERTY()
	TArray< FRootMotionGuideAnimationTrack > AnimationData;

	/**
	* AnimSequence used when extracting AnimationData
	*/
	UPROPERTY()
	TObjectPtr<class UAnimSequence> AnimSequenceForAnimationData;

	/**
	* Arrows for displaying animation tracks
	*/
	UPROPERTY()
	TArray< TObjectPtr<class UArrowComponent>> ArrowArray;

	/**
	* Whether to display arrows
	*/
	UPROPERTY(EditAnywhere, Category = "RootMotionGuide")
	bool bShowArrows;

	/**
	* Frame interval to display arrows
	* If the value is 0, only the first and last are displayed.
	*/
	UPROPERTY(EditAnywhere, Category = "RootMotionGuide", meta = (ClampMin = "0"))
	int ArrowInterval;

	/**
	* Whether to display character snapshots
	*/
	UPROPERTY(EditAnywhere, Category = "RootMotionGuide")
	bool bShowSnapshot;

	/**
	* Whether to display a snapshot of the subtrigger
	* Only the first and last are displayed.
	*/
	UPROPERTY(EditAnywhere, Category = "RootMotionGuide")
	bool bShowSubTriggerSnapshot;

	/**
	* Frame interval to display the character snapshot
	* If the value is 0, only the first and last are displayed.
	*/
	UPROPERTY(EditAnywhere, Category = "RootMotionGuide", meta = (ClampMin = "0"))
	int SnapshotInterval;

	/**
	* Used to show snapshots of a specific frame
	*/
	UPROPERTY(EditAnywhere, Category = "RootMotionGuide")
	TArray< int > SnapshotFrameArray;

	/**
	* Character snapshot array
	*/
	UPROPERTY()
	TArray< TObjectPtr<class USkinnedMeshComponent> > SnapshotArray;

	

#endif

	/**
	* The total movement value of this root motion.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionGuide")
	FVector TotalRootMotion;

	/**
	* Total rotation value of root motion
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionGuide")
	FRotator TotalRootRotation;
	
	/**
	* Enable root motion scale adjustment
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionGuide")	
	bool bEnableAdjustRootMotionScale;

	/**
	* Start time to apply root motion scale adjustment
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionGuide", meta = (editcondition = "bEnableAdjustRootMotionScale"))
	float AdjustRootMotionScaleBeginTime;

	/**
	* Last time of root motion scale adjustment
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionGuide", meta = (editcondition = "bEnableAdjustRootMotionScale"))
	float AdjustRootMotionScaleEndTime;

	/**
	* Root motion scale adjustment is applied smoothly as long as blend time.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionGuide", meta = (editcondition = "bEnableAdjustRootMotionScale", ClampMin = "0.0", UIMin = "0.0"))
	float AdjustRootMotionScaleBlendInTime;

	/**
	* The root motion scale adjustment is smoothly deactivated as long as the blend-out time.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionGuide", meta = (editcondition = "bEnableAdjustRootMotionScale", ClampMin = "0.0", UIMin = "0.0"))
	float AdjustRootMotionScaleBlendOutTime;

	/**
	* The root motion scale to be applied.
	* This applies in the coordinate system of the animation. (The coordinate system visible in the AnimationEditor)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RootMotionGuide", meta = (editcondition = "bEnableAdjustRootMotionScale"))
	FVector AdjustRootMotionScale;

	/**
	* Position value of the extracted root motion. (AdjustRootMotionScaleBeginTime)
	*/
	UPROPERTY()
	FVector AdjustRootMotionScaleBeginLocation;

	/**
	* Position value of the extracted root motion. (AdjustRootMotionScaleBeginTime + AdjustRootMotionScaleBlendInTime)
	*/
	UPROPERTY()
	FVector AdjustRootMotionScaleBlendInLocation;

	/**
	* Position value of the extracted root motion. (AdjustRootMotionScaleEndTime - AdjustRootMotionScaleBlendOutTime)
	*/
	UPROPERTY()
	FVector AdjustRootMotionScaleBlendOutLocation;

	/**
	* Position value of the extracted root motion. (AdjustRootMotionScaleEndTime)
	*/
	UPROPERTY()
	FVector AdjustRootMotionScaleEndLocation;


	/**
	* Where to play AnimMontage
	*/
	UFUNCTION(BlueprintPure, Category = "RootMotionTrigger")
	FVector GetRootMotionLocation() const;

	/**
	* Direction to play AnimMontage
	*/
	UFUNCTION(BlueprintPure, Category = "RootMotionTrigger")
	FVector GetRootMotionDirection() const;

	/**
	* Rotation value to play AnimMontage
	*/
	UFUNCTION(BlueprintPure, Category = "RootMotionTrigger")
	FRotator GetRootMotionRotation() const;

	/**
	* Get AnimMontage to play
	*/
	UFUNCTION(BlueprintPure, Category = "RootMotionTrigger")
	class UAnimMontage* GetRootMotionAnimMontage() const;

	/**
	* Returns the MovementDirection value based on the world.
	*/
	UFUNCTION(BlueprintPure, Category = "RootMotionTrigger")
	FVector GetMovementDirectionInWorld() const;


	/**
	* Getting the Movement mode after the AnimMontage play
	*/
	UFUNCTION(BlueprintPure, Category = "RootMotionTrigger")
	EMovementMode GetRootMotionNextMovementMode() const;

	/**
	* Called when overlapped with character.
	* Used to display information about current root motion during game.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RootMotionTrigger")
	void BeginOverlap(class ACharacter* Character);
	virtual void BeginOverlap_Implementation(class ACharacter* Character);

	/**
	* Called when end of overlap with character.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RootMotionTrigger")
	void EndOverlap(class ACharacter* Character);
	virtual void EndOverlap_Implementation(class ACharacter* Character);

	/**
	* Check whether you can play AnimMontage.
	* Can be overridden in descendant classes
	*/
	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "RootMotionTrigger")
	bool CanPlayRootMotionMontage(class ACharacter* Character, const URootMotionMovement* RootMotionMovement) const;
	virtual bool CanPlayRootMotionMontage_Implementation(class ACharacter* Character, const URootMotionMovement* RootMotionMovement) const;

	/**
	* Called when the root motion guide is played.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "RootMotionTrigger")
	void OnBeginRootMotion(class ACharacter* Character);

	/**
	* Create AnimationTrack and character snapshots
	*/
	virtual void OnConstruction(const FTransform& Transform) override;

	/**
	* Update AnimationTrack
	*/
	void UpdateArrow();

	/**
	* Update character snapshot
	*/
	void UpdateSnapshot();


	/**
	* Apply root motion scale adjustment
	*/
	void UpdateRootMotionScale();
};
