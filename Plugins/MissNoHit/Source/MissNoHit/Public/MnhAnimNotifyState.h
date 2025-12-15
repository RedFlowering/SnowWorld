// Copyright 2024 Eren Balatkan. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MnhTracer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Components/SkeletalMeshComponent.h"
#include "Runtime/Launch/Resources/Version.h"
#include "MnhAnimNotifyState.generated.h"

class UMnhTracerComponent;
/**
 * 
 */
UCLASS(Abstract)
class MISSNOHIT_API UMnhAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	FORCEINLINE static UMnhTracerComponent* FindMnhTracerComponent(const USkeletalMeshComponent* MeshComp);
	FORCEINLINE static int FindTracerConfigIdx(const USkeletalMeshComponent* MeshComp, const FGameplayTag TracerTag);
};


UCLASS()
class MISSNOHIT_API UMnhActivateTracer : public UMnhAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit")
	FGameplayTagContainer MnhTracerTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit")
	bool bResetHitCacheOnActivation = true;
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};

UCLASS()
class MISSNOHIT_API UMnhAnimNotifyTracer : public UMnhAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit")
	bool bResetHitCacheOnActivation = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit")
	FGameplayTag TracerTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit")
	FName AttachedSocketOrBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Transient, Category="MissNoHit",
		Meta=(
			WarningMessage="WARNING: Disable this before saving or changes will be lost!",
			DisplayName="Edit Using World Space (WARNING: Disable before saving)"
			))
	bool EditUsingWorldSpace = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ShowOnlyInnerProperties), Category="MissNoHit")
	FMnhShapeData ShapeData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="EditUsingWorldSpace"), Category="MissNoHit")
	FTransform WorldSpaceTransform;

	/* WARNING: Not compatible with dynamic animation logic such as blending, procedural animation, etc.
	 * This is an experimental feature, use with caution and test often!
	 * Computationally expensive compared to other tracers, use only for player character or bosses.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit")
	bool bUsePrecomputedPath = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditConditionHides, EditCondition="bUsePrecomputedPath"))
	int PrecomputeFps = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditConditionHides, EditCondition="bUsePrecomputedPath"))
	bool bDrawPrecomputedPath = false;
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
private:
	int TickIdx = 0;
	float CurrentTimeInNotify = 0;
};

