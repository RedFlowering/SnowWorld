// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotifyState_VisualMeshNiagaraEffect.h"
#include "CosmeticComponent.h"
#include "CosmeticActor.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AnimNotifyState_VisualMeshNiagaraEffect)

UAnimNotifyState_VisualMeshNiagaraEffect::UAnimNotifyState_VisualMeshNiagaraEffect(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Template = nullptr;
	LocationOffset.Set(0.0f, 0.0f, 0.0f);
	RotationOffset = FRotator(0.0f, 0.0f, 0.0f);
}

UFXSystemComponent* UAnimNotifyState_VisualMeshNiagaraEffect::SpawnEffect(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) const
{
	USkeletalMeshComponent* VisualMesh = FindVisualMesh(MeshComp);

	// Only spawn if we've got valid params
	if (ValidateParameters(VisualMesh))
	{
		FFXSystemSpawnParameters SpawnParams;
		SpawnParams.SystemTemplate		= Template;
		SpawnParams.AttachToComponent	= VisualMesh;
		SpawnParams.AttachPointName		= SocketName;
		SpawnParams.Location			= LocationOffset;
		SpawnParams.Rotation			= RotationOffset;
		SpawnParams.Scale				= Scale;
		SpawnParams.LocationType		= EAttachLocation::KeepRelativeOffset;
		SpawnParams.bAutoDestroy		= !bDestroyAtEnd;

		if (UNiagaraComponent* NewComponent = UNiagaraFunctionLibrary::SpawnSystemAttachedWithParams(SpawnParams))
		{
			if (bApplyRateScaleAsTimeDilation)
			{
				NewComponent->SetCustomTimeDilation(Animation->RateScale);
			}
			return NewComponent;
		}
	}
	return nullptr;
}

UFXSystemComponent* UAnimNotifyState_VisualMeshNiagaraEffect::GetSpawnedEffect(UMeshComponent* MeshComp)
{
	USkeletalMeshComponent* VisualMesh = FindVisualMesh(Cast<USkeletalMeshComponent>(MeshComp));

	if (VisualMesh)
	{
		TArray<USceneComponent*> Children;
		VisualMesh->GetChildrenComponents(false, Children);

		if (Children.Num())
		{
			for (USceneComponent* Component : Children)
			{
				if (Component && Component->ComponentHasTag(GetSpawnedComponentTag()))
				{
					if (UFXSystemComponent* FXComponent = CastChecked<UFXSystemComponent>(Component))
					{
						return FXComponent;
					}
				}
			}
		}
	}

	return nullptr;
}

void UAnimNotifyState_VisualMeshNiagaraEffect::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
}

void UAnimNotifyState_VisualMeshNiagaraEffect::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (UFXSystemComponent* Component = SpawnEffect(MeshComp, Animation))
	{
		// tag the component with the AnimNotify that is triggering the animation so that we can properly clean it up
		Component->ComponentTags.AddUnique(GetSpawnedComponentTag());
	}

	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
}

void UAnimNotifyState_VisualMeshNiagaraEffect::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
}

void UAnimNotifyState_VisualMeshNiagaraEffect::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (UFXSystemComponent* FXComponent = GetSpawnedEffect(MeshComp))
	{
		// untag the component
		FXComponent->ComponentTags.Remove(GetSpawnedComponentTag());

		// Either destroy the component or deactivate it to have it's active FXSystems finish.
		// The component will auto destroy once all FXSystem are gone.
		if (bDestroyAtEnd)
		{
			FXComponent->DestroyComponent();
		}
		else
		{
			FXComponent->Deactivate();
		}
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}

bool UAnimNotifyState_VisualMeshNiagaraEffect::ValidateParameters(USkeletalMeshComponent* VisualMesh) const
{
	bool bValid = true;

	if (!Template)
	{
		bValid = false;
	}
	else if (!VisualMesh)
	{
		bValid = false;
	}
	else if (!VisualMesh->DoesSocketExist(SocketName) && VisualMesh->GetBoneIndex(SocketName) == INDEX_NONE)
	{
		bValid = false;
	}

	return bValid;
}

FName UAnimNotifyState_VisualMeshNiagaraEffect::GetSpawnedComponentTag() const
{
	// we generate a unique tag to associate with our spawned components so that we can clean things up upon completion
	FName NotifyName = GetFName();
	NotifyName.SetNumber(GetUniqueID());

	return NotifyName;
}

FString UAnimNotifyState_VisualMeshNiagaraEffect::GetNotifyName_Implementation() const
{
	if (Template)
	{
		return Template->GetName();
	}

	return UAnimNotifyState::GetNotifyName_Implementation();
}

USkeletalMeshComponent* UAnimNotifyState_VisualMeshNiagaraEffect::FindVisualMesh(USkeletalMeshComponent* AnimMeshComp) const
{
	if (!AnimMeshComp)
	{
		return nullptr;
	}

	// If using animation mesh directly, return it
	if (bUseAnimationMesh)
	{
		return AnimMeshComp;
	}

	AActor* Owner = AnimMeshComp->GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// Check if Owner is a CosmeticActor - it has GetVisualMesh() directly
	if (ACosmeticActor* CosmeticActor = Cast<ACosmeticActor>(Owner))
	{
		if (USkeletalMeshComponent* VisualMesh = CosmeticActor->GetVisualMesh())
		{
			return VisualMesh;
		}
	}

	// Try to find CosmeticComponent's visual mesh on the Owner
	if (UCosmeticComponent* CosmeticComp = Owner->FindComponentByClass<UCosmeticComponent>())
	{
		if (USkeletalMeshComponent* VisualMesh = CosmeticComp->GetVisualMesh())
		{
			return VisualMesh;
		}
	}

	// If Owner is CosmeticActor, try to find parent character's CosmeticComponent
	if (AActor* ParentActor = Owner->GetAttachParentActor())
	{
		if (UCosmeticComponent* ParentCosmeticComp = ParentActor->FindComponentByClass<UCosmeticComponent>())
		{
			if (USkeletalMeshComponent* VisualMesh = ParentCosmeticComp->GetVisualMesh())
			{
				return VisualMesh;
			}
		}

		// Try parent as character
		if (ACharacter* ParentCharacter = Cast<ACharacter>(ParentActor))
		{
			return ParentCharacter->GetMesh();
		}
	}

	// Fallback to character's own mesh
	if (ACharacter* Character = Cast<ACharacter>(Owner))
	{
		return Character->GetMesh();
	}

	// Last fallback: return animation mesh
	return AnimMeshComp;
}

//////////////////////////////////////////////////////////////////////////

UAnimNotifyState_VisualMeshNiagaraEffectAdvanced::UAnimNotifyState_VisualMeshNiagaraEffectAdvanced(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NotifyProgressUserParameter = FName("NormalizedNotifyProgress");
}

void UAnimNotifyState_VisualMeshNiagaraEffectAdvanced::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
}

void UAnimNotifyState_VisualMeshNiagaraEffectAdvanced::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	FInstanceProgressInfo& NewInfo = ProgressInfoMap.Add(MeshComp);
	NewInfo.Duration = TotalDuration;
	NewInfo.Elapsed = 0.0f;
}

void UAnimNotifyState_VisualMeshNiagaraEffectAdvanced::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
}

void UAnimNotifyState_VisualMeshNiagaraEffectAdvanced::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	ProgressInfoMap.Remove(MeshComp);
}

void UAnimNotifyState_VisualMeshNiagaraEffectAdvanced::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
}

void UAnimNotifyState_VisualMeshNiagaraEffectAdvanced::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	// Advance the progress
	if (FInstanceProgressInfo* ProgressInfo = ProgressInfoMap.Find(MeshComp))
	{
		const float RateScale = bApplyRateScaleToProgress ? FMath::Abs(Animation->RateScale) : 1.0f;
		ProgressInfo->Elapsed += FrameDeltaTime * RateScale;
	}

	USkeletalMeshComponent* VisualMesh = FindVisualMesh(MeshComp);
	if (UFXSystemComponent* FXComponent = GetSpawnedEffect(VisualMesh))
	{
		// Send the notify progress to the FX Component
		if (NotifyProgressType != EVisualMeshNiagaraAnimNotifyProgressType::None && !NotifyProgressUserParameter.IsNone())
		{
			float NotifyProgress = GetNotifyProgress(MeshComp);
			if (NotifyProgressType == EVisualMeshNiagaraAnimNotifyProgressType::Reverse)
			{
				NotifyProgress = 1.0f - NotifyProgress;
			}
			FXComponent->SetFloatParameter(NotifyProgressUserParameter, NotifyProgress);
		}

		// Send anim curve data to the FX Component
		if (AnimCurves.Num() != 0)
		{
			if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
			{
				for (int32 Index = 0; Index != AnimCurves.Num(); ++Index)
				{
					FName CurveName = AnimCurves[Index].AnimCurveName;
					FName NiagaraUserVariableName = AnimCurves[Index].UserVariableName;
					if ((!CurveName.IsNone()) && (!NiagaraUserVariableName.IsNone()))
					{
						float CurveValue = 0.0f;
						if (AnimInst->GetCurveValue(CurveName, CurveValue))
						{
							FXComponent->SetFloatParameter(NiagaraUserVariableName, CurveValue);
						}
					}
				}
			}
			else
			{
				// No anim instance, defer to the mesh component's curves
				for (int32 Index = 0; Index != AnimCurves.Num(); ++Index)
				{
					FName CurveName = AnimCurves[Index].AnimCurveName;
					FName NiagaraUserVariableName = AnimCurves[Index].UserVariableName;
					if ((!CurveName.IsNone()) && (!NiagaraUserVariableName.IsNone()))
					{
						float CurveValue = 0.0f;
						if (MeshComp->GetCurveValue(CurveName, 0.0f, CurveValue))
						{
							FXComponent->SetFloatParameter(NiagaraUserVariableName, CurveValue);
						}
					}
				}
			}
		}
	}
}

float UAnimNotifyState_VisualMeshNiagaraEffectAdvanced::GetNotifyProgress(UMeshComponent* MeshComp)
{
	if (FInstanceProgressInfo* ProgressInfo = ProgressInfoMap.Find(MeshComp))
	{
		return FMath::Clamp(ProgressInfo->Elapsed / FMath::Max(ProgressInfo->Duration, SMALL_NUMBER), 0.0f, 1.0f);
	}
	return 0.0f;
}
