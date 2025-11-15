// Copyright 2025 Snow Game Studio.

#pragma once

#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaBuildingSystemDefinitions.h"
#include "HarmoniaBuildingComponent.generated.h"

class AHarmoniaBuildingPreviewActor;
class UHarmoniaBuildingInstanceManager;
class APlayerController;
class UHarmoniaInventoryComponent;
class UInputMappingContext;
class UDataTable;

UCLASS(ClassGroup=(Custom), meta = ( BlueprintSpawnableComponent ))
class HARMONIAKIT_API UHarmoniaBuildingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHarmoniaBuildingComponent();

	// === 모드 제어 ===
	UFUNCTION(BlueprintCallable, Category = "Building")
	void EnterBuildingMode();

	UFUNCTION(BlueprintCallable, Category = "Building")
	void ExitBuildingMode();

	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetBuildingMode(EBuildingMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "Building")
	void SetSelectedPart(FName PartID);

	// === 현재 선택된 파트 정보 ===
	UFUNCTION(BlueprintPure, Category = "Building")
	FName GetSelectedPartID() const { return SelectedPartID; }

	UFUNCTION(BlueprintPure, Category = "Building")
	EBuildingMode GetCurrentMode() const { return CurrentMode; }

	// === Tick 처리 ===
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	// === 입력 처리 ===
	void SetupInput();
	void HandlePlaceAction();
	void HandleRotateAction();
	void HandleCancelAction();

	// === 프리뷰 관련 함수 ===
	void SpawnPreviewActor();
	void UpdatePreviewTransform();
	void DestroyPreviewActor();

	// === 배치 처리 ===
	void PlaceCurrentPart();
	bool ValidatePlacement(FVector& OutLocation, FRotator& OutRotation);

	// === 유틸 함수 ===
	FBuildingPartData* GetCurrentPartData() const;

	// === 자원 검사 ===
	bool CheckAndConsumeResources(const FBuildingPartData& PartData);

private:
	// === 상태 ===
	EBuildingMode CurrentMode;

	UPROPERTY()
	FName SelectedPartID;

	// === 외부 참조 ===
	UPROPERTY()
	TObjectPtr<APlayerController> CachedPC;

	UPROPERTY()
	TObjectPtr<AHarmoniaBuildingPreviewActor> PreviewActor;

	UPROPERTY()
	TObjectPtr<UHarmoniaBuildingInstanceManager> InstanceManager;

	UPROPERTY()
	TObjectPtr<UHarmoniaInventoryComponent> InventoryComponent;

	// === 설정 ===
	UPROPERTY(EditDefaultsOnly, Category = "Building")
	TSubclassOf<AHarmoniaBuildingPreviewActor> PreviewActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Building|DataTable")
	FName BuildingDataTableKey = FName(TEXT("BuildingParts"));

	// === 배치 검증 설정 ===
	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	float MaxPlacementDistance = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	float SnapDistance = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	float GridSize = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Building|Placement")
	bool bUseGridSnapping = true;

	// 현재 회전 각도 (누적)
	float CurrentRotationYaw = 0.0f;

	// 캐시된 건축물 데이터 테이블
	UPROPERTY()
	TObjectPtr<UDataTable> CachedBuildingDataTable;
};
