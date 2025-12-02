// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Managers/HarmoniaInstancedObjectManagerBase.h"
#include "Definitions/HarmoniaBuildingSystemDefinitions.h"
#include "HarmoniaBuildingInstanceManager.generated.h"

class AActor;
class AController;

/**
 * ê±´ì¶•ë¬??¸ìŠ¤?´ìŠ¤ ë§¤ë‹ˆ?€ (WorldSubsystem)
 * - ë°°ì¹˜??ê±´ì¶•ë¬¼ì„ ?¸ìŠ¤?´ì‹±?¼ë¡œ ê´€ë¦?
 * - ?Œë ˆ?´ì–´ ?‘ê·¼ ???„ìš”???°ë¼ ?¡í„°ë¡?ë³€??
 * - ê±´ì¶•ë¬?ë°°ì¹˜, ?Œê´´, ?˜ë¦¬ ê¸°ëŠ¥ ?œê³µ
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaBuildingInstanceManager : public UHarmoniaInstancedObjectManagerBase
{
	GENERATED_BODY()

public:
	// ê±´ì¶•ë¬?ë°°ì¹˜
	UFUNCTION(BlueprintCallable, Category = "Building")
	FGuid PlaceBuilding(const FHarmoniaBuildingPartData& PartData, const FVector& Location, const FRotator& Rotation, AActor* Owner);

	// ê±´ì¶•ë¬??œê±°
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool RemoveBuilding(const FGuid& BuildingGuid);

	// ê±´ì¶•ë¬??˜ë¦¬
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool RepairBuilding(const FGuid& BuildingGuid, float RepairAmount);

	// ê±´ì¶•ë¬??´êµ¬??ê°ì†Œ
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool DamageBuilding(const FGuid& BuildingGuid, float DamageAmount);

	// ê±´ì¶•ë¬??°ì´??ì¡°íšŒ
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool GetBuildingMetadata(const FGuid& BuildingGuid, FBuildingInstanceMetadata& OutMetadata) const;

	// ì¶©ëŒ ê²€??- ?¹ì • ?„ì¹˜?€ ë²”ìœ„???´ë? ë°°ì¹˜??ê±´ì¶•ë¬¼ì´ ?ˆëŠ”ì§€ ?•ì¸
	// @param Location - ë°°ì¹˜?˜ë ¤???„ì¹˜
	// @param Rotation - ë°°ì¹˜?˜ë ¤???Œì „
	// @param BoundsExtent - ë°°ì¹˜?˜ë ¤??ê±´ì¶•ë¬¼ì˜ ê²½ê³„ ?¬ê¸°
	// @param PlacingPartType - ë°°ì¹˜?˜ë ¤??ê±´ì¶•ë¬??€??(?¤ë²„???ˆìš© ê·œì¹™ ?ë‹¨???¬ìš©)
	// @param MinDistance - ìµœì†Œ ê±°ë¦¬ (?Œìˆ˜??ê²½ìš° ê±°ë¦¬ ê²€???¤í‚µ)
	// @return trueë©?ì¶©ëŒ ë°œìƒ (ë°°ì¹˜ ë¶ˆê?), falseë©?ë°°ì¹˜ ê°€??
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool CheckBuildingOverlap(const FVector& Location, const FRotator& Rotation, const FVector& BoundsExtent, EBuildingPartType PlacingPartType, float MinDistance = -1.0f) const;

	// ëª¨ë“  ê±´ì¶•ë¬?ë©”í??°ì´??ê°€?¸ì˜¤ê¸?
	UFUNCTION(BlueprintCallable, Category = "Building")
	void GetAllBuildingMetadata(TArray<FBuildingInstanceMetadata>& OutMetadataArray) const;

	// ?¤ëƒ… ?¬ì¸??ì°¾ê¸° - ì£¼ì–´ì§??„ì¹˜ ì£¼ë??ì„œ ?¤ëƒ… ê°€?¥í•œ ì§€?ì„ ì°¾ìŒ
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool FindNearbySnapPoint(const FVector& TargetLocation, EBuildingPartType PartType, float SearchRadius, FVector& OutSnapLocation, FRotator& OutSnapRotation) const;

protected:
	// ê±´ì¶•ë¬??¡í„° ?ì„± (?¤ì œ 3D ?¤ë¸Œ?íŠ¸)
	virtual AActor* SpawnWorldActor(const FHarmoniaInstancedObjectData& Data, AController* Requestor) override;

	// ê±´ì¶•ë¬??¡í„° ?Œê´´
	virtual void DestroyWorldActor(AActor* Actor) override;

	// WorldSubsystem ì´ˆê¸°??
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	// ê±´ì¶•ë¬?ë©”í??°ì´???€??(?´êµ¬?? ?Œìœ ????
	UPROPERTY()
	TMap<FGuid, FBuildingInstanceMetadata> BuildingMetadataMap;

	// ê±´ì¶•ë¬??ŒíŠ¸ë³?Instanced Static Mesh Component ë§?
	// Key: PartID, Value: ISM Component
	UPROPERTY()
	TMap<FName, class UInstancedStaticMeshComponent*> PartToISMMap;

	// ISM??ê´€ë¦¬í•  ë£¨íŠ¸ ?¡í„°
	UPROPERTY()
	TObjectPtr<AActor> ISMManagerActor = nullptr;

	// ISM ì»´í¬?ŒíŠ¸ ì´ˆê¸°??
	void InitializeISMComponent(const FName& PartID, UStaticMesh* Mesh);

	// ?€?…ë³„ ?¤ë²„???ˆìš© ê·œì¹™ ì²´í¬
	bool IsOverlapAllowed(EBuildingPartType PlacingType, EBuildingPartType ExistingType) const;

	// OBB(Oriented Bounding Box) ì¶©ëŒ ê²€??
	bool DoBoxesOverlap(const FTransform& TransformA, const FVector& ExtentA, const FTransform& TransformB, const FVector& ExtentB) const;

	// ê±´ì¶•ë¬??°ì´???Œì´ë¸?ìºì‹œ
	UPROPERTY()
	TObjectPtr<UDataTable> BuildingDataTable = nullptr;
};
