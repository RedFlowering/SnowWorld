// Copyright 2024 Eren Balatkan, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MissNoHit.h"
#include "MnhHelpers.h"
#include "UObject/Object.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WorldCollision.h"
#include "MnhTracer.generated.h"

struct FMnhTracerData;
class UMnhTraceType;
class UMnhTracerComponent;
struct FGameplayTag;
struct FMnhTraceSettings;
class UMnhHitFilter;

USTRUCT(BlueprintType)
struct FMnhTracerConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit")
	FGameplayTag TracerTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MissNoHit")
	EMnhTraceSource TraceSource = EMnhTraceSource::MnhShapeComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="(TraceSource==EMnhTraceSource::StaticMeshSockets||TraceSource==EMnhTraceSource::SkeletalMeshSockets)", EditConditionHides))
	FName MeshSocket_1 = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="(TraceSource==EMnhTraceSource::StaticMeshSockets||TraceSource==EMnhTraceSource::SkeletalMeshSockets)", EditConditionHides))
	FName MeshSocket_2 = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="(TraceSource==EMnhTraceSource::StaticMeshSockets||TraceSource==EMnhTraceSource::SkeletalMeshSockets)", EditConditionHides))
	float MeshSocketTracerRadius = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="(TraceSource==EMnhTraceSource::StaticMeshSockets||TraceSource==EMnhTraceSource::SkeletalMeshSockets)", EditConditionHides))
	float MeshSocketTracerLengthOffset = 0;

	/* Name of the Socket or Bone Tracer will be attached to */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="(TraceSource==EMnhTraceSource::SocketOrBone)||(TraceSource==EMnhTraceSource::PhysicsAsset)", EditConditionHides))
	FName SocketOrBoneName;

	/* Shape specifications of the Tracer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="false", EditConditionHides))
	FMnhShapeData ShapeData;

	/* Collision settings for the Tracer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit")
	FMnhTraceSettings TraceSettings;

	/* Specifies how the tracer should tick. Sub-stepping is disabled on Match Game Tick */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit")
	EMnhTracerTickType TracerTickType = EMnhTracerTickType::MatchGameTick;

	/* Tick Rate of the Tracer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="TracerTickType==EMnhTracerTickType::FixedRateTick", EditConditionHides))
	int TargetFps = 30;

	/* Distance traveled by Tracer between each Tick */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit",
		meta=(EditCondition="TracerTickType==EMnhTracerTickType::DistanceTick", EditConditionHides))
	int TickDistanceTraveled = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit|Debug")
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit|Debug",
		meta=(EditCondition="DrawDebugType!=EDrawDebugTrace::None", EditConditionHides))
	FColor DebugTraceColor = FColor::Red;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit|Debug",
		meta=(EditCondition="DrawDebugType!=EDrawDebugTrace::None", EditConditionHides))
	FColor DebugTraceHitColor = FColor::Green;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit|Debug",
			meta=(EditCondition="DrawDebugType!=EDrawDebugTrace::None", EditConditionHides))
	FColor DebugTraceBlockColor = FColor::Blue; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit|Debug",
		meta=(EditCondition="DrawDebugType==EDrawDebugTrace::ForDuration", EditConditionHides))
	float DebugDrawTime = 0.5;
	
	int32 TracerDataIdx = INDEX_NONE;
	FGuid TracerDataGuid;
	bool bIsTracerActive;
	
	TObjectPtr<UPrimitiveComponent> SourceComponent;
	int OwnerTracerConfigIdx;
	TObjectPtr<UMnhTracerComponent> OwnerTracerComponent;

	FCollisionQueryParams CollisionParams;
	FCollisionObjectQueryParams ObjectQueryParams;
	
	void InitializeParameters(UPrimitiveComponent* SourceComponentArg);
	void InitializeFromPhysicAsset();
	void InitializeFromCollisionShape();
	void InitializeFromStaticMeshSockets();
	void InitializeFromSkeletalMeshSockets();

	void ChangeTracerState(const UWorld* World, bool bIsTracerActiveArg, bool bStopImmediate=true);
	bool IsTracerActive(const UWorld* World) const;
	void RegisterTracerData(const UWorld* World);
	void UpdateTracerData(const UWorld* World) const;
	void MarkTracerDataForRemoval(const UWorld* World) const;

private:
	FMnhTracerData* GetTracerData(const UWorld* World) const;
};
