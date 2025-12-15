// Copyright 2024 Eren Balatkan. All Rights Reserved.

#pragma once

#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "CoreMinimal.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/Object.h"
#include "KismetTraceUtils.h"
#include "UnrealEngine.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "MnhHelpers.generated.h"


DECLARE_STATS_GROUP(TEXT("MissNoHit"), STATGROUP_MISSNOHIT, STATCAT_Advanced)
DECLARE_CYCLE_STAT(TEXT("MissNoHit Start Tracer"), STAT_MNHStartTracer, STATGROUP_MISSNOHIT);
DECLARE_CYCLE_STAT(TEXT("MissNoHit Check Filters"), STAT_MNHCheckFilters, STATGROUP_MISSNOHIT);
DECLARE_CYCLE_STAT(TEXT("MissNoHit Add Tracer"), STAT_MnhAddTracer, STATGROUP_MISSNOHIT);
DECLARE_CYCLE_STAT(TEXT("MissNoHit Remove Tracer"), STAT_MnhRemoveTracer, STATGROUP_MISSNOHIT);
DECLARE_CYCLE_STAT(TEXT("MissNoHit Tick Tracers"), STAT_MnhTickTracers, STATGROUP_MISSNOHIT);
DECLARE_CYCLE_STAT(TEXT("MissNoHit Tracer Update Transforms"), STAT_MnhTracerUpdateTransforms, STATGROUP_MISSNOHIT);
DECLARE_CYCLE_STAT(TEXT("MissNoHit Tracer Do Trace"), STAT_MnhTracerDoTrace, STATGROUP_MISSNOHIT);
DECLARE_CYCLE_STAT(TEXT("MissNoHit Tracer Debug Draw"), STAT_MnhTracerDebugDraw, STATGROUP_MISSNOHIT);
DECLARE_CYCLE_STAT(TEXT("MissNoHit Tracer Trace Done"), STAT_MnhTracerTraceDone, STATGROUP_MISSNOHIT);
DECLARE_CYCLE_STAT(TEXT("MissNoHit TracerComponent Hit Detected"), STAT_MnhTracerComponentHitDetected, STATGROUP_MISSNOHIT);
DECLARE_CYCLE_STAT(TEXT("MissNoHit Tracer Control Node Hit Detected "), STAT_MnhTracerHitDetectedAsyncNode, STATGROUP_MISSNOHIT);
DECLARE_CYCLE_STAT(TEXT("MissNoHit Tracer Get Shape"), STAT_MnhGetTracerShape, STATGROUP_MISSNOHIT)

DECLARE_LOG_CATEGORY_EXTERN(LogMnh, Log, All)

UENUM(BlueprintType)
enum class EMnhTraceSource : uint8
{
	MnhShapeComponent 		UMETA(DisplayName = "Mnh Shape Component"),
	PhysicsAsset 			UMETA(DisplayName = "Physics Asset"),
	AnimNotify 				UMETA(DisplayName = "AnimNotify"),
	StaticMeshSockets 		UMETA(DisplayName = "Static Mesh Sockets"),
	SkeletalMeshSockets		UMETA(DisplayName = "Skeletal Mesh Sockets"),
};

UENUM(BlueprintType)
enum class EMnhTraceShape : uint8
{
	Sphere					UMETA(DisplayName = "Sphere"),
	Box						UMETA(DisplayName = "Box"),
	Capsule					UMETA(DisplayName = "Capsule")
};

UENUM(BlueprintType)
enum class EMnhTraceType : uint8
{
	ByChannel				UMETA(DisplayName = "By Channel"),
	ByObject				UMETA(DisplayName = "By Object"),
	ByProfile				UMETA(DisplayName = "By Profile")
};

UENUM(BlueprintType)
enum class EMnhTracerTickType : uint8
{
	MatchGameTick			UMETA(DisplayName = "Match Game Tick"),
	FixedRateTick			UMETA(DisplayName = "Fixed Rate Tick"),
	DistanceTick			UMETA(DisplayName = "Tick by Distance Traveled")
};

UENUM()
enum class EMnhTracerState : uint8
{
	Active					UMETA(DisplayName = "Active"),
	Stopped					UMETA(DisplayName = "Stopped"),
	PendingStop				UMETA(DisplayName = "PendingStop")
};

inline void MnhDrawDebugSweptSphere(const UWorld* InWorld, FVector const& Start, FVector const& End, float Radius, FColor const& Color, bool bPersistentLines, float LifeTime, uint8 DepthPriority)
{
	// DrawDebugSweptSphere(InWorld, Start, End, Radius, Color, bPersistentLines, LifeTime, DepthPriority);
	FVector const TraceVec = End - Start;
	float const Dist = TraceVec.Size();
	
	FVector const Center = Start + TraceVec * 0.5f;
	float const HalfHeight = (Dist * 0.5f) + Radius;
	
	FQuat const CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
	::DrawDebugCapsule(InWorld, Center, HalfHeight, Radius, CapsuleRot, Color, bPersistentLines, LifeTime, DepthPriority);
}

inline void MnhDrawDebugSweptBox(const UWorld* InWorld, FVector const& Start, FVector const& End, FRotator const & Orientation, FVector const & HalfSize, FColor const& Color, bool bPersistentLines, float LifeTime, uint8 DepthPriority)
{
	// DrawDebugSweptBox(InWorld, Start, End, Orientation, HalfSize, Color, bPersistentLines, LifeTime, DepthPriority);
	FVector const TraceVec = End - Start;
	
	FQuat const CapsuleRot = Orientation.Quaternion();
	::DrawDebugBox(InWorld, Start, HalfSize, CapsuleRot, Color, bPersistentLines, LifeTime, DepthPriority);
	
	//now draw lines from vertices
	FVector Vertices[8];
	Vertices[0] = Start + CapsuleRot.RotateVector(FVector(-HalfSize.X, -HalfSize.Y, -HalfSize.Z));	//flt
	Vertices[1] = Start + CapsuleRot.RotateVector(FVector(-HalfSize.X, HalfSize.Y, -HalfSize.Z));	//frt
	Vertices[2] = Start + CapsuleRot.RotateVector(FVector(-HalfSize.X, -HalfSize.Y, HalfSize.Z));	//flb
	Vertices[3] = Start + CapsuleRot.RotateVector(FVector(-HalfSize.X, HalfSize.Y, HalfSize.Z));	//frb
	Vertices[4] = Start + CapsuleRot.RotateVector(FVector(HalfSize.X, -HalfSize.Y, -HalfSize.Z));	//blt
	Vertices[5] = Start + CapsuleRot.RotateVector(FVector(HalfSize.X, HalfSize.Y, -HalfSize.Z));	//brt
	Vertices[6] = Start + CapsuleRot.RotateVector(FVector(HalfSize.X, -HalfSize.Y, HalfSize.Z));	//blb
	Vertices[7] = Start + CapsuleRot.RotateVector(FVector(HalfSize.X, HalfSize.Y, HalfSize.Z));		//brb
	for (int32 VertexIdx = 0; VertexIdx < 8; ++VertexIdx)
	{
		::DrawDebugLine(InWorld, Vertices[VertexIdx], Vertices[VertexIdx] + TraceVec, Color, bPersistentLines, LifeTime, DepthPriority);
	}
	
	DrawDebugBox(InWorld, End, HalfSize, CapsuleRot, Color, bPersistentLines, LifeTime, DepthPriority);
}

USTRUCT(BlueprintType)
struct FMnhShapeData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit")
	EMnhTraceShape TraceShape = EMnhTraceShape::Sphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ShowOnlyInnerProperties), Category="MissNoHit")
	FVector Offset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="TraceShape!=EMnhTraceShape::Box", EditConditionHides, ShowOnlyInnerProperties))
	float Radius = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="TraceShape==EMnhTraceShape::Capsule", EditConditionHides, ShowOnlyInnerProperties))
	float HalfHeight = 10.f;

	// HalfSize's X is also used for storing length offset on Skeletal Mesh Tracer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="TraceShape==EMnhTraceShape::Box", EditConditionHides, ShowOnlyInnerProperties))
	FVector HalfSize = FVector(10.f, 10.f, 10.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="TraceShape!=EMnhTraceShape::Sphere", EditConditionHides, ShowOnlyInnerProperties))
	FRotator Orientation = FRotator::ZeroRotator;

	FORCEINLINE FCollisionShape GetTracerShape(const FVector& Scale = FVector::OneVector) const
	{
		SCOPE_CYCLE_COUNTER(STAT_MnhGetTracerShape)
		switch (TraceShape)
		{
		case EMnhTraceShape::Sphere:
			return FCollisionShape::MakeSphere(Radius);
			break;
		case EMnhTraceShape::Box:
			return FCollisionShape::MakeBox(HalfSize);
			break;
		case EMnhTraceShape::Capsule:
			return FCollisionShape::MakeCapsule(Radius, HalfHeight * Scale.X);
			break;
		default:
			return FCollisionShape::MakeSphere(Radius);
			break;
		}
	}
};

USTRUCT(BlueprintType)
struct FMnhTraceSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit")
	EMnhTraceType TraceType = EMnhTraceType::ByChannel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="TraceType==EMnhTraceType::ByChannel", EditConditionHides))
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="TraceType==EMnhTraceType::ByObject", EditConditionHides))
	TArray<TEnumAsByte<ECollisionChannel>> ObjectTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="TraceType==EMnhTraceType::ByProfile", EditConditionHides))
	FName ProfileName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit")
	bool bTraceComplex=true;
};

USTRUCT()
struct MISSNOHIT_API FMnhHelpers
{
	GENERATED_BODY()

public:
	FORCEINLINE static void Mnh_Log(const FString& Text, const bool OnScreen=true)
	{
#if !UE_BUILD_SHIPPING
		if (OnScreen)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0, FColor::Red, Text);
		}
		UE_LOG(LogMnh, Warning, TEXT("%s"), *Text)
#endif
	}
	
	FORCEINLINE static FTransform FindBoneTransform(const USkeletalMeshComponent* Mesh, FName BoneName)
	{
		const auto BoneIndex = Mesh->GetBoneIndex(BoneName);
		if (BoneIndex == INDEX_NONE)
		{
			return Mesh->GetSocketTransform(BoneName);
		}
		return Mesh->GetBoneTransform(BoneIndex);
	}

	
	FORCEINLINE static void DrawDebug(const FVector& Start, const FVector& End, const FVector& Scale, const FQuat& Rot, TArray<FHitResult> Hits, const FMnhShapeData& ShapeData, const UWorld* World,
	                                  const EDrawDebugTrace::Type DrawDebugType, float DebugDrawTime, const FColor& DebugTraceColor, const FColor& DebugTraceBlockColor, const FColor& DebugTraceHitColor)
	{
		SCOPE_CYCLE_COUNTER(STAT_MnhTracerDebugDraw)
	
		if (Hits.Num() > 0) {
			const FHitResult& FirstHit = Hits[0];
			DrawTrace(Start, FirstHit.Location, Scale, Rot, DebugTraceColor, ShapeData, World, DrawDebugType, DebugDrawTime);

			if (Hits.Last().bBlockingHit)
			{
				if (Hits.Num() == 1)
				{
					DrawTrace(FirstHit.Location, End, Scale, Rot, DebugTraceBlockColor, ShapeData, World, DrawDebugType, DebugDrawTime);
				}
				else
				{
					DrawTrace(FirstHit.Location, Hits.Last().Location, Scale, Rot, DebugTraceHitColor, ShapeData, World, DrawDebugType, DebugDrawTime);
					DrawTrace(Hits.Last().Location, End, Scale, Rot, DebugTraceBlockColor, ShapeData, World, DrawDebugType, DebugDrawTime);
				}
			}
			else {DrawTrace(FirstHit.Location, End, Scale, Rot, DebugTraceHitColor, ShapeData, World, DrawDebugType, DebugDrawTime);}
		} else {DrawTrace(Start, End, Scale, Rot, DebugTraceColor, ShapeData, World, DrawDebugType, DebugDrawTime);}
	
		for (const FHitResult& Hit : Hits){
			DrawDebugPoint(World, Hit.ImpactPoint, 10, Hit.bBlockingHit ? DebugTraceColor : DebugTraceHitColor,
				DrawDebugType == EDrawDebugTrace::Persistent, DebugDrawTime);
		}
	}

	
	FORCEINLINE static void DrawTrace(const FVector& Start, const FVector& End, const FVector& Scale, const FQuat& Rot, const FColor Color, const FMnhShapeData& ShapeData, const UWorld* World,
		const EDrawDebugTrace::Type DrawDebugType, float DebugDrawTime)
	{
		switch (ShapeData.TraceShape)
		{
		case EMnhTraceShape::Sphere:
			{
				MnhDrawDebugSweptSphere(World, Start, End, ShapeData.Radius, Color, DrawDebugType == EDrawDebugTrace::Persistent, DebugDrawTime, 0);
				break;
			}
		case EMnhTraceShape::Box:
			{
				MnhDrawDebugSweptBox(World, Start, End, Rot.Rotator(), ShapeData.HalfSize, Color, DrawDebugType == EDrawDebugTrace::Persistent, DebugDrawTime, 0);
				break;
			}
		case EMnhTraceShape::Capsule:
			{
				DrawDebugCapsule(World, Start, ShapeData.HalfHeight * Scale.X, ShapeData.Radius, Rot, Color, DrawDebugType == EDrawDebugTrace::Persistent, DebugDrawTime);
				DrawDebugCapsule(World, End, ShapeData.HalfHeight * Scale.X, ShapeData.Radius, Rot, Color, DrawDebugType == EDrawDebugTrace::Persistent, DebugDrawTime);
				DrawDebugLine(World, Start, End, Color, DrawDebugType == EDrawDebugTrace::Persistent, DebugDrawTime);
				break;
			}
		default:
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid TraceShape value in UTracer::DrawTrace."));
				break;
			}
		}
	}
	
	FORCEINLINE static void PerformTrace(const FTransform& StartTransform, const FTransform& EndTransform, const FTransform& AverageTransform, TArray<FHitResult>& OutHits, const UWorld* World,
		const FMnhTraceSettings& TraceSettings,
		const FMnhShapeData& ShapeData, const FCollisionQueryParams& CollisionParams,
		const FCollisionResponseParams& CollisionResponseParams,
		const FCollisionObjectQueryParams& ObjectQueryParams)
	{
		switch (TraceSettings.TraceType)
		{
		case EMnhTraceType::ByChannel:
			World->SweepMultiByChannel(
				OutHits,
				StartTransform.GetLocation(),
				EndTransform.GetLocation(),
				AverageTransform.GetRotation(),
				TraceSettings.TraceChannel,
				ShapeData.GetTracerShape(AverageTransform.GetScale3D()),
				CollisionParams,
				CollisionResponseParams);
			break;
		case EMnhTraceType::ByObject:
			World->SweepMultiByObjectType(
				OutHits,
				StartTransform.GetLocation(),
				EndTransform.GetLocation(),
				AverageTransform.GetRotation(),
				ObjectQueryParams,
				ShapeData.GetTracerShape(AverageTransform.GetScale3D()),
				CollisionParams);
			break;
		case EMnhTraceType::ByProfile:
			World->SweepMultiByProfile(
				OutHits,
				StartTransform.GetLocation(),
				EndTransform.GetLocation(),
				AverageTransform.GetRotation(),
				TraceSettings.ProfileName,
				ShapeData.GetTracerShape(AverageTransform.GetScale3D()),
				CollisionParams);
			break;
		}
	}

	FORCEINLINE static FMnhShapeData GetCapsuleShapeDataFromTransforms(const FTransform& Transform1, const FTransform& Transform2, const float LengthOffset, const float Radius)
	{
		const FVector Socket1Location = Transform1.GetLocation();
		const FVector Socket2Location = Transform2.GetLocation();
		const FVector CenterLocation = (Socket1Location + Socket2Location) / 2.f;

		FMnhShapeData ShapeData;
		ShapeData.TraceShape = EMnhTraceShape::Capsule;
		ShapeData.HalfHeight = FMath::Max((Socket1Location - Socket2Location).Length() / 2.f + LengthOffset, 1.f);
		ShapeData.Offset = CenterLocation;
		ShapeData.Orientation = FRotationMatrix::MakeFromZ(Socket1Location - Socket2Location).Rotator();
		ShapeData.Radius = FMath::Max(1.f, Radius);
		return ShapeData;
	}
};
