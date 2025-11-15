// Copyright 2019-2023 Henry Galimberti. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"

#include "Animation/PoseSnapshot.h"

#include "UEGoreSystemAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class UEGORESYSTEM_API UUEGoreSystemAnimInstance : public UAnimInstance
{
	GENERATED_UCLASS_BODY()

		UUEGoreSystemAnimInstance();

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "GoreSystem")
		struct FPoseSnapshot Detached_Snapshot;
	UPROPERTY(BlueprintReadOnly, Category = "GoreSystem")
		FTimerHandle PhysTimerHandle;

	UPROPERTY()
		FName CollisionProfile;
	UPROPERTY()
		FVector ImpulseOnDetach;
	UPROPERTY()
		FName Rootbone;
	UPROPERTY()
		uint8 StretchFixEnabled : 1;
	UPROPERTY()
		TArray<FName> BonesToHide;

	/** [Internal] This function will enable physics, collisions, visibility and apply impulse once the Snapshot pose has been applied to the model	*/
	UFUNCTION()
		void EnablePhys();
};
