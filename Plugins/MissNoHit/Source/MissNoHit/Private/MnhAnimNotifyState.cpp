// Copyright 2024 Eren Balatkan. All Rights Reserved.

#include "MnhAnimNotifyState.h"

#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "MnhTracer.h"
#include "MnhTracerComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Animation/Skeleton.h"


UMnhTracerComponent* UMnhAnimNotifyState::FindMnhTracerComponent(const USkeletalMeshComponent* MeshComp)
{
	if (const auto OwnerActor = MeshComp->GetOwner())
	{
		return OwnerActor->FindComponentByClass<UMnhTracerComponent>();
	}
	return nullptr;
}

int UMnhAnimNotifyState::FindTracerConfigIdx(const USkeletalMeshComponent* MeshComp, const FGameplayTag TracerTag)
{
	if (const auto TracerComp = FindMnhTracerComponent(MeshComp))
	{
		return TracerComp->FindTracerConfig(TracerTag);
	}
	return -1;
}

void UMnhActivateTracer::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                     const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (const auto TracerComp = FindMnhTracerComponent(MeshComp))
	{
		TracerComp->StartTracers(MnhTracerTags, bResetHitCacheOnActivation);
	}
}

void UMnhActivateTracer::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
}

void UMnhActivateTracer::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (const auto TracerComp = FindMnhTracerComponent(MeshComp))
	{
		TracerComp->StopTracersDelayed(MnhTracerTags);
	}
}

void UMnhAnimNotifyTracer::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                       const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (const auto TracerComp = FindMnhTracerComponent(MeshComp))
	{
		const auto TracerConfigIdx = TracerComp->FindTracerConfig(TracerTag);
		if (TracerConfigIdx != -1)
		{
			auto& TracerConfig = TracerComp->TracerConfigs[TracerConfigIdx];
			if (TracerConfig.TraceSource != EMnhTraceSource::AnimNotify)
			{
				const FString DebugMessage = FString::Printf(TEXT("MissNoHit Warning: "
														   "AnimNotifyTracer must have matching AnimNotify Tracer Setup with Tag [%s]"
														   " on Owner's [%s] MnhTracerComponent"), *TracerTag.ToString(), *MeshComp->GetOwner()->GetName());
				FMnhHelpers::Mnh_Log(DebugMessage);
				return;
			}
			
			if (!bUsePrecomputedPath)
			{
				TracerConfig.SocketOrBoneName = AttachedSocketOrBoneName;
				TracerConfig.ShapeData = ShapeData;
				TracerConfig.SourceComponent = MeshComp;
				TracerConfig.UpdateTracerData(MeshComp->GetWorld());
				TracerComp->StartTracersInternal(FGameplayTagContainer{TracerTag}, bResetHitCacheOnActivation, true);
			}
			else
			{
				TracerConfig.bIsTracerActive = true;
				TickIdx = 0;
				if (bResetHitCacheOnActivation)
				{
					TracerComp->ResetHitCache();
				}
				CurrentTimeInNotify = 0;
			}
		}
		else
		{
			const FString DebugMessage = FString::Printf(TEXT("MissNoHit Warning: "
														   "No Anim Notify Tracer with with Tag [%s] found"
														   " on Owner's [%s] MnhTracerComponent"), *TracerTag.ToString(), *MeshComp->GetOwner()->GetName());
			FMnhHelpers::Mnh_Log(DebugMessage);
		}
	}
}

UAnimSequence* ExtractAnimSequenceFromMontage(const UAnimSequenceBase* AnimSequenceBase, double& Time)
{
	const UAnimMontage* AnimMontage = Cast<UAnimMontage>(AnimSequenceBase);
	if (!AnimMontage)
	{
		return nullptr;
	}

	for (const auto& Slot : AnimMontage->SlotAnimTracks)
	{
		const auto AnimSegment = Slot.AnimTrack.GetSegmentAtTime(FMath::Clamp(Time, 0.0, AnimSequenceBase->GetPlayLength()));
		if (!AnimSegment)
		{
			continue;
		}

		if (UAnimSequence* AnimSequence = Cast<UAnimSequence>(AnimSegment->GetAnimReference()))
		{
			Time -= AnimSegment->AnimStartTime;
			return AnimSequence;
		}
	}

	return nullptr;
}

FTransform GetBoneTransformFromSequence(const USkeletalMeshComponent* MeshComponent, const UAnimSequenceBase* AnimSequenceBase, const FName TargetBoneName, double Time)
{
	if (!IsValid(MeshComponent) || !IsValid(AnimSequenceBase) || !MeshComponent->GetSkinnedAsset())
	{
		return FTransform::Identity;
	}

	const UAnimSequence* AnimSequence = Cast<UAnimSequence>(AnimSequenceBase);
	if (!AnimSequence)
	{
		AnimSequence = ExtractAnimSequenceFromMontage(AnimSequenceBase, Time);
	}

	if (!AnimSequence)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to retrieve Animation Sequence. Precompute Path is only supported on Animation Sequences"));
		return FTransform::Identity;
	}

	const USkeleton* Skeleton = AnimSequence->GetSkeleton();
	if (!Skeleton)
	{
		return FTransform::Identity;
	}

	FName BoneName = TargetBoneName;
	FTransform GlobalTransform = FTransform::Identity;

	while (BoneName != NAME_None)
	{
		const int32 BoneIndex = Skeleton->GetReferenceSkeleton().FindBoneIndex(BoneName);
		if (BoneIndex == INDEX_NONE)
		{
			break;
		}

		FTransform BoneTransform;
		AnimSequence->GetBoneTransform(BoneTransform, FSkeletonPoseBoneIndex(BoneIndex), Time, false);

		GlobalTransform *= BoneTransform;
        
		const int32 ParentIndex = Skeleton->GetReferenceSkeleton().GetParentIndex(BoneIndex);
		BoneName = (ParentIndex != INDEX_NONE) ? Skeleton->GetReferenceSkeleton().GetBoneName(ParentIndex) : NAME_None;
	}

	return GlobalTransform * MeshComponent->GetComponentTransform();
}

void UMnhAnimNotifyTracer::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if (GEngine->IsEditor() && !GIsPlayInEditorWorld){
		const auto BoneTransform = FMnhHelpers::FindBoneTransform(MeshComp, AttachedSocketOrBoneName);
		
		auto CurrentTransform = FTransform(ShapeData.Orientation, ShapeData.Offset) * BoneTransform;
		if (EditUsingWorldSpace)
		{
			FTransform LocalTransform = WorldSpaceTransform * BoneTransform.Inverse();
			ShapeData.Offset = LocalTransform.GetLocation();
			ShapeData.Orientation = LocalTransform.Rotator();
		}
		else
		{
			WorldSpaceTransform = CurrentTransform;
		}
		
		FMnhHelpers::DrawTrace(CurrentTransform.GetLocation(), CurrentTransform.GetLocation(), FVector::OneVector, CurrentTransform.GetRotation(), FColor::Red, ShapeData,
		MeshComp->GetWorld(), EDrawDebugTrace::ForDuration, FrameDeltaTime * 2);

		if (bDrawPrecomputedPath && bUsePrecomputedPath) 
		{
			auto StartTime = EventReference.GetNotify()->GetTriggerTime();
			const auto NotifyDuration = EventReference.GetNotify()->GetDuration();
			const auto NotifyFrameCount = FMath::CeilToInt(NotifyDuration * PrecomputeFps);

			for (int NotifyFrameIdx = 0; NotifyFrameIdx < NotifyFrameCount-1; NotifyFrameIdx++)
			{
				FTransform CurrentBonePose = GetBoneTransformFromSequence(MeshComp, Animation, AttachedSocketOrBoneName, StartTime + NotifyDuration * NotifyFrameIdx/NotifyFrameCount);
				FTransform NextBonePose = GetBoneTransformFromSequence(MeshComp, Animation, AttachedSocketOrBoneName, StartTime + NotifyDuration * (NotifyFrameIdx + 1)/NotifyFrameCount);;
			
				auto CurrentPoseTransform =
					FTransform(ShapeData.Orientation, ShapeData.Offset) *
					CurrentBonePose;

				auto NextPoseTransform =
					FTransform(ShapeData.Orientation, ShapeData.Offset) *
					NextBonePose;

				const auto& AverageTransform = UKismetMathLibrary::TLerp(CurrentPoseTransform, NextPoseTransform, 0.5, ELerpInterpolationMode::DualQuatInterp);
			
				FMnhHelpers::DrawTrace(CurrentPoseTransform.GetLocation(), NextPoseTransform.GetLocation(), FVector::OneVector,
					AverageTransform.GetRotation(), FColor::Purple, ShapeData,
					MeshComp->GetWorld(), EDrawDebugTrace::ForDuration, 2 * FrameDeltaTime);
			}
		}
		return;
	}
	
	// - - - We are in game mode - - -
	if (!bUsePrecomputedPath)
	{
		return;
	}

	CurrentTimeInNotify += FrameDeltaTime;

	const auto TracerConfigIdx = FindTracerConfigIdx(MeshComp, TracerTag);
	
	if(TracerConfigIdx != -1)
	{
		auto& TracerConfig = FindMnhTracerComponent(MeshComp)->TracerConfigs[TracerConfigIdx];
		TickIdx ++;
		
		const auto NotifyFrameCount = FMath::CeilToInt(FrameDeltaTime * PrecomputeFps);
		const auto CurrentAnimTime = CurrentTimeInNotify + EventReference.GetNotify()->GetTriggerTime();
	
		for (int PoseIdx = 0; PoseIdx < NotifyFrameCount; PoseIdx++)
		{
			if (TracerConfig.bIsTracerActive == false)
			{
				break;
			}
			
			FTransform CurrentBonePose = GetBoneTransformFromSequence(MeshComp, Animation, AttachedSocketOrBoneName, (CurrentAnimTime - FrameDeltaTime) + FrameDeltaTime * PoseIdx/NotifyFrameCount);
			FTransform NextBonePose = GetBoneTransformFromSequence(MeshComp, Animation, AttachedSocketOrBoneName, (CurrentAnimTime - FrameDeltaTime) + FrameDeltaTime * (PoseIdx+1)/NotifyFrameCount);
			
			const auto CurrentPoseTransform =
				FTransform(ShapeData.Orientation, ShapeData.Offset) *
				CurrentBonePose;
	
			const auto NextPoseTransform =
				FTransform(ShapeData.Orientation, ShapeData.Offset) *
				NextBonePose;
	
			auto AverageTransform = UKismetMathLibrary::TLerp(CurrentPoseTransform, NextPoseTransform,
				0.5, ELerpInterpolationMode::DualQuatInterp);
			AverageTransform.SetScale3D(FVector::OneVector);
	
			TArray<FHitResult> OutHits;
			FMnhHelpers::PerformTrace(
				CurrentPoseTransform, NextPoseTransform, AverageTransform,
				OutHits,
				MeshComp->GetWorld(),
				TracerConfig.TraceSettings,
				ShapeData,
				TracerConfig.CollisionParams, FCollisionResponseParams(), TracerConfig.ObjectQueryParams
			);
			
			if (TracerConfig.DrawDebugType != EDrawDebugTrace::None)
			{
				FMnhHelpers::DrawDebug(
					CurrentPoseTransform.GetLocation(),
					NextPoseTransform.GetLocation(),
					FVector::OneVector,
					AverageTransform.GetRotation(),
					OutHits,
					ShapeData,
					MeshComp->GetWorld(),
					TracerConfig.DrawDebugType,
					TracerConfig.DebugDrawTime,
					TracerConfig.DebugTraceColor,
					TracerConfig.DebugTraceBlockColor,
					TracerConfig.DebugTraceHitColor
				);
			}
	
			if (const auto TracerComp = FindMnhTracerComponent(MeshComp))
			{
				TracerComp->OnTracerHitDetected(TracerTag, OutHits, FrameDeltaTime / NotifyFrameCount, TickIdx, MeshComp);
			}
		}
	}
}

void UMnhAnimNotifyTracer::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (bUsePrecomputedPath)
	{
		return;
	}
	
	if (const auto TracerComp = FindMnhTracerComponent(MeshComp))
	{
		TracerComp->StopTracersDelayed(FGameplayTagContainer(TracerTag));
	}
}
