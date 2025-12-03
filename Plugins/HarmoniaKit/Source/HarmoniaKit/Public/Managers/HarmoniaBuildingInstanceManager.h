// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Managers/HarmoniaInstancedObjectManagerBase.h"
#include "Definitions/HarmoniaBuildingSystemDefinitions.h"
#include "HarmoniaBuildingInstanceManager.generated.h"

class AActor;
class AController;

/**
 * @file HarmoniaBuildingInstanceManager.h
 * @brief Building instance manager (WorldSubsystem)
 * 
 * This manager handles:
 * - Managing placed buildings via instanced rendering
 * - Actor-to-instancing transitions based on player proximity
 * - Building placement, destruction, and repair functionality
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaBuildingInstanceManager : public UHarmoniaInstancedObjectManagerBase
{
	GENERATED_BODY()

public:
	/** Place a building at the specified location */
	UFUNCTION(BlueprintCallable, Category = "Building")
	FGuid PlaceBuilding(const FHarmoniaBuildingPartData& PartData, const FVector& Location, const FRotator& Rotation, AActor* Owner);

	/** Remove a building by GUID */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool RemoveBuilding(const FGuid& BuildingGuid);

	/** Repair a building */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool RepairBuilding(const FGuid& BuildingGuid, float RepairAmount);

	/** Apply damage to a building's durability */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool DamageBuilding(const FGuid& BuildingGuid, float DamageAmount);

	/** Get building metadata by GUID */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool GetBuildingMetadata(const FGuid& BuildingGuid, FBuildingInstanceMetadata& OutMetadata) const;

	/**
	 * Check for building overlap at a location
	 * @param Location - Target placement location
	 * @param Rotation - Target placement rotation
	 * @param BoundsExtent - Bounding box extents of the building to place
	 * @param PlacingPartType - Type of building part (used for overlap rules)
	 * @param MinDistance - Minimum distance check (negative skips distance check)
	 * @return true if overlap detected (cannot place), false if placement is valid
	 */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool CheckBuildingOverlap(const FVector& Location, const FRotator& Rotation, const FVector& BoundsExtent, EBuildingPartType PlacingPartType, float MinDistance = -1.0f) const;

	/** Get all building metadata */
	UFUNCTION(BlueprintCallable, Category = "Building")
	void GetAllBuildingMetadata(TArray<FBuildingInstanceMetadata>& OutMetadataArray) const;

	/** Find nearby snap point for building placement */
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool FindNearbySnapPoint(const FVector& TargetLocation, EBuildingPartType PartType, float SearchRadius, FVector& OutSnapLocation, FRotator& OutSnapRotation) const;

protected:
	/** Spawn a building actor (3D world object) */
	virtual AActor* SpawnWorldActor(const FHarmoniaInstancedObjectData& Data, AController* Requestor) override;

	/** Destroy a building actor */
	virtual void DestroyWorldActor(AActor* Actor) override;

	/** WorldSubsystem initialization */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	/** Building metadata map (durability, ownership, etc.) */
	UPROPERTY()
	TMap<FGuid, FBuildingInstanceMetadata> BuildingMetadataMap;

	/** Building part to Instanced Static Mesh Component mapping */
	// Key: PartID, Value: ISM Component
	UPROPERTY()
	TMap<FName, class UInstancedStaticMeshComponent*> PartToISMMap;

	/** Root actor that manages all ISM components */
	UPROPERTY()
	TObjectPtr<AActor> ISMManagerActor = nullptr;

	/** Initialize ISM component for a building part */
	void InitializeISMComponent(const FName& PartID, UStaticMesh* Mesh);

	/** Check if overlap is allowed between two building part types */
	bool IsOverlapAllowed(EBuildingPartType PlacingType, EBuildingPartType ExistingType) const;

	/** OBB (Oriented Bounding Box) collision check */
	bool DoBoxesOverlap(const FTransform& TransformA, const FVector& ExtentA, const FTransform& TransformB, const FVector& ExtentB) const;

	/** Building data table cache */
	UPROPERTY()
	TObjectPtr<UDataTable> BuildingDataTable = nullptr;
};
