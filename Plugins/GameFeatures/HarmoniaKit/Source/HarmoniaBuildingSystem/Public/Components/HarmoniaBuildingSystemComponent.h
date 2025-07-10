// Copyright 2025 Snow Game Studio.

#pragma once

#include "Components/ActorComponent.h"
#include "Definitions/HarmoniaBuildingSystemDefinitions.h"
#include "HarmoniaBuildingSystemComponent.generated.h"

//class ABuildingPreviewActor;
// class UBuildingInstanceManagerComponent;
class APlayerController;
// class USenseReceiverComponent;
class UInputMappingContext;

UCLASS(ClassGroup=(Custom), meta = ( BlueprintSpawnableComponent ))
class HARMONIABUILDINGSYSTEM_API UHarmoniaBuildingSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHarmoniaBuildingSystemComponent();

//    // === 모드 제어 ===
//    UFUNCTION(BlueprintCallable)
//    void EnterBuildingMode();
//
//    UFUNCTION(BlueprintCallable)
//    void ExitBuildingMode();
//
//    UFUNCTION(BlueprintCallable)
//    void SetBuildingMode(EBuildingMode NewMode);
//
//    UFUNCTION(BlueprintCallable)
//    void SetSelectedPart(FName PartID);
//
//    // === Tick 처리 ===
//    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
//
//protected:
//    virtual void BeginPlay() override;
//
//    // === 입력 처리 ===
//    void SetupInput();
//    void HandlePlaceAction();
//    void HandleRotateAction();
//    void HandleCancelAction();
//
//    // === 프리뷰 액터 관리 ===
//    void SpawnPreviewActor();
//    void UpdatePreviewTransform();
//    void DestroyPreviewActor();
//
//    // === 배치 처리 ===
//    void PlaceCurrentPart(); // <- 여기에 RequiredResources 검사 추가
//    bool ValidatePlacement(FVector& OutLocation, FRotator& OutRotation);
//
//    // === 내부 유틸 ===
//    FBuildingPartData* GetCurrentPartData() const;
//
//private:
//    // === 상태 ===
//    EBuildingMode CurrentMode;
//    
//    UPROPERTY()
//    FName SelectedPartID;
//
//    // === 외부 참조 ===
//    UPROPERTY()
//    APlayerController* CachedPC;
//
//    UPROPERTY()
//    ABuildingPreviewActor* PreviewActor;
//
//    UPROPERTY()
//    UBuildingInstanceManagerComponent* InstanceManager;
//
//    UPROPERTY()
//    USenseReceiverComponent* SenseReceiver;
//
//    // === 설정 ===
//    UPROPERTY(EditDefaultsOnly, Category = "Building")
//    TSubclassOf<ABuildingPreviewActor> PreviewActorClass;
//
//    UPROPERTY(EditDefaultsOnly, Category = "Input")
//    UInputMappingContext* InputMapping;
};
