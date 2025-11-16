// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Managers/HarmoniaInstancedObjectManagerBase.h"
#include "Definitions/HarmoniaBuildingSystemDefinitions.h"
#include "HarmoniaBuildingInstanceManager.generated.h"

class AActor;
class AController;

/**
 * 건축물 인스턴스 매니저 (WorldSubsystem)
 * - 배치된 건축물을 인스턴싱으로 관리
 * - 플레이어 접근 시 필요에 따라 액터로 변환
 * - 건축물 배치, 파괴, 수리 기능 제공
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaBuildingInstanceManager : public UHarmoniaInstancedObjectManagerBase
{
	GENERATED_BODY()

public:
	// 건축물 배치
	UFUNCTION(BlueprintCallable, Category = "Building")
	FGuid PlaceBuilding(const FBuildingPartData& PartData, const FVector& Location, const FRotator& Rotation, AActor* Owner);

	// 건축물 제거
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool RemoveBuilding(const FGuid& BuildingGuid);

	// 건축물 수리
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool RepairBuilding(const FGuid& BuildingGuid, float RepairAmount);

	// 건축물 내구도 감소
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool DamageBuilding(const FGuid& BuildingGuid, float DamageAmount);

	// 건축물 데이터 조회
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool GetBuildingMetadata(const FGuid& BuildingGuid, FBuildingInstanceMetadata& OutMetadata) const;

	// 충돌 검사 - 특정 위치와 범위에 이미 배치된 건축물이 있는지 확인
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool CheckBuildingOverlap(const FVector& Location, const FRotator& Rotation, const FVector& BoundsExtent, float MinDistance = 50.0f) const;

	// 모든 건축물 메타데이터 가져오기
	UFUNCTION(BlueprintCallable, Category = "Building")
	void GetAllBuildingMetadata(TArray<FBuildingInstanceMetadata>& OutMetadataArray) const;

	// 스냅 포인트 찾기 - 주어진 위치 주변에서 스냅 가능한 지점을 찾음
	UFUNCTION(BlueprintCallable, Category = "Building")
	bool FindNearbySnapPoint(const FVector& TargetLocation, EBuildingPartType PartType, float SearchRadius, FVector& OutSnapLocation, FRotator& OutSnapRotation) const;

protected:
	// 건축물 액터 생성 (실제 3D 오브젝트)
	virtual AActor* SpawnWorldActor(const FHarmoniaInstancedObjectData& Data, AController* Requestor) override;

	// 건축물 액터 파괴
	virtual void DestroyWorldActor(AActor* Actor) override;

	// WorldSubsystem 초기화
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	// 건축물 메타데이터 저장 (내구도, 소유자 등)
	UPROPERTY()
	TMap<FGuid, FBuildingInstanceMetadata> BuildingMetadataMap;

	// 건축물 파트별 Instanced Static Mesh Component 맵
	// Key: PartID, Value: ISM Component
	UPROPERTY()
	TMap<FName, class UInstancedStaticMeshComponent*> PartToISMMap;

	// ISM을 관리할 루트 액터
	UPROPERTY()
	TObjectPtr<AActor> ISMManagerActor = nullptr;

	// ISM 컴포넌트 초기화
	void InitializeISMComponent(const FName& PartID, UStaticMesh* Mesh);

	// 건축물 데이터 테이블 캐시
	UPROPERTY()
	TObjectPtr<UDataTable> BuildingDataTable = nullptr;
};
