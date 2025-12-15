// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HarmoniaSweepTraceComponent.generated.h"

class USkeletalMeshComponent;

/**
 * Shape type for sweep collision
 */
UENUM(BlueprintType)
enum class ESweepShapeType : uint8
{
	Box UMETA(DisplayName = "Box"),
	Sphere UMETA(DisplayName = "Sphere"),
	Capsule UMETA(DisplayName = "Capsule")
};

/**
 * Delegate for sweep trace hit detection
 * Fired when sweep detects collision with actors
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnSweepTraceHitDelegate,
	AActor*, HitActor,
	const FHitResult&, HitResult,
	UHarmoniaSweepTraceComponent*, SweepComponent);

/**
 * UHarmoniaSweepTraceComponent
 *
 * Performs frame-to-frame sweep collision detection for combat hit detection.
 * Attach to Character BP and configure sweep settings.
 *
 * Features:
 * - Socket-based positioning (follows skeletal mesh socket)
 * - Box sweep between previous and current frame positions
 * - Editor preview visualization
 * - Runtime debug draw
 * - Duplicate hit prevention per sweep session
 *
 * Usage:
 * 1. Add component to Character BP
 * 2. Configure SocketName, SweepBoxHalfExtent, ObjectTypes
 * 3. Bind to OnSweepTraceHit delegate
 * 4. Use AnimNotifyState_SweepTrace or call StartSweep/TickSweep/StopSweep manually
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (HarmoniaKit), meta = (BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaSweepTraceComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UHarmoniaSweepTraceComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// ============================================================================
	// Socket Configuration
	// ============================================================================

	/** Name of the socket to attach sweep origin to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Socket")
	FName SocketName = FName("weapon_r");

	/** Local offset from socket location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Socket")
	FVector SocketOffset = FVector::ZeroVector;

	/** Local rotation offset from socket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Socket")
	FRotator SocketRotation = FRotator::ZeroRotator;

	/** Automatically find SkeletalMeshComponent from owner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Socket")
	bool bAutoFindOwnerMesh = true;

	/** Use CosmeticActor's visual mesh instead of Character mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Socket", meta = (EditCondition = "bAutoFindOwnerMesh"))
	bool bUseCosmeticActorMesh = false;

	// ============================================================================
	// Sweep Configuration
	// ============================================================================

	/** Shape type for sweep collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Collision")
	ESweepShapeType SweepShapeType = ESweepShapeType::Box;

	/** Half-extents of the sweep collision box (for Box shape) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Collision", meta = (EditCondition = "SweepShapeType == ESweepShapeType::Box", EditConditionHides))
	FVector SweepBoxHalfExtent = FVector(15.0f, 15.0f, 40.0f);

	/** Radius for sphere/capsule sweep */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Collision", meta = (EditCondition = "SweepShapeType != ESweepShapeType::Box", EditConditionHides))
	float SweepRadius = 15.0f;

	/** Half height for capsule sweep */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Collision", meta = (EditCondition = "SweepShapeType == ESweepShapeType::Capsule", EditConditionHides))
	float SweepCapsuleHalfHeight = 40.0f;

	/** Object types to detect with sweep */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Collision")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	/** Actors to always ignore (in addition to owner) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Collision")
	TArray<AActor*> IgnoredActors;

	/**
	 * Number of interpolation steps between frames.
	 * Higher values = smoother traces but more expensive.
	 * 1 = No interpolation (single sweep per tick)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Collision", meta = (ClampMin = "1", ClampMax = "10"))
	int32 InterpolationSteps = 1;

	// ============================================================================
	// Preview & Debug
	// ============================================================================

	/** Show sweep box preview in editor viewport */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Preview")
	bool bShowEditorPreview = true;

	/** Preview box color in editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Preview", meta = (EditCondition = "bShowEditorPreview"))
	FColor EditorPreviewColor = FColor::Cyan;

	/** Preview line thickness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Preview", meta = (EditCondition = "bShowEditorPreview", ClampMin = "0.5", ClampMax = "10.0"))
	float EditorPreviewThickness = 2.0f;

	/** Preview segments for sphere/capsule */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Preview", meta = (EditCondition = "bShowEditorPreview", ClampMin = "4", ClampMax = "32"))
	int32 EditorPreviewSegments = 16;

	/** Show debug draw during gameplay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Debug")
	bool bDebugDraw = false;

	/** Debug draw duration (0 = single frame) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Config|Debug", meta = (EditCondition = "bDebugDraw", ClampMin = "0.0"))
	float DebugDrawDuration = 0.0f;

	// ============================================================================
	// API
	// ============================================================================

	/**
	 * Start a new sweep session
	 * Clears previous hit actors and records initial position
	 */
	UFUNCTION(BlueprintCallable, Category = "Sweep Trace")
	void StartSweep();

	/**
	 * Perform sweep test for this frame
	 * Call every frame during attack window
	 * @param DeltaTime Time since last tick
	 */
	UFUNCTION(BlueprintCallable, Category = "Sweep Trace")
	void TickSweep(float DeltaTime);

	/**
	 * End the current sweep session
	 * Clears state for next attack
	 */
	UFUNCTION(BlueprintCallable, Category = "Sweep Trace")
	void StopSweep();

	/** Check if currently in a sweep session */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sweep Trace")
	bool IsSweeping() const { return bIsSweeping; }

	/** Set the target skeletal mesh explicitly */
	UFUNCTION(BlueprintCallable, Category = "Sweep Trace")
	void SetTargetMesh(USkeletalMeshComponent* InMesh);

	/** Get current socket world transform */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sweep Trace")
	FTransform GetSweepSocketTransform() const;

	/** Get actors hit during current sweep session */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Sweep Trace")
	TArray<AActor*> GetHitActorsThisSweep() const;

	/** Clear the hit actors list (allows re-hitting same targets) */
	UFUNCTION(BlueprintCallable, Category = "Sweep Trace")
	void ClearHitActors();

	// ============================================================================
	// Delegates
	// ============================================================================

	/**
	 * Called when sweep detects a new hit
	 * Only fires once per actor per sweep session (prevents duplicate damage)
	 */
	UPROPERTY(BlueprintAssignable, Category = "Sweep Trace|Events")
	FOnSweepTraceHitDelegate OnSweepTraceHit;

protected:
	/** Cached target mesh */
	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent> TargetMeshComponent;

	/** Currently in sweep mode */
	bool bIsSweeping = false;

	/** Previous frame's socket transform */
	FTransform PreviousTransform;

	/** Has valid previous transform */
	bool bHasPreviousTransform = false;

	/** Actors already hit in this sweep session (prevents duplicates) */
	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> HitActorsThisSweep;

	/** Find skeletal mesh from owner */
	USkeletalMeshComponent* FindOwnerSkeletalMesh() const;

	/** Perform the actual sweep trace */
	void PerformSweep(const FTransform& FromTransform, const FTransform& ToTransform);

	/** Perform sweep without debug draw (called from TickSweep) */
	void PerformSweepWithoutDebug(const FTransform& FromTransform, const FTransform& ToTransform);

	/** Process hit results */
	void ProcessHits(const TArray<FHitResult>& HitResults);

	/** Draw debug visualization */
	void DrawDebugSweep(const FVector& From, const FVector& To, const FQuat& Rotation, bool bHadHits) const;
};
