// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_SweepTrace.generated.h"

class UHarmoniaSweepTraceComponent;

/**
 * Shape type for preview visualization
 */
UENUM(BlueprintType)
enum class EPreviewShapeType : uint8
{
	Box UMETA(DisplayName = "Box"),
	Sphere UMETA(DisplayName = "Sphere"),
	Capsule UMETA(DisplayName = "Capsule")
};

/**
 * UAnimNotifyState_SweepTrace
 *
 * Animation Notify State for sweep-based hit detection.
 * Automatically controls HarmoniaSweepTraceComponent during animation playback.
 *
 * Features:
 * - Integrates with HarmoniaSweepTraceComponent on the character
 * - Preview in Persona editor
 * - Supports multiple sweep components via ComponentTag
 *
 * Usage:
 * 1. Add HarmoniaSweepTraceComponent to Character BP
 * 2. Add this AnimNotifyState to attack animation montage
 * 3. Configure ComponentTag if using multiple sweep components
 *
 * The notify will:
 * - NotifyBegin: Call StartSweep() on the component
 * - NotifyTick: Call TickSweep() each frame
 * - NotifyEnd: Call StopSweep() on the component
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Sweep Trace"))
class HARMONIAKIT_API UAnimNotifyState_SweepTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UAnimNotifyState_SweepTrace();

	// ============================================================================
	// Configuration
	// ============================================================================

	/**
	 * Tag to identify which SweepTraceComponent to use
	 * Leave empty to use the first found component
	 * Set to match ComponentTags on specific components for multi-weapon support
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Trace")
	FName ComponentTag = NAME_None;

	/**
	 * Override sweep box extent for this specific attack
	 * If zero, uses the component's default
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Trace")
	FVector OverrideSweepExtent = FVector::ZeroVector;

	/**
	 * Enable debug draw during this notify
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Trace|Debug")
	bool bDebugDrawDuringNotify = false;

	// ============================================================================
	// Editor Preview
	// ============================================================================

	/**
	 * Show sweep box preview in Persona montage editor
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Trace|Preview")
	bool bShowPreviewInEditor = true;

	/**
	 * Socket name to use for preview (reads from preview mesh in editor)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Trace|Preview", meta = (EditCondition = "bShowPreviewInEditor"))
	FName PreviewSocketName = FName("weapon_r");

	/**
	 * Local offset from socket location for preview
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Trace|Preview", meta = (EditCondition = "bShowPreviewInEditor"))
	FVector PreviewSocketOffset = FVector::ZeroVector;

	/**
	 * Local rotation offset from socket for preview
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Trace|Preview", meta = (EditCondition = "bShowPreviewInEditor"))
	FRotator PreviewSocketRotation = FRotator::ZeroRotator;

	/**
	 * Preview shape type
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Trace|Preview", meta = (EditCondition = "bShowPreviewInEditor"))
	EPreviewShapeType PreviewShapeType = EPreviewShapeType::Box;

	/**
	 * Preview sweep box extent (used for Box shape)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Trace|Preview", meta = (EditCondition = "bShowPreviewInEditor && PreviewShapeType == EPreviewShapeType::Box", EditConditionHides))
	FVector PreviewSweepExtent = FVector(15.0f, 15.0f, 40.0f);

	/**
	 * Preview sphere/capsule radius
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Trace|Preview", meta = (EditCondition = "bShowPreviewInEditor && PreviewShapeType != EPreviewShapeType::Box", EditConditionHides))
	float PreviewRadius = 15.0f;

	/**
	 * Preview capsule half height (for Capsule shape only)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Trace|Preview", meta = (EditCondition = "bShowPreviewInEditor && PreviewShapeType == EPreviewShapeType::Capsule", EditConditionHides))
	float PreviewCapsuleHalfHeight = 40.0f;

	/**
	 * Preview color in editor
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Trace|Preview", meta = (EditCondition = "bShowPreviewInEditor"))
	FColor PreviewColor = FColor::Cyan;

	/**
	 * Preview line thickness
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Trace|Preview", meta = (EditCondition = "bShowPreviewInEditor", ClampMin = "0.5", ClampMax = "10.0"))
	float PreviewThickness = 2.0f;

	/**
	 * Number of segments for sphere/capsule preview
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sweep Trace|Preview", meta = (EditCondition = "bShowPreviewInEditor", ClampMin = "4", ClampMax = "32"))
	int32 PreviewSegments = 16;

	// ============================================================================
	// AnimNotifyState Overrides
	// ============================================================================

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

#if WITH_EDITORONLY_DATA
	virtual bool ShouldFireInEditor() override { return true; }
#endif

protected:
	/** Cached sweep component references (supports multiple components) */
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UHarmoniaSweepTraceComponent>> CachedSweepComponents;

	/** Original values per component (to restore after notify) */
	struct FOriginalComponentValues
	{
		FVector SweepExtent;
		bool bDebugDraw;
	};
	TArray<FOriginalComponentValues> OriginalValues;

	/** Notify duration for debug draw persistence */
	float NotifyDuration = 0.0f;

	/** Find all sweep trace components on the owner (optionally filtered by tag) */
	TArray<UHarmoniaSweepTraceComponent*> FindAllSweepComponents(USkeletalMeshComponent* MeshComp) const;

	/** Draw editor preview in Persona */
	void DrawEditorPreview(USkeletalMeshComponent* MeshComp) const;
};
