// Copyright 2024 Eren Balatkan. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "MnhHelpers.h"
#include "MnhData.generated.h"

class UMnhTracerComponent;

USTRUCT()
struct FMnhMultiTraceResultContainer
{
	GENERATED_BODY()
	FVector StartLocation = FVector::ZeroVector;
	FVector EndLocation = FVector::ZeroVector;
	FVector Scale = FVector::OneVector;
	FQuat Rotation = FQuat::Identity;
	TArray<FHitResult> HitResults;
};

USTRUCT(BlueprintType)
struct FMnhTracerData
{
	GENERATED_BODY()
	EMnhTraceSource TraceSource;
	FName MeshSocket_1 = NAME_None;
	FName MeshSocket_2 = NAME_None;
	FName SocketOrBoneName;
	FMnhShapeData ShapeData;
	FMnhTraceSettings TraceSettings;
	EMnhTracerTickType TracerTickType;
	float TickInterval = 30;
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType;

	float DeltaTimeLastTick = 0;
	FGuid Guid;

	TWeakObjectPtr<UPrimitiveComponent> SourceComponent;

	int32 OwnerTracerConfigIdx = INDEX_NONE;
	TWeakObjectPtr<UMnhTracerComponent> OwnerTracerComponent;
	EMnhTracerState TracerState = EMnhTracerState::Stopped;

	FCollisionQueryParams CollisionParams;
	FCollisionObjectQueryParams ObjectQueryParams;

	TArray<FMnhMultiTraceResultContainer> SubstepHits;
	TArray<FTransform, TFixedAllocator<2>> TracerTransformsOverTime;
	bool bShouldTickThisFrame = false;
	bool IsPendingRemoval = false;

	void ChangeTracerState(bool bIsTracerActiveArg, bool bStopImmediate=true);
	void DoTrace(const UWorld* World, const uint32 Substeps, const uint32 TickIdx);

	FORCEINLINE FTransform GetCurrentTracerTransform()
	{
		FTransform CurrentTransform;
		if (TraceSource == EMnhTraceSource::PhysicsAsset || TraceSource == EMnhTraceSource::AnimNotify)
		{
			const USkeletalMeshComponent* Source = Cast<USkeletalMeshComponent>(SourceComponent);
			if(!Source)
			{
				const auto EnumText = UEnum::GetDisplayValueAsText(TraceSource).ToString();
				const auto Message = FString::Printf(TEXT("MissNoHit Warning: Initialization Error: Selected Source type is [%s] "
												 "but supplied source component [%s] is not SkeletalMesh"), *EnumText, *SourceComponent.Get()->GetName());
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message);
				Source = nullptr;
				return CurrentTransform;
			}
			const auto BoneTransform = FMnhHelpers::FindBoneTransform(Source, SocketOrBoneName);
			CurrentTransform = FTransform(ShapeData.Orientation, ShapeData.Offset) * BoneTransform;
			CurrentTransform.SetScale3D(FVector::OneVector);
		}
		else if (TraceSource == EMnhTraceSource::MnhShapeComponent || TraceSource == EMnhTraceSource::StaticMeshSockets)
		{
			const auto ComponentCurrentTransform = SourceComponent->GetComponentTransform();
			CurrentTransform = FTransform(ShapeData.Orientation, ShapeData.Offset) * ComponentCurrentTransform;
			CurrentTransform.SetScale3D(FVector::OneVector);
		}
		else if (TraceSource == EMnhTraceSource::SkeletalMeshSockets)
		{
			const USkeletalMeshComponent* Source = Cast<USkeletalMeshComponent>(SourceComponent);
			if(!Source)
			{
				const auto EnumText = UEnum::GetDisplayValueAsText(TraceSource).ToString();
				const auto Message = FString::Printf(TEXT("MissNoHit Warning: Initialization Error: Selected Source type is [%s] "
												 "but supplied source component [%s] is not SkeletalMesh"), *EnumText, *SourceComponent.Get()->GetName());
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message);
				Source = nullptr;
				return CurrentTransform;
			}
			const auto LengthOffset = ShapeData.HalfSize.X;
			const auto Socket1Transform = Source->GetSocketTransform(MeshSocket_1, RTS_World);
			const auto Socket2Transform = Source->GetSocketTransform(MeshSocket_2, RTS_World);
			const auto NewShapeData =
				FMnhHelpers::GetCapsuleShapeDataFromTransforms(Socket1Transform, Socket2Transform, LengthOffset, ShapeData.Radius);
			ShapeData = NewShapeData;
			ShapeData.HalfSize.X = LengthOffset;
			CurrentTransform = FTransform(ShapeData.Orientation, ShapeData.Offset);
			CurrentTransform.SetScale3D({ShapeData.HalfHeight, 1, 1});
			ShapeData.HalfHeight = 1;
		}
		return CurrentTransform;
	}

	FORCEINLINE void UpdatePreviousTransform(const FTransform& Transform)
	{
		if (TracerTransformsOverTime.Num() == 0)
		{
			TracerTransformsOverTime.Add(Transform);
		}
		else
		{
			TracerTransformsOverTime[0] = Transform;
		}
	}
};