// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaBuildingSystemComponent.h"
// #include "BuildingPreviewActor.h"
// #include "BuildingInstanceManagerComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
// #include "SenseReceiverComponent.h"

// 로그용
DEFINE_LOG_CATEGORY_STATIC(LogBuildingSystem, Log, All);

UHarmoniaBuildingSystemComponent::UHarmoniaBuildingSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    // CurrentMode = EBuildingMode::None;
    // SelectedPartID = NAME_None;
}

//void UBuildingSystemComponent::BeginPlay()
//{
//    Super::BeginPlay();
//
//    CachedPC = Cast<APlayerController>(GetOwner()->GetInstigatorController());
//    // 나중에 InstanceManager, SenseReceiver 같은 외부 참조도 여기서 찾기
//}
//
//void UBuildingSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
//{
//    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
//
//    if (CurrentMode == EBuildingMode::Build && PreviewActor)
//    {
//        UpdatePreviewTransform();
//    }
//}
//void UBuildingSystemComponent::SpawnPreviewActor()
//{
//    if (!PreviewActorClass || PreviewActor)
//        return;
//
//    UWorld* World = GetWorld();
//    if (!World)
//        return;
//
//    PreviewActor = World->SpawnActor<ABuildingPreviewActor>(PreviewActorClass);
//    if (FBuildingPartData* PartData = GetCurrentPartData())
//    {
//        PreviewActor->ApplyPreviewData(*PartData);
//    }
//}
//
//void UBuildingSystemComponent::DestroyPreviewActor()
//{
//    if (PreviewActor)
//    {
//        PreviewActor->Destroy();
//        PreviewActor = nullptr;
//    }
//}
//
//void UBuildingSystemComponent::UpdatePreviewTransform()
//{
//    if (!PreviewActor || !SenseReceiver)
//        return;
//
//    // 예: 마우스 위치 → 스냅 or 지면 위치 계산
//    FVector Location;
//    FRotator Rotation;
//
//    bool bValid = ValidatePlacement(Location, Rotation);
//    PreviewActor->SetActorLocation(Location);
//    PreviewActor->SetActorRotation(Rotation);
//    PreviewActor->SetIsPlacementValid(bValid);
//}
//void UBuildingSystemComponent::PlaceCurrentPart()
//{
//    if (!PreviewActor || !InstanceManager || !GetOwner())
//        return;
//
//    FBuildingPartData* PartData = GetCurrentPartData();
//    if (!PartData)
//        return;
//
//    FVector Location;
//    FRotator Rotation;
//
//    if (!ValidatePlacement(Location, Rotation))
//        return;
//
//    // ✅ 자원 검사 (임시)
//    for (const FItemCount& Cost : PartData->RequiredResources)
//    {
//        UE_LOG(LogBuildingSystem, Log, TEXT("Required: %s x%d"), *Cost.Item.ID.ToString(), Cost.Count);
//        // TODO: 인벤토리 시스템에서 실제 검사 및 소비 필요
//    }
//
//    // ✅ 인스턴스 매니저에 배치 요청
//    InstanceManager->PlaceInstance(*PartData, Location, Rotation, GetOwner());
//
//    // Destroy or reset preview?
//    DestroyPreviewActor();
//    SpawnPreviewActor();
//}
//void UBuildingSystemComponent::SetSelectedPart(FName PartID)
//{
//    SelectedPartID = PartID;
//
//    if (PreviewActor&& FBuildingPartData* PartData = GetCurrentPartData())
//    {
//        PreviewActor->ApplyPreviewData(*PartData);
//    }
//}
//
//void UBuildingSystemComponent::SetBuildingMode(EBuildingMode NewMode)
//{
//    if (NewMode == CurrentMode)
//        return;
//
//    CurrentMode = NewMode;
//
//    if (CurrentMode == EBuildingMode::Build)
//    {
//        SpawnPreviewActor();
//    }
//    else
//    {
//        DestroyPreviewActor();
//    }
//
//    // 나중에 InputMapping 변경 대응 (Lyra 스타일)
//}
//
//void UBuildingSystemComponent::EnterBuildingMode()
//{
//    SetBuildingMode(EBuildingMode::Build);
//}
//
//void UBuildingSystemComponent::ExitBuildingMode()
//{
//    SetBuildingMode(EBuildingMode::None);
//}
//FBuildingPartData* UBuildingSystemComponent::GetCurrentPartData() const
//{
//    // DataTable 또는 글로벌 매니저에서 ID로 파츠 정보 조회
//    // 임시:
//    return nullptr; // TODO: 실제 조회 로직 연결
//}
//
//bool UBuildingSystemComponent::ValidatePlacement(FVector& OutLocation, FRotator& OutRotation)
//{
//    // TODO: SenseSystem 연동 및 SnapSystem 활용
//    OutLocation = FVector::ZeroVector;
//    OutRotation = FRotator::ZeroRotator;
//    return true;
//}
