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
	TObjectPtr<AActor> ISMManagerActor;

	// ISM 컴포넌트 초기화
	void InitializeISMComponent(const FName& PartID, UStaticMesh* Mesh);

	// 건축물 데이터 테이블 캐시
	UPROPERTY()
	TObjectPtr<UDataTable> BuildingDataTable;
};
