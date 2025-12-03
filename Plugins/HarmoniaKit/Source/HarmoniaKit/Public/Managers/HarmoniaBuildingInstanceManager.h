// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Managers/HarmoniaInstancedObjectManagerBase.h"
#include "Definitions/HarmoniaBuildingSystemDefinitions.h"
#include "HarmoniaBuildingInstanceManager.generated.h"

class AActor;
class AController;

/**
 * 건축�??�스?�스 매니?� (WorldSubsystem)
 * - 배치??건축물을 ?�스?�싱?�로 관�?
 * - ?�레?�어 ?�근 ???�요???�라 ?�터�?변??
 * - 건축�?배치, ?�괴, ?�리 기능 ?�공
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaBuildingInstanceManager : public UHarmoniaInstancedObjectManagerBase
{
	GENERATED_BODY()

public:
	// 건축�?배치
	UFUNCTION(BlueprintCallable, Category = "Building")
	FGuid PlaceBuilding(const FHarmoniaBuildingPartData& PartData, const FVector& Location, const FRotator& Rotation, AActor* Owner);

	// 건축�??�거
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool RemoveBuilding(const FGuid& BuildingGuid);

	// 건축�??�리
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool RepairBuilding(const FGuid& BuildingGuid, float RepairAmount);

	// 건축�??�구??감소
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool DamageBuilding(const FGuid& BuildingGuid, float DamageAmount);

	// 건축�??�이??조회
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool GetBuildingMetadata(const FGuid& BuildingGuid, FBuildingInstanceMetadata& OutMetadata) const;

	// 충돌 검??- ?�정 ?�치?� 범위???��? 배치??건축물이 ?�는지 ?�인
	// @param Location - 배치?�려???�치
	// @param Rotation - 배치?�려???�전
	// @param BoundsExtent - 배치?�려??건축물의 경계 ?�기
	// @param PlacingPartType - 배치?�려??건축�??�??(?�버???�용 규칙 ?�단???�용)
	// @param MinDistance - 최소 거리 (?�수??경우 거리 검???�킵)
	// @return true�?충돌 발생 (배치 불�?), false�?배치 가??
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool CheckBuildingOverlap(const FVector& Location, const FRotator& Rotation, const FVector& BoundsExtent, EBuildingPartType PlacingPartType, float MinDistance = -1.0f) const;

	// 모든 건축�?메�??�이??가?�오�?
	UFUNCTION(BlueprintCallable, Category = "Building")
	void GetAllBuildingMetadata(TArray<FBuildingInstanceMetadata>& OutMetadataArray) const;

	// ?�냅 ?�인??찾기 - 주어�??�치 주�??�서 ?�냅 가?�한 지?�을 찾음
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool FindNearbySnapPoint(const FVector& TargetLocation, EBuildingPartType PartType, float SearchRadius, FVector& OutSnapLocation, FRotator& OutSnapRotation) const;

protected:
	// 건축�??�터 ?�성 (?�제 3D ?�브?�트)
	virtual AActor* SpawnWorldActor(const FHarmoniaInstancedObjectData& Data, AController* Requestor) override;

	// 건축�??�터 ?�괴
	virtual void DestroyWorldActor(AActor* Actor) override;

	// WorldSubsystem 초기??
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	// 건축�?메�??�이???�??(?�구?? ?�유????
	UPROPERTY()
	TMap<FGuid, FBuildingInstanceMetadata> BuildingMetadataMap;

	// 건축�??�트�?Instanced Static Mesh Component �?
	// Key: PartID, Value: ISM Component
	UPROPERTY()
	TMap<FName, class UInstancedStaticMeshComponent*> PartToISMMap;

	// ISM??관리할 루트 ?�터
	UPROPERTY()
	TObjectPtr<AActor> ISMManagerActor = nullptr;

	// ISM 컴포?�트 초기??
	void InitializeISMComponent(const FName& PartID, UStaticMesh* Mesh);

	// ?�?�별 ?�버???�용 규칙 체크
	bool IsOverlapAllowed(EBuildingPartType PlacingType, EBuildingPartType ExistingType) const;

	// OBB(Oriented Bounding Box) 충돌 검??
	bool DoBoxesOverlap(const FTransform& TransformA, const FVector& ExtentA, const FTransform& TransformB, const FVector& ExtentB) const;

	// 건축�??�이???�이�?캐시
	UPROPERTY()
	TObjectPtr<UDataTable> BuildingDataTable = nullptr;
};
