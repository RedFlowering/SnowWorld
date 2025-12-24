// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_VisualMeshNiagaraEffect.generated.h"

class UNiagaraSystem;
class UFXSystemComponent;

/**
 * UAnimNotifyState_VisualMeshNiagaraEffect
 * Same as TimedNiagaraEffect but attaches to visual mesh
 */
UCLASS(Blueprintable, CollapseCategories, EditInlineNew, meta = (DisplayName = "Visual Mesh Niagara Effect"))
class HARMONIAKIT_API UAnimNotifyState_VisualMeshNiagaraEffect : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_VisualMeshNiagaraEffect(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Spawn the effect
	UFUNCTION(BlueprintCallable, Category = "AnimNotify")
	virtual UFXSystemComponent* SpawnEffect(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) const;

	// Get the spawned effect
	UFUNCTION(BlueprintCallable, Category = "AnimNotify")
	UFXSystemComponent* GetSpawnedEffect(UMeshComponent* MeshComp);

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

	// Niagara System to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (DisplayName = "Niagara System"))
	TObjectPtr<UNiagaraSystem> Template;

	// Socket name on the visual mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify", meta = (AnimNotifyBoneName = "true"))
	FName SocketName = NAME_None;

	// If true, use animation mesh directly
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	bool bUseAnimationMesh = false;

	// Location offset from the socket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FVector LocationOffset;

	// Rotation offset from the socket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FRotator RotationOffset;

	// Scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FVector Scale = FVector(1.f);

	// Should the effect be destroyed immediately when the notify ends?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	bool bDestroyAtEnd = true;

	// Scale the effect's time dilation by animation RateScale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	bool bApplyRateScaleAsTimeDilation = false;

protected:
	bool ValidateParameters(USkeletalMeshComponent* VisualMesh) const;
	FName GetSpawnedComponentTag() const;
	USkeletalMeshComponent* FindVisualMesh(USkeletalMeshComponent* AnimMeshComp) const;
};

//////////////////////////////////////////////////////////////////////////

UENUM()
enum class EVisualMeshNiagaraAnimNotifyProgressType : uint8
{
	None,
	Forward,
	Reverse,
};

USTRUCT(BlueprintType)
struct FVisualMeshNiagaraAnimCurveParameter
{
	GENERATED_BODY()

	// Animation curve name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FName AnimCurveName;

	// Niagara user variable name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FName UserVariableName;
};

/**
 * UAnimNotifyState_VisualMeshNiagaraEffectAdvanced
 * Advanced version with progress tracking
 */
UCLASS(Blueprintable, CollapseCategories, EditInlineNew, meta = (DisplayName = "Visual Mesh Niagara Effect Advanced"))
class HARMONIAKIT_API UAnimNotifyState_VisualMeshNiagaraEffectAdvanced : public UAnimNotifyState_VisualMeshNiagaraEffect
{
	GENERATED_BODY()

public:
	UAnimNotifyState_VisualMeshNiagaraEffectAdvanced(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	// Progress type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	EVisualMeshNiagaraAnimNotifyProgressType NotifyProgressType = EVisualMeshNiagaraAnimNotifyProgressType::Forward;

	// Parameter to send progress to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	FName NotifyProgressUserParameter;

	// Animation curves to send
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	TArray<FVisualMeshNiagaraAnimCurveParameter> AnimCurves;

	// Apply rate scale to progress
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	bool bApplyRateScaleToProgress = true;

protected:
	float GetNotifyProgress(UMeshComponent* MeshComp);

private:
	struct FInstanceProgressInfo
	{
		float Duration = 0.0f;
		float Elapsed = 0.0f;
	};
	TMap<UMeshComponent*, FInstanceProgressInfo> ProgressInfoMap;
};
