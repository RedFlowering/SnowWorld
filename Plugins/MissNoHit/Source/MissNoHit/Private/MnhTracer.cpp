// Copyright 2024 Eren Balatkan. All Rights Reserved.

#include "MnhTracer.h"

#include "MissNoHit.h"
#include "DrawDebugHelpers.h"
#include "MnhComponents.h"
#include "MnhData.h"
#include "MnhHelpers.h"
#include "MnhSubsystem.h"
#include "MnhTracerComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicsEngine/BodySetup.h"

class FMissNoHitModule;

void FMnhTracerConfig::InitializeParameters(UPrimitiveComponent* SourceComponentArg)
{
	if (!SourceComponentArg)
	{
		const auto Message = FString::Printf(TEXT("MissNoHit Warning: Initialization Error: Source Component is null!"));
		FMnhHelpers::Mnh_Log(Message);
		return;
	}
	
	this->SourceComponent = SourceComponentArg;

	CollisionParams.AddIgnoredActor(SourceComponent->GetOwner());
	CollisionParams.AddIgnoredActor(OwnerTracerComponent->GetOwner());
	CollisionParams.bReturnPhysicalMaterial = true;
	CollisionParams.bTraceComplex = TraceSettings.bTraceComplex;
	
	for (const auto& ObjectType : TraceSettings.ObjectTypes)
	{
		ObjectQueryParams.AddObjectTypesToQuery(ObjectType);
	}
	switch (TraceSource)
	{
	case EMnhTraceSource::PhysicsAsset:
		InitializeFromPhysicAsset();
		break;
	case EMnhTraceSource::MnhShapeComponent:
		InitializeFromCollisionShape();
		break;
	case EMnhTraceSource::StaticMeshSockets:
		InitializeFromStaticMeshSockets();
		break;
	case EMnhTraceSource::SkeletalMeshSockets:
		InitializeFromSkeletalMeshSockets();
		break;
	default:
		break;
	}
	
	UpdateTracerData(SourceComponentArg->GetWorld());
}

void FMnhTracerConfig::InitializeFromPhysicAsset()
{
	const USkeletalMeshComponent* Source = Cast<USkeletalMeshComponent>(SourceComponent);
	if(!Source)
	{
		const auto Message = FString::Printf(TEXT("MissNoHit Warning: Initialization Error: Selected Source type is PhysicAsset but supplied source component type is not SkeletalMesh: %s"), *SourceComponent.GetName());
		FMnhHelpers::Mnh_Log(Message);
		SourceComponent = nullptr;
		return;
	}

	const auto BodyInstance = Source->GetBodyInstance(SocketOrBoneName);
	if (!BodyInstance)
	{
		const auto Message = FString::Printf(TEXT("MissNoHit Warning: Initialization Error: No BodyInstance found with Bone Name [%s] on Component [%s]. "
									  "Please use Mnh Shape Component Attached to Bone Instead"), *SocketOrBoneName.ToString(), *SourceComponent.GetName());
		FMnhHelpers::Mnh_Log(Message);
		
		SourceComponent = nullptr;
		return;
	}
	
	const auto Geom = BodyInstance->GetBodySetup()->AggGeom;
	for(const auto& Sphere : Geom.SphereElems)
	{
		this->ShapeData.TraceShape = EMnhTraceShape::Sphere;
		this->ShapeData.Offset = Sphere.Center;
		this->ShapeData.Radius = Sphere.Radius;
		return;
	}
	for (const auto& Box : Geom.BoxElems)
	{
		this->ShapeData.TraceShape = EMnhTraceShape::Box;
		this->ShapeData.Offset = Box.Center;
		this->ShapeData.HalfSize = {Box.X/2, Box.Y/2, Box.Z/2};
		this->ShapeData.Orientation = Box.Rotation;
		return;
	}
	for (const auto& Capsule : Geom.SphylElems)
	{
		this->ShapeData.TraceShape = EMnhTraceShape::Capsule;
		this->ShapeData.Offset = Capsule.Center;
		this->ShapeData.HalfHeight = Capsule.GetScaledHalfLength(FVector::OneVector);
		this->ShapeData.Orientation = Capsule.Rotation;
		return;
	}
	const auto Message = FString::Printf(TEXT("MissNoHit Warning: No compatible geometry was found with Bone Name [%s] on Component [%s]"), *SocketOrBoneName.ToString(), *SourceComponent.GetName());
	FMnhHelpers::Mnh_Log(Message);
}

void FMnhTracerConfig::InitializeFromCollisionShape()
{
	if (const auto SphereCollision = Cast<UMnhSphereComponent>(SourceComponent))
	{
		this->ShapeData.TraceShape = EMnhTraceShape::Sphere;
		this->ShapeData.Radius = SphereCollision->GetScaledSphereRadius();
		return;
	}
	else if (const auto BoxCollision = Cast<UMnhBoxComponent>(SourceComponent))
	{
		this->ShapeData.TraceShape = EMnhTraceShape::Box;
		this->ShapeData.HalfSize = BoxCollision->GetScaledBoxExtent();
		return;
	}
	else if (const auto CapsuleCollision = Cast<UMnhCapsuleComponent>(SourceComponent))
	{
		this->ShapeData.TraceShape = EMnhTraceShape::Capsule;
		this->ShapeData.HalfHeight = CapsuleCollision->GetScaledCapsuleHalfHeight();
		this->ShapeData.Radius = CapsuleCollision->GetScaledCapsuleRadius();
		return;
	}
	const auto Message = FString::Printf(TEXT("MissNoHit Warning: Initialization of Tracer Source unsuccessful! "
										   "Mnh Component Tracer expects Sphere, Box or Capsule component as input, not [%s] on Actor [%s]"),
										   *SourceComponent.GetName(),
										   *SourceComponent->GetOuter()->GetName());
	this->SourceComponent = nullptr;
	FMnhHelpers::Mnh_Log(Message);
}

void FMnhTracerConfig::InitializeFromStaticMeshSockets()
{
	if (const auto StaticMeshSource = Cast<UStaticMeshComponent>(SourceComponent))
	{
		const auto Socket1Transform = StaticMeshSource->GetSocketTransform(MeshSocket_1, RTS_Component);
		const auto Socket2Transform = StaticMeshSource->GetSocketTransform(MeshSocket_2, RTS_Component);
		this->ShapeData = FMnhHelpers::GetCapsuleShapeDataFromTransforms(Socket1Transform, Socket2Transform, MeshSocketTracerLengthOffset, MeshSocketTracerRadius);
	}
	else
	{
		const auto Message = FString::Printf(TEXT("MissNoHit Warning: Initialization of Tracer Source unsuccessful! "
										   "Supplied Source is not Static Mesh Component [%s] on Actor [%s]"),
										   *SourceComponent.GetName(),
										   *SourceComponent->GetOuter()->GetName());
		FMnhHelpers::Mnh_Log(Message);
		SourceComponent = nullptr;
	}
}

void FMnhTracerConfig::InitializeFromSkeletalMeshSockets()
{
	if(const auto SkeletalMeshSource= Cast<USkeletalMeshComponent>(SourceComponent))
	{
		const auto Socket1Transform = SkeletalMeshSource->GetSocketTransform(MeshSocket_1, RTS_Component);
		const auto Socket2Transform = SkeletalMeshSource->GetSocketTransform(MeshSocket_2, RTS_Component);
		this->ShapeData = FMnhHelpers::GetCapsuleShapeDataFromTransforms(
			Socket1Transform, Socket2Transform, MeshSocketTracerLengthOffset, MeshSocketTracerRadius);
		this->ShapeData.HalfSize.X = MeshSocketTracerLengthOffset;
	}
	else
	{
		const auto Message = FString::Printf(TEXT("MissNoHit Warning: Initialization of Tracer Source unsuccessful! "
										   "Supplied Source is not Skeletal Mesh Component [%s] on Actor [%s]"),
										   *SourceComponent.GetName(),
										   *SourceComponent->GetOuter()->GetName());
		FMnhHelpers::Mnh_Log(Message);
		SourceComponent = nullptr;
	}
}

void FMnhTracerConfig::ChangeTracerState(const UWorld* World, bool bIsTracerActiveArg, bool bStopImmediate)
{
	bIsTracerActive = bIsTracerActiveArg;
	if (const auto TracerData = GetTracerData(World))
	{
		TracerData->ChangeTracerState(bIsTracerActiveArg, bStopImmediate);
	}
}

bool FMnhTracerConfig::IsTracerActive(const UWorld* World) const
{
	if (TracerDataIdx == -1)
	{
		return false;
	}
	
	return GetTracerData(World)->TracerState != EMnhTracerState::Stopped;
}

void FMnhTracerConfig::RegisterTracerData(const UWorld* World)
{
	SCOPE_CYCLE_COUNTER(STAT_MnhAddTracer)
	const auto MnhSubsystem = UMnhSubsystem::GetMnhSubsystem(World);
	TracerDataIdx = MnhSubsystem->RequestNewTracerData();
	TracerDataGuid = FGuid::NewGuid();
	UpdateTracerData(World);
}

void FMnhTracerConfig::UpdateTracerData(const UWorld* World) const
{
	float TickInterval = 0;
	if (TracerTickType == EMnhTracerTickType::DistanceTick)
	{
		TickInterval = TickDistanceTraveled;
	}
	else if (TracerTickType == EMnhTracerTickType::FixedRateTick)
	{
		TickInterval = 1.0f/float(TargetFps);
	}

	const auto MnhSubsystem = UMnhSubsystem::GetMnhSubsystem(World);
	auto& TracerData = *MnhSubsystem->GetTracerDataAt(TracerDataIdx);
	TracerData.OwnerTracerConfigIdx = OwnerTracerConfigIdx;
	TracerData.Guid = TracerDataGuid;
	TracerData.TraceSource = TraceSource;
	TracerData.ShapeData = ShapeData;
	TracerData.SocketOrBoneName = SocketOrBoneName;
	TracerData.MeshSocket_1 = MeshSocket_1;
	TracerData.MeshSocket_2 = MeshSocket_2;
	TracerData.TraceSettings = TraceSettings;
	TracerData.SourceComponent = SourceComponent;
	TracerData.OwnerTracerComponent = OwnerTracerComponent;
	TracerData.DeltaTimeLastTick = 0;
	TracerData.TracerTickType = TracerTickType;
	TracerData.TickInterval = TickInterval;
	TracerData.bShouldTickThisFrame = false;
	TracerData.TracerState = EMnhTracerState::Stopped;
	TracerData.CollisionParams = CollisionParams;
	TracerData.ObjectQueryParams = ObjectQueryParams;
}

void FMnhTracerConfig::MarkTracerDataForRemoval(const UWorld* World) const
{
	if (const auto MnhSubsystem = UMnhSubsystem::GetMnhSubsystem(World))
	{
		MnhSubsystem->MarkTracerDataForRemoval(TracerDataIdx, TracerDataGuid);
	}
}

FMnhTracerData* FMnhTracerConfig::GetTracerData(const UWorld* World) const
{
	const auto MnhSubsystem = UMnhSubsystem::GetMnhSubsystem(World);
	const auto TracerDataPtr = MnhSubsystem->GetTracerDataAt(TracerDataIdx);

	if (TracerDataPtr)
	{
		if (TracerDataPtr->Guid != TracerDataGuid)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Red, "Retrieved TracerData Guid does not match Tracer Guid!");
		}
	}

	return TracerDataPtr;
}