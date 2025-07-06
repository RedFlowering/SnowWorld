// Copyright 2025 Snow Game Studio.

#pragma once 

#include "AlsLinkedAnimationInstance.h"
#include "QAAnimationInstance.generated.h"

class ABaseCharacter;

UCLASS()
class UQAAnimationInstance : public UAlsLinkedAnimationInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TObjectPtr<ABaseCharacter> OwnerCharacter = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State", Transient)
	FTransform RightHandIK = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State", Transient)
	FTransform LeftHandIK = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State", Transient)
	bool bUseRightHandIK = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State", Transient)
	bool bUseLeftHandIK = false;

public:
	UQAAnimationInstance();

	virtual void NativeInitializeAnimation() override;

	virtual void NativeBeginPlay() override;

	void NativeUpdateAnimation(float DeltaSeconds) override;
};

