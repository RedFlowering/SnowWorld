// Copyright 2025 Snow Game Studio.

#include "Managers/HarmoniaBuildingInstanceManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/Guid.h"
#include "HarmoniaLoadManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildingInstanceManager, Log, All);

void UHarmoniaBuildingInstanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// ISM을 관리할 더미 액터 생성
	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = FName(TEXT("BuildingISMManager"));
		ISMManagerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (ISMManagerActor)
		{
			ISMManagerActor->SetActorLabel(TEXT("BuildingISMManager"));
		}
	}

	// BuildingDataTable 로드
	if (UHarmoniaLoadManager* LoadManager = UHarmoniaLoadManager::Get())
	{
		BuildingDataTable = LoadManager->GetDataTableByKey(FName(TEXT("BuildingParts")));
		if (!BuildingDataTable)
		{
			UE_LOG(LogBuildingInstanceManager, Warning, TEXT("Failed to load BuildingDataTable"));
		}
	}

	UE_LOG(LogBuildingInstanceManager, Log, TEXT("Building Instance Manager Initialized"));
}

void UHarmoniaBuildingInstanceManager::Deinitialize()
{
	// ISM 매니저 액터 정리
	if (ISMManagerActor)
	{
		ISMManagerActor->Destroy();
		ISMManagerActor = nullptr;
	}

	PartToISMMap.Empty();
	BuildingMetadataMap.Empty();

	Super::Deinitialize();

	UE_LOG(LogBuildingInstanceManager, Log, TEXT("Building Instance Manager Deinitialized"));
}

FGuid UHarmoniaBuildingInstanceManager::PlaceBuilding(const FBuildingPartData& PartData, const FVector& Location, const FRotator& Rotation, AActor* Owner)
{
	// Server-only execution
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogBuildingInstanceManager, Warning, TEXT("PlaceBuilding called on client - this is a server-only function"));
		return FGuid();
	}

	if (!ISMManagerActor)
	{
		UE_LOG(LogBuildingInstanceManager, Error, TEXT("ISMManagerActor is null. Cannot place building."));
		return FGuid();
	}

	// ISM 컴포넌트 가져오기 또는 생성
	UInstancedStaticMeshComponent* ISMComponent = nullptr;

	if (PartToISMMap.Contains(PartData.ID))
	{
		ISMComponent = PartToISMMap[PartData.ID];
	}
	else if (PartData.PreviewMesh.Mesh)
	{
		InitializeISMComponent(PartData.ID, PartData.PreviewMesh.Mesh);
		ISMComponent = PartToISMMap.FindRef(PartData.ID);
	}

	if (!ISMComponent)
	{
		UE_LOG(LogBuildingInstanceManager, Error, TEXT("Failed to get or create ISM component for PartID: %s"), *PartData.ID.ToString());
		return FGuid();
	}

	// 트랜스폼 생성
	FTransform InstanceTransform(Rotation, Location);

	// ISM에 인스턴스 추가
	int32 InstanceIndex = ISMComponent->AddInstance(InstanceTransform);

	if (InstanceIndex == INDEX_NONE)
	{
		UE_LOG(LogBuildingInstanceManager, Error, TEXT("Failed to add instance to ISM"));
		return FGuid();
	}

	// 고유 GUID 생성
	FGuid NewGuid = FGuid::NewGuid();

	// 인스턴스 데이터 생성
	FHarmoniaInstancedObjectData InstanceData;
	InstanceData.InstanceGuid = NewGuid;
	InstanceData.ObjectType = EHarmoniaInstancedObjectType::BuildingPart;
	InstanceData.DataId = PartData.ID;
	InstanceData.WorldTransform = InstanceTransform;
	InstanceData.Quantity = 1;

	// 베이스 맵에 추가
	InstanceMap.Add(NewGuid, InstanceData);

	// 메타데이터 생성
	FBuildingInstanceMetadata Metadata;
	Metadata.InstanceIndex = InstanceIndex;
	Metadata.PartID = PartData.ID;
	Metadata.Location = Location;
	Metadata.Rotation = Rotation;
	Metadata.Health = 100.0f; // 기본 내구도
	Metadata.OwnerPlayerID = Owner ? Owner->GetName() : TEXT("");

	BuildingMetadataMap.Add(NewGuid, Metadata);

	UE_LOG(LogBuildingInstanceManager, Log, TEXT("Building placed: %s at %s (GUID: %s, Instance: %d)"),
		*PartData.ID.ToString(), *Location.ToString(), *NewGuid.ToString(), InstanceIndex);

	return NewGuid;
}

bool UHarmoniaBuildingInstanceManager::RemoveBuilding(const FGuid& BuildingGuid)
{
	// Server-only execution
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogBuildingInstanceManager, Warning, TEXT("RemoveBuilding called on client - this is a server-only function"));
		return false;
	}

	if (!BuildingMetadataMap.Contains(BuildingGuid))
	{
		UE_LOG(LogBuildingInstanceManager, Warning, TEXT("Building not found: %s"), *BuildingGuid.ToString());
		return false;
	}

	const FBuildingInstanceMetadata& Metadata = BuildingMetadataMap[BuildingGuid];

	// ISM 컴포넌트에서 인스턴스 제거
	if (UInstancedStaticMeshComponent** ISMPtr = PartToISMMap.Find(Metadata.PartID))
	{
		if (*ISMPtr)
		{
			(*ISMPtr)->RemoveInstance(Metadata.InstanceIndex);

			// 인덱스 재정렬 - 제거된 인스턴스 이후의 모든 메타데이터 인덱스 업데이트
			for (auto& Pair : BuildingMetadataMap)
			{
				if (Pair.Value.PartID == Metadata.PartID && Pair.Value.InstanceIndex > Metadata.InstanceIndex)
				{
					Pair.Value.InstanceIndex--;
				}
			}
		}
	}

	// 데이터 제거
	InstanceMap.Remove(BuildingGuid);
	BuildingMetadataMap.Remove(BuildingGuid);

	UE_LOG(LogBuildingInstanceManager, Log, TEXT("Building removed: %s"), *BuildingGuid.ToString());
	return true;
}

bool UHarmoniaBuildingInstanceManager::RepairBuilding(const FGuid& BuildingGuid, float RepairAmount)
{
	// Server-only execution
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogBuildingInstanceManager, Warning, TEXT("RepairBuilding called on client - this is a server-only function"));
		return false;
	}

	if (!BuildingMetadataMap.Contains(BuildingGuid))
		return false;

	FBuildingInstanceMetadata& Metadata = BuildingMetadataMap[BuildingGuid];
	Metadata.Health = FMath::Min(Metadata.Health + RepairAmount, 100.0f);

	UE_LOG(LogBuildingInstanceManager, Log, TEXT("Building repaired: %s (Health: %.1f)"),
		*BuildingGuid.ToString(), Metadata.Health);

	return true;
}

bool UHarmoniaBuildingInstanceManager::DamageBuilding(const FGuid& BuildingGuid, float DamageAmount)
{
	// Server-only execution
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		UE_LOG(LogBuildingInstanceManager, Warning, TEXT("DamageBuilding called on client - this is a server-only function"));
		return false;
	}

	if (!BuildingMetadataMap.Contains(BuildingGuid))
		return false;

	FBuildingInstanceMetadata& Metadata = BuildingMetadataMap[BuildingGuid];
	Metadata.Health = FMath::Max(Metadata.Health - DamageAmount, 0.0f);

	UE_LOG(LogBuildingInstanceManager, Log, TEXT("Building damaged: %s (Health: %.1f)"),
		*BuildingGuid.ToString(), Metadata.Health);

	// 내구도가 0이 되면 자동 파괴
	if (Metadata.Health <= 0.0f)
	{
		UE_LOG(LogBuildingInstanceManager, Warning, TEXT("Building destroyed due to zero health: %s"),
			*BuildingGuid.ToString());
		RemoveBuilding(BuildingGuid);
		return false;
	}

	return true;
}

bool UHarmoniaBuildingInstanceManager::GetBuildingMetadata(const FGuid& BuildingGuid, FBuildingInstanceMetadata& OutMetadata) const
{
	if (const FBuildingInstanceMetadata* Metadata = BuildingMetadataMap.Find(BuildingGuid))
	{
		OutMetadata = *Metadata;
		return true;
	}
	return false;
}

bool UHarmoniaBuildingInstanceManager::CheckBuildingOverlap(const FVector& Location, const FRotator& Rotation, const FVector& BoundsExtent, float MinDistance) const
{
	// 모든 배치된 건축물과 충돌 검사
	for (const auto& Pair : BuildingMetadataMap)
	{
		const FBuildingInstanceMetadata& ExistingBuilding = Pair.Value;

		// 거리 기반 충돌 검사 (빠른 체크)
		float Distance = FVector::Dist(Location, ExistingBuilding.Location);

		// MinDistance보다 가까우면 충돌로 간주
		if (Distance < MinDistance)
		{
			UE_LOG(LogBuildingInstanceManager, Warning, TEXT("Building too close: Distance=%.1f, Min=%.1f"), Distance, MinDistance);
			return true;
		}

		// 박스 오버랩 검사 (BoundsExtent 사용)
		if (BoundsExtent.SizeSquared() > 0.0f)
		{
			// 기존 건축물의 BoundsExtent를 가져오기 위해 BuildingDataTable 조회 필요
			// 지금은 간단하게 거리 기반으로만 검사
			// TODO: 실제 박스 오버랩 검사 구현 시 BuildingDataTable에서 기존 건축물의 BoundsExtent 조회
		}
	}

	return false; // 충돌 없음
}

void UHarmoniaBuildingInstanceManager::GetAllBuildingMetadata(TArray<FBuildingInstanceMetadata>& OutMetadataArray) const
{
	OutMetadataArray.Empty();
	OutMetadataArray.Reserve(BuildingMetadataMap.Num());

	for (const auto& Pair : BuildingMetadataMap)
	{
		OutMetadataArray.Add(Pair.Value);
	}
}

bool UHarmoniaBuildingInstanceManager::FindNearbySnapPoint(const FVector& TargetLocation, EBuildingPartType PartType, float SearchRadius, FVector& OutSnapLocation, FRotator& OutSnapRotation) const
{
	if (!BuildingDataTable)
	{
		return false;
	}

	float ClosestDistance = SearchRadius;
	bool bFoundSnapPoint = false;

	// 모든 배치된 건축물을 순회
	for (const auto& Pair : BuildingMetadataMap)
	{
		const FBuildingInstanceMetadata& ExistingBuilding = Pair.Value;

		// 거리 체크 - 검색 반경 내에 있는지
		float Distance = FVector::Dist(TargetLocation, ExistingBuilding.Location);
		if (Distance > SearchRadius)
		{
			continue;
		}

		// 기존 건축물의 데이터 가져오기
		FBuildingPartData* ExistingPartData = BuildingDataTable->FindRow<FBuildingPartData>(ExistingBuilding.PartID, TEXT("FindNearbySnapPoint"));
		if (!ExistingPartData || ExistingPartData->SnapPoints.Num() == 0)
		{
			continue;
		}

		// 각 스냅 포인트 검사
		for (const FBuildingSnapPoint& SnapPoint : ExistingPartData->SnapPoints)
		{
			// 스냅 포인트가 현재 배치하려는 건축물 타입을 허용하는지 확인
			if (SnapPoint.AcceptsType != EBuildingPartType::None && SnapPoint.AcceptsType != PartType)
			{
				continue;
			}

			// 스냅 포인트의 월드 위치 계산
			FTransform BuildingTransform(ExistingBuilding.Rotation, ExistingBuilding.Location);
			FVector SnapWorldLocation = BuildingTransform.TransformPosition(SnapPoint.LocalOffset);
			FRotator SnapWorldRotation = (BuildingTransform.GetRotation() * SnapPoint.LocalRotation.Quaternion()).Rotator();

			// 타겟 위치와 스냅 포인트 간의 거리 계산
			float SnapDistance = FVector::Dist(TargetLocation, SnapWorldLocation);

			if (SnapDistance < ClosestDistance)
			{
				ClosestDistance = SnapDistance;
				OutSnapLocation = SnapWorldLocation;
				OutSnapRotation = SnapWorldRotation;
				bFoundSnapPoint = true;

				UE_LOG(LogBuildingInstanceManager, Log, TEXT("Found snap point: Distance=%.1f at %s"),
					SnapDistance, *SnapWorldLocation.ToString());
			}
		}
	}

	return bFoundSnapPoint;
}

AActor* UHarmoniaBuildingInstanceManager::SpawnWorldActor(const FHarmoniaInstancedObjectData& Data, AController* Requestor)
{
	// TODO: 필요시 인스턴스를 실제 액터로 변환
	// 현재는 모든 건축물을 ISM으로만 관리
	UE_LOG(LogBuildingInstanceManager, Log, TEXT("SpawnWorldActor called for building: %s (currently managed as ISM)"),
		*Data.DataId.ToString());

	return nullptr;
}

void UHarmoniaBuildingInstanceManager::DestroyWorldActor(AActor* Actor)
{
	// TODO: 액터 -> 인스턴스 변환 시 구현
	if (Actor)
	{
		Actor->Destroy();
	}
}

void UHarmoniaBuildingInstanceManager::InitializeISMComponent(const FName& PartID, UStaticMesh* Mesh)
{
	if (!ISMManagerActor || !Mesh)
		return;

	// 새 ISM 컴포넌트 생성
	UInstancedStaticMeshComponent* NewISM = NewObject<UInstancedStaticMeshComponent>(ISMManagerActor,
		UInstancedStaticMeshComponent::StaticClass(),
		*FString::Printf(TEXT("ISM_%s"), *PartID.ToString()));

	if (NewISM)
	{
		NewISM->SetStaticMesh(Mesh);
		NewISM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		NewISM->SetCastShadow(true);
		NewISM->RegisterComponent();
		NewISM->AttachToComponent(ISMManagerActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		PartToISMMap.Add(PartID, NewISM);

		UE_LOG(LogBuildingInstanceManager, Log, TEXT("ISM Component created for PartID: %s"), *PartID.ToString());
	}
}
