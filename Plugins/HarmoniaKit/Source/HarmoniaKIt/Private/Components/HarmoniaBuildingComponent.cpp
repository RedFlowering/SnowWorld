// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaBuildingComponent.h"
#include "Actors/HarmoniaBuildingPreviewActor.h"
#include "Managers/HarmoniaBuildingInstanceManager.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "HarmoniaLoadManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"

// 로그용
DEFINE_LOG_CATEGORY_STATIC(LogBuildingSystem, Log, All);

UHarmoniaBuildingComponent::UHarmoniaBuildingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentMode = EBuildingMode::None;
	SelectedPartID = NAME_None;
}

void UHarmoniaBuildingComponent::BeginPlay()
{
	Super::BeginPlay();

	// PlayerController 캐싱
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		CachedPC = Cast<APlayerController>(OwnerPawn->GetController());
	}

	// BuildingInstanceManager 가져오기 (WorldSubsystem)
	if (UWorld* World = GetWorld())
	{
		InstanceManager = World->GetSubsystem<UHarmoniaBuildingInstanceManager>();
		if (!InstanceManager)
		{
			UE_LOG(LogBuildingSystem, Warning, TEXT("UHarmoniaBuildingInstanceManager not found in world"));
		}
	}

	// InventoryComponent 찾기
	if (AActor* Owner = GetOwner())
	{
		InventoryComponent = Owner->FindComponentByClass<UHarmoniaInventoryComponent>();
		if (!InventoryComponent)
		{
			UE_LOG(LogBuildingSystem, Warning, TEXT("UHarmoniaInventoryComponent not found on owner"));
		}
	}

	// BuildingDataTable 로드
	if (UHarmoniaLoadManager* LoadManager = UHarmoniaLoadManager::Get())
	{
		CachedBuildingDataTable = LoadManager->GetDataTableByKey(BuildingDataTableKey);
		if (!CachedBuildingDataTable)
		{
			UE_LOG(LogBuildingSystem, Warning, TEXT("Failed to load BuildingDataTable with key: %s"), *BuildingDataTableKey.ToString());
		}
	}
}

void UHarmoniaBuildingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentMode == EBuildingMode::Build && PreviewActor)
	{
		UpdatePreviewTransform();
	}
}

void UHarmoniaBuildingComponent::EnterBuildingMode()
{
	SetBuildingMode(EBuildingMode::Build);
}

void UHarmoniaBuildingComponent::ExitBuildingMode()
{
	SetBuildingMode(EBuildingMode::None);
}

void UHarmoniaBuildingComponent::SetBuildingMode(EBuildingMode NewMode)
{
	if (NewMode == CurrentMode)
		return;

	CurrentMode = NewMode;

	if (CurrentMode == EBuildingMode::Build)
	{
		SpawnPreviewActor();
		UE_LOG(LogBuildingSystem, Log, TEXT("Entered Build Mode"));
	}
	else
	{
		DestroyPreviewActor();
		UE_LOG(LogBuildingSystem, Log, TEXT("Exited Build Mode"));
	}

	// TODO: 나중에 InputMapping 변경 대응
}

void UHarmoniaBuildingComponent::SetSelectedPart(FName PartID)
{
	SelectedPartID = PartID;
	CurrentRotationYaw = 0.0f; // 회전 초기화

	if (PreviewActor)
	{
		if (FBuildingPartData* PartData = GetCurrentPartData())
		{
			PreviewActor->ApplyPreviewData(*PartData);
			UE_LOG(LogBuildingSystem, Log, TEXT("Selected Part: %s"), *PartID.ToString());
		}
		else
		{
			UE_LOG(LogBuildingSystem, Warning, TEXT("Part data not found for ID: %s"), *PartID.ToString());
		}
	}
}

void UHarmoniaBuildingComponent::SpawnPreviewActor()
{
	if (!PreviewActorClass || PreviewActor)
		return;

	UWorld* World = GetWorld();
	if (!World)
		return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PreviewActor = World->SpawnActor<AHarmoniaBuildingPreviewActor>(PreviewActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (PreviewActor)
	{
		if (FBuildingPartData* PartData = GetCurrentPartData())
		{
			PreviewActor->ApplyPreviewData(*PartData);
		}
		UE_LOG(LogBuildingSystem, Log, TEXT("Preview Actor Spawned"));
	}
}

void UHarmoniaBuildingComponent::DestroyPreviewActor()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
		UE_LOG(LogBuildingSystem, Log, TEXT("Preview Actor Destroyed"));
	}
}

void UHarmoniaBuildingComponent::UpdatePreviewTransform()
{
	if (!PreviewActor || !CachedPC)
		return;

	FVector Location;
	FRotator Rotation;

	bool bValid = ValidatePlacement(Location, Rotation);

	PreviewActor->SetActorLocation(Location);
	PreviewActor->SetActorRotation(Rotation);
	PreviewActor->SetIsPlacementValid(bValid);
}

bool UHarmoniaBuildingComponent::ValidatePlacement(FVector& OutLocation, FRotator& OutRotation)
{
	if (!CachedPC)
		return false;

	// 카메라 위치와 방향 가져오기
	FVector CameraLocation;
	FRotator CameraRotation;
	CachedPC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector TraceStart = CameraLocation;
	FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * MaxPlacementDistance);

	// 레이캐스트 수행
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	if (PreviewActor)
	{
		QueryParams.AddIgnoredActor(PreviewActor);
	}

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (!bHit)
	{
		// 히트하지 않으면 최대 거리에 배치
		OutLocation = TraceEnd;
		OutRotation = FRotator(0.0f, CameraRotation.Yaw + CurrentRotationYaw, 0.0f);
		return false;
	}

	// 그리드 스냅 적용
	FVector SnappedLocation = HitResult.Location;
	if (bUseGridSnapping && GridSize > 0.0f)
	{
		SnappedLocation.X = FMath::RoundToFloat(SnappedLocation.X / GridSize) * GridSize;
		SnappedLocation.Y = FMath::RoundToFloat(SnappedLocation.Y / GridSize) * GridSize;
		SnappedLocation.Z = FMath::RoundToFloat(SnappedLocation.Z / GridSize) * GridSize;
	}

	OutLocation = SnappedLocation;
	OutRotation = FRotator(0.0f, CameraRotation.Yaw + CurrentRotationYaw, 0.0f);

	// TODO: 스냅 포인트 검사 (다른 건축물과의 연결)
	// TODO: 충돌 검사 (다른 건축물과 겹치는지)

	return true;
}

void UHarmoniaBuildingComponent::PlaceCurrentPart()
{
	if (!PreviewActor || !InstanceManager || !GetOwner())
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("Cannot place: Missing required components"));
		return;
	}

	FBuildingPartData* PartData = GetCurrentPartData();
	if (!PartData)
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("Cannot place: Part data not found"));
		return;
	}

	FVector Location;
	FRotator Rotation;

	if (!ValidatePlacement(Location, Rotation))
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("Cannot place: Invalid placement location"));
		return;
	}

	// 자원 검사 및 소비
	if (!CheckAndConsumeResources(*PartData))
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("Cannot place: Insufficient resources"));
		return;
	}

	// 건축물 배치
	FGuid BuildingGuid = InstanceManager->PlaceBuilding(*PartData, Location, Rotation, GetOwner());

	if (BuildingGuid.IsValid())
	{
		UE_LOG(LogBuildingSystem, Log, TEXT("Building placed successfully: %s at %s"),
			*PartData->ID.ToString(), *Location.ToString());

		// 프리뷰 리셋 (계속 건축 모드 유지)
		DestroyPreviewActor();
		SpawnPreviewActor();
	}
	else
	{
		UE_LOG(LogBuildingSystem, Error, TEXT("Failed to place building"));
	}
}

bool UHarmoniaBuildingComponent::CheckAndConsumeResources(const FBuildingPartData& PartData)
{
	if (!InventoryComponent)
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("InventoryComponent not found, skipping resource check"));
		return true; // 인벤토리가 없으면 무료로 건축 허용 (디버그용)
	}

	// 1단계: 모든 필요 자원을 확인
	for (const FBuildingResourceCost& Cost : PartData.RequiredResources)
	{
		int32 CurrentCount = InventoryComponent->GetTotalCount(Cost.Item);
		if (CurrentCount < Cost.Count)
		{
			UE_LOG(LogBuildingSystem, Warning, TEXT("Insufficient resource: %s (Required: %d, Have: %d)"),
				*Cost.Item.ID.ToString(), Cost.Count, CurrentCount);
			return false;
		}
	}

	// 2단계: 모든 자원 소비
	for (const FBuildingResourceCost& Cost : PartData.RequiredResources)
	{
		bool bSuccess = InventoryComponent->RemoveItem(Cost.Item, Cost.Count, 0.0f);
		if (!bSuccess)
		{
			UE_LOG(LogBuildingSystem, Error, TEXT("Failed to remove item: %s"), *Cost.Item.ID.ToString());
			// TODO: 롤백 로직 필요 (이미 제거된 아이템 복구)
			return false;
		}
		else
		{
			UE_LOG(LogBuildingSystem, Log, TEXT("Consumed resource: %s x%d"), *Cost.Item.ID.ToString(), Cost.Count);
		}
	}

	return true;
}

FBuildingPartData* UHarmoniaBuildingComponent::GetCurrentPartData() const
{
	if (SelectedPartID.IsNone())
		return nullptr;

	if (!CachedBuildingDataTable)
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("BuildingDataTable is null"));
		return nullptr;
	}

	// DataTable에서 PartID로 조회
	FBuildingPartData* PartData = CachedBuildingDataTable->FindRow<FBuildingPartData>(SelectedPartID, TEXT("GetCurrentPartData"));

	if (!PartData)
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("Part data not found in DataTable: %s"), *SelectedPartID.ToString());
	}

	return PartData;
}

void UHarmoniaBuildingComponent::SetupInput()
{
	// TODO: Enhanced Input 시스템 연동
	// - 배치 액션
	// - 회전 액션
	// - 취소 액션
}

void UHarmoniaBuildingComponent::HandlePlaceAction()
{
	PlaceCurrentPart();
}

void UHarmoniaBuildingComponent::HandleRotateAction()
{
	if (PreviewActor)
	{
		CurrentRotationYaw += 90.0f; // 90도씩 회전
		if (CurrentRotationYaw >= 360.0f)
		{
			CurrentRotationYaw -= 360.0f;
		}
		UE_LOG(LogBuildingSystem, Log, TEXT("Rotated preview: %.1f degrees"), CurrentRotationYaw);
	}
}

void UHarmoniaBuildingComponent::HandleCancelAction()
{
	ExitBuildingMode();
}
