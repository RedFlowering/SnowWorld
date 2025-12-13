// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Sensors/ActiveSensor.h"
#include "HarmoniaSocketSensor.generated.h"

class USkeletalMeshComponent;

/**
 * UHarmoniaSocketSensor
 * 
 * A sensor that attaches to a skeletal mesh socket for hit detection.
 * The sensor's transform follows the specified socket in real-time.
 * 
 * Usage:
 * - Set SocketName to the target socket (e.g., "weapon_r", "hand_r")
 * - Call SetTargetMesh() when equipping weapons to set the mesh
 * - SensorTests (Distance, Box, etc.) define the detection area from the socket
 * 
 * Default SensorType is Manual - triggered via TriggerManualSensors() from AnimNotify
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew)
class HARMONIAKIT_API UHarmoniaSocketSensor : public UActiveSensor
{
	GENERATED_BODY()

public:
	UHarmoniaSocketSensor(const FObjectInitializer& ObjectInitializer);

	// ============================================================================
	// Socket Attachment Configuration
	// ============================================================================

	/** Name of the socket to attach to (e.g., "weapon_r", "hand_r") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Attachment")
	FName SocketName = NAME_None;

	/** Local offset from socket location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Attachment")
	FVector LocalOffset = FVector::ZeroVector;

	/** Local rotation offset from socket rotation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Attachment")
	FRotator LocalRotation = FRotator::ZeroRotator;

	/** 
	 * If true, automatically find SkeletalMeshComponent from owner on initialization
	 * If false, must call SetTargetMesh() explicitly
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket Attachment")
	bool bAutoFindOwnerMesh = true;

	// ============================================================================
	// API
	// ============================================================================

	/**
	 * Set the target skeletal mesh component for socket attachment
	 * Call this when equipping a weapon to set the weapon's mesh
	 * @param InMesh The skeletal mesh component containing the socket
	 */
	UFUNCTION(BlueprintCallable, Category = "SenseSystem|SocketSensor")
	void SetTargetMesh(USkeletalMeshComponent* InMesh);

	/**
	 * Get the current target mesh component
	 */
	UFUNCTION(BlueprintCallable, Category = "SenseSystem|SocketSensor")
	USkeletalMeshComponent* GetTargetMesh() const;

	/**
	 * Check if socket is valid on current target mesh
	 */
	UFUNCTION(BlueprintCallable, Category = "SenseSystem|SocketSensor")
	bool IsSocketValid() const;

	/**
	 * Get the current socket world transform
	 * @return Socket transform in world space, or Identity if invalid
	 */
	UFUNCTION(BlueprintCallable, Category = "SenseSystem|SocketSensor")
	FTransform GetSocketTransform() const;

	// ============================================================================
	// Overrides
	// ============================================================================

	virtual void InitializeFromReceiver(USenseReceiverComponent* FromReceiver) override;
	virtual EUpdateReady GetSensorReadyBP_Implementation() override;

#if WITH_EDITORONLY_DATA
	virtual void DrawSensor(const class FSceneView* View, class FPrimitiveDrawInterface* PDI) const override;
	virtual void DrawDebug(bool bTest, bool bCurrentSensed, bool bLostSensed, bool bBestSensed, bool bAge, float Duration) const override;
#endif

protected:
	/** Cached reference to target skeletal mesh component */
	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent> TargetMeshComponent;

	/** Find skeletal mesh component from owner actor */
	USkeletalMeshComponent* FindOwnerSkeletalMesh() const;

	/** Update SensorTransform from socket transform */
	void UpdateSensorTransformFromSocket();

	/** Sync all SensorTest transforms to match current socket position */
	void SyncSensorTestTransforms();
};
