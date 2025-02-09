// Copyright 2025 RedFlowering.

#pragma once 

#include "AlsLinkedAnimationInstance.h"
#include "RFAnimationInstance.generated.h"

class ARFCharacter;

UCLASS()
class URFAnimationInstance : public UAlsLinkedAnimationInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TObjectPtr<ARFCharacter> OwnerCharacter = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State", Transient)
	FTransform RightHandIK = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State", Transient)
	FTransform LeftHandIK = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State", Transient)
	bool bUseRightHandIK = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State", Transient)
	bool bUseLeftHandIK = false;

public:
	URFAnimationInstance();

	virtual void NativeInitializeAnimation() override;

	virtual void NativeBeginPlay() override;

	void NativeUpdateAnimation(float DeltaSeconds) override;
};

