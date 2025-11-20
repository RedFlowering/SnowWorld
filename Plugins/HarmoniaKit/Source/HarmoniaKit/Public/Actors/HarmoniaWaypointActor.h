// Copyright RedFlowering. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Definitions/HarmoniaMovementSystemDefinitions.h"
#include "HarmoniaWaypointActor.generated.h"

class USphereComponent;
class UBillboardComponent;

/**
 * Waypoint Actor for Fast Travel System
 * Place in world to create fast travel points
 */
UCLASS(BlueprintType)
class HARMONIAKIT_API AHarmoniaWaypointActor : public AActor
{
	GENERATED_BODY()

public:
	AHarmoniaWaypointActor();

protected:
	virtual void BeginPlay() override;

public:
	/** Get waypoint data */
	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	FHarmoniaWaypointData GetWaypointData() const { return WaypointData; }

	/** Is this waypoint discovered? */
	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	bool IsDiscovered() const { return bDiscovered; }

	/** Discover this waypoint */
	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	void DiscoverWaypoint();

	/** Can travel to this waypoint? */
	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	bool CanTravelTo() const;

protected:
	/** Waypoint configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
	FHarmoniaWaypointData WaypointData;

	/** Is discovered */
	UPROPERTY(BlueprintReadOnly, Category = "Waypoint")
	bool bDiscovered;

	/** Detection sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* DetectionSphere;

	/** Visual representation in editor */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBillboardComponent* Billboard;

	/** Handle player entering detection range */
	UFUNCTION()
	void OnDetectionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
