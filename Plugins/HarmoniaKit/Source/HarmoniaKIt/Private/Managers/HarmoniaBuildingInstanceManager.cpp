// Copyright 2025 Snow Game Studio.

#include "Managers/HarmoniaBuildingInstanceManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/Guid.h"

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
