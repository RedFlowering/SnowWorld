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

// Client request functions
void UHarmoniaBuildingComponent::RequestEnterBuildingMode()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		EnterBuildingMode();
	}
	else
	{
		ServerEnterBuildingMode();
	}
}

void UHarmoniaBuildingComponent::RequestExitBuildingMode()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		ExitBuildingMode();
	}
	else
	{
		ServerExitBuildingMode();
	}
}

void UHarmoniaBuildingComponent::RequestSetBuildingMode(EBuildingMode NewMode)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		SetBuildingMode(NewMode);
	}
	else
	{
		ServerSetBuildingMode(NewMode);
	}
}

void UHarmoniaBuildingComponent::RequestSetSelectedPart(FName PartID)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		SetSelectedPart(PartID);
	}
	else
	{
		ServerSetSelectedPart(PartID);
	}
}

// Server-authoritative functions
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
		UE_LOG(LogBuildingSystem, Warning, TEXT("Cannot place: No valid surface found"));
		return false;
	}

	// 지형 경사 검사
	if (bCheckTerrainSlope)
	{
		FVector SurfaceNormal = HitResult.Normal;
		float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector)));

		if (SlopeAngle > MaxAllowedSlope)
		{
			UE_LOG(LogBuildingSystem, Warning, TEXT("Cannot place: Terrain too steep (%.1f degrees, max %.1f)"),
				SlopeAngle, MaxAllowedSlope);
			OutLocation = HitResult.Location;
			OutRotation = FRotator(0.0f, CameraRotation.Yaw + CurrentRotationYaw, 0.0f);
			return false;
		}
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

	// 스냅 포인트 검사 (다른 건축물과의 연결)
	if (InstanceManager && bPreferSnapPoints)
	{
		FBuildingPartData* PartData = GetCurrentPartData();
		if (PartData)
		{
			FVector SnapLocation;
			FRotator SnapRotation;

			// 주변 스냅 포인트 찾기
			bool bFoundSnap = InstanceManager->FindNearbySnapPoint(
				OutLocation,
				PartData->PartType,
				SnapSearchRadius,
				SnapLocation,
				SnapRotation
			);

			if (bFoundSnap)
			{
				// 스냅 포인트로 위치 조정
				OutLocation = SnapLocation;
				OutRotation = SnapRotation;
				UE_LOG(LogBuildingSystem, Log, TEXT("Snapped to nearby building"));
			}
		}
	}

	// 충돌 검사 (다른 건축물과 겹치는지)
	if (InstanceManager)
	{
		FBuildingPartData* PartData = GetCurrentPartData();
		if (PartData && PartData->BoundsExtent.SizeSquared() > 0.0f)
		{
			// 건축물 간 최소 거리 (BoundsExtent의 평균값 사용)
			float MinDistance = (PartData->BoundsExtent.X + PartData->BoundsExtent.Y) * 0.5f;

			// 기존 건축물과의 충돌 검사
			bool bHasOverlap = InstanceManager->CheckBuildingOverlap(OutLocation, OutRotation, PartData->BoundsExtent, PartData->PartType, MinDistance);

			if (bHasOverlap)
			{
				UE_LOG(LogBuildingSystem, Warning, TEXT("Cannot place: Overlaps with existing building"));
				return false; // 충돌 발생 - 배치 불가
			}
		}
	}

	return true;
}

void UHarmoniaBuildingComponent::PlaceCurrentPart()
{
	if (!PreviewActor || !GetOwner())
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("Cannot place: Missing required components"));
		return;
	}

	FVector Location;
	FRotator Rotation;

	if (!ValidatePlacement(Location, Rotation))
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("Cannot place: Invalid placement location"));
		return;
	}

	// Request server to place building
	if (GetOwner()->HasAuthority())
	{
		// Server: Perform placement directly
		FBuildingPartData* PartData = GetCurrentPartData();
		if (!PartData)
		{
			UE_LOG(LogBuildingSystem, Warning, TEXT("Cannot place: Part data not found"));
			return;
		}

		// 자원 검사 및 소비 (Server-only)
		if (!CheckAndConsumeResources(*PartData))
		{
			UE_LOG(LogBuildingSystem, Warning, TEXT("Cannot place: Insufficient resources"));
			return;
		}

		if (!InstanceManager)
		{
			UE_LOG(LogBuildingSystem, Warning, TEXT("Cannot place: InstanceManager not found"));
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
	else
	{
		// Client: Request server to place
		ServerPlacePart(Location, Rotation, SelectedPartID);
	}
}

bool UHarmoniaBuildingComponent::CheckAndConsumeResources(const FBuildingPartData& PartData)
{
	// Server-only execution
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("CheckAndConsumeResources called on client - this is a server-only function"));
		return false;
	}

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
				*Cost.Item.Id.ToString(), Cost.Count, CurrentCount);
			return false;
		}
	}

	// 2단계: 모든 자원 소비 (실패 시 롤백 지원)
	TArray<FBuildingResourceCost> ConsumedResources; // 롤백용 추적

	for (const FBuildingResourceCost& Cost : PartData.RequiredResources)
	{
		bool bSuccess = InventoryComponent->RemoveItem(Cost.Item, Cost.Count, 0.0f);
		if (!bSuccess)
		{
			UE_LOG(LogBuildingSystem, Error, TEXT("Failed to remove item: %s - Rolling back consumed resources"), *Cost.Item.Id.ToString());

			// 롤백: 이미 제거된 아이템들을 복구
			for (const FBuildingResourceCost& ConsumedCost : ConsumedResources)
			{
				InventoryComponent->AddItem(ConsumedCost.Item, ConsumedCost.Count, 0.0f);
				UE_LOG(LogBuildingSystem, Log, TEXT("Rolled back resource: %s x%d"), *ConsumedCost.Item.Id.ToString(), ConsumedCost.Count);
			}

			return false;
		}
		else
		{
			ConsumedResources.Add(Cost);
			UE_LOG(LogBuildingSystem, Log, TEXT("Consumed resource: %s x%d"), *Cost.Item.Id.ToString(), Cost.Count);
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
	RequestExitBuildingMode();
}

// Server RPC implementations
void UHarmoniaBuildingComponent::ServerEnterBuildingMode_Implementation()
{
	EnterBuildingMode();
}

bool UHarmoniaBuildingComponent::ServerEnterBuildingMode_Validate()
{
	// Basic validation - could add cooldown check or permission check here
	return true;
}

void UHarmoniaBuildingComponent::ServerExitBuildingMode_Implementation()
{
	ExitBuildingMode();
}

bool UHarmoniaBuildingComponent::ServerExitBuildingMode_Validate()
{
	return true;
}

void UHarmoniaBuildingComponent::ServerSetBuildingMode_Implementation(EBuildingMode NewMode)
{
	SetBuildingMode(NewMode);
}

bool UHarmoniaBuildingComponent::ServerSetBuildingMode_Validate(EBuildingMode NewMode)
{
	// Validate mode is valid enum value
	return true;
}

void UHarmoniaBuildingComponent::ServerSetSelectedPart_Implementation(FName PartID)
{
	SetSelectedPart(PartID);
}

bool UHarmoniaBuildingComponent::ServerSetSelectedPart_Validate(FName PartID)
{
	// Anti-cheat: Validate part exists in data table
	if (PartID.IsNone())
	{
		return true; // Allow clearing selection
	}

	if (!CachedBuildingDataTable)
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("[ANTI-CHEAT] ServerSetSelectedPart: BuildingDataTable not loaded"));
		return false;
	}

	FBuildingPartData* PartData = CachedBuildingDataTable->FindRow<FBuildingPartData>(PartID, TEXT("ServerSetSelectedPart_Validate"));
	if (!PartData)
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("[ANTI-CHEAT] ServerSetSelectedPart: Invalid PartID %s"), *PartID.ToString());
		return false;
	}

	return true;
}

void UHarmoniaBuildingComponent::ServerPlacePart_Implementation(const FVector& Location, const FRotator& Rotation, FName PartID)
{
	// Validate part ID
	if (PartID.IsNone() || !CachedBuildingDataTable)
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("ServerPlacePart: Invalid PartID or DataTable"));
		return;
	}

	FBuildingPartData* PartData = CachedBuildingDataTable->FindRow<FBuildingPartData>(PartID, TEXT("ServerPlacePart"));
	if (!PartData)
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("ServerPlacePart: Part data not found"));
		return;
	}

	// Check resources (server-authoritative)
	if (!CheckAndConsumeResources(*PartData))
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("ServerPlacePart: Insufficient resources"));
		return;
	}

	if (!InstanceManager)
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("ServerPlacePart: InstanceManager not found"));
		return;
	}

	// Place building
	FGuid BuildingGuid = InstanceManager->PlaceBuilding(*PartData, Location, Rotation, GetOwner());

	if (BuildingGuid.IsValid())
	{
		UE_LOG(LogBuildingSystem, Log, TEXT("Building placed successfully via RPC: %s at %s"),
			*PartData->ID.ToString(), *Location.ToString());
	}
	else
	{
		UE_LOG(LogBuildingSystem, Error, TEXT("Failed to place building via RPC"));
	}
}

bool UHarmoniaBuildingComponent::ServerPlacePart_Validate(const FVector& Location, const FRotator& Rotation, FName PartID)
{
	// Anti-cheat: Validate location is not too far from player
	if (AActor* Owner = GetOwner())
	{
		float Distance = FVector::Dist(Owner->GetActorLocation(), Location);
		if (Distance > MaxPlacementDistance * 2.0f) // Allow 2x buffer for network lag
		{
			UE_LOG(LogBuildingSystem, Warning, TEXT("[ANTI-CHEAT] ServerPlacePart: Location too far from player (%.1f > %.1f)"),
				Distance, MaxPlacementDistance * 2.0f);
			return false;
		}
	}

	// Validate PartID
	if (PartID.IsNone())
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("[ANTI-CHEAT] ServerPlacePart: Invalid PartID"));
		return false;
	}

	if (!CachedBuildingDataTable)
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("[ANTI-CHEAT] ServerPlacePart: BuildingDataTable not loaded"));
		return false;
	}

	FBuildingPartData* PartData = CachedBuildingDataTable->FindRow<FBuildingPartData>(PartID, TEXT("ServerPlacePart_Validate"));
	if (!PartData)
	{
		UE_LOG(LogBuildingSystem, Warning, TEXT("[ANTI-CHEAT] ServerPlacePart: Invalid PartID %s"), *PartID.ToString());
		return false;
	}

	return true;
}
