// Copyright 2025 Snow Game Studio.

#include "Managers/HarmoniaBuildingInstanceManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Misc/Guid.h"
#include "HarmoniaLoadManager.h"
#include "Teams/LyraTeamAgentInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogBuildingInstanceManager, Log, All);

void UHarmoniaBuildingInstanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// ISM??관리할 ?��? ?�터 ?�성
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogBuildingInstanceManager, Error, TEXT("Failed to get World in Initialize - cannot create ISMManagerActor"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(TEXT("BuildingISMManager"));
	ISMManagerActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (!ISMManagerActor)
	{
		UE_LOG(LogBuildingInstanceManager, Error, TEXT("Failed to spawn ISMManagerActor - building system will not work"));
		return;
	}

	// AActor??기본?�으�?RootComponent가 ?�으므�??�성
	USceneComponent* RootComp = NewObject<USceneComponent>(ISMManagerActor, USceneComponent::StaticClass(), TEXT("RootComponent"));
	if (RootComp)
	{
		RootComp->RegisterComponent();
		ISMManagerActor->SetRootComponent(RootComp);
	}
	else
	{
		UE_LOG(LogBuildingInstanceManager, Error, TEXT("Failed to create RootComponent for ISMManagerActor"));
		ISMManagerActor->Destroy();
		ISMManagerActor = nullptr;
		return;
	}

#if WITH_EDITOR
	ISMManagerActor->SetActorLabel(TEXT("BuildingISMManager"));
#endif

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
	// ISM 매니?� ?�터 ?�리
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

FGuid UHarmoniaBuildingInstanceManager::PlaceBuilding(const FHarmoniaBuildingPartData& PartData, const FVector& Location, const FRotator& Rotation, AActor* Owner)
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

	// ISM 컴포?�트 가?�오�??�는 ?�성
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

	// ?�랜?�폼 ?�성
	FTransform InstanceTransform(Rotation, Location);

	// ISM???�스?�스 추�?
	int32 InstanceIndex = ISMComponent->AddInstance(InstanceTransform);

	if (InstanceIndex == INDEX_NONE)
	{
		UE_LOG(LogBuildingInstanceManager, Error, TEXT("Failed to add instance to ISM"));
		return FGuid();
	}

	// 고유 GUID ?�성
	FGuid NewGuid = FGuid::NewGuid();

	// ?�스?�스 ?�이???�성
	FHarmoniaInstancedObjectData InstanceData;
	InstanceData.InstanceGuid = NewGuid;
	InstanceData.ObjectType = EHarmoniaInstancedObjectType::BuildingPart;
	InstanceData.DataId = PartData.ID;
	InstanceData.WorldTransform = InstanceTransform;
	InstanceData.Quantity = 1;

	// 베이??맵에 추�?
	InstanceMap.Add(NewGuid, InstanceData);

	// 메�??�이???�성
	FBuildingInstanceMetadata Metadata;
	Metadata.InstanceIndex = InstanceIndex;
	Metadata.PartID = PartData.ID;
	Metadata.Location = Location;
	Metadata.Rotation = Rotation;
	Metadata.Health = 100.0f; // 기본 ?�구??
	Metadata.OwnerPlayerID = Owner ? Owner->GetName() : TEXT("");

	// [TEAM SUPPORT] Set team ownership and sharing
	Metadata.OwnerTeamID = INDEX_NONE;
	Metadata.bSharedWithTeam = false;

	if (Owner)
	{
		// Get team ID from owner
		if (ILyraTeamAgentInterface* TeamAgent = Cast<ILyraTeamAgentInterface>(Owner))
		{
			FGenericTeamId TeamId = TeamAgent->GetGenericTeamId();
			Metadata.OwnerTeamID = GenericTeamIdToInteger(TeamId);
		}
		// Try to get from pawn's controller
		else if (APawn* Pawn = Cast<APawn>(Owner))
		{
			if (Pawn->GetController())
			{
				if (ILyraTeamAgentInterface* ControllerTeamAgent = Cast<ILyraTeamAgentInterface>(Pawn->GetController()))
				{
					FGenericTeamId TeamId = ControllerTeamAgent->GetGenericTeamId();
					Metadata.OwnerTeamID = GenericTeamIdToInteger(TeamId);
				}
			}
		}

		// Default: share with team if owner has a team
		if (Metadata.OwnerTeamID != INDEX_NONE)
		{
			Metadata.bSharedWithTeam = true;
		}
	}

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

	// ISM 컴포?�트?�서 ?�스?�스 ?�거
	if (UInstancedStaticMeshComponent** ISMPtr = PartToISMMap.Find(Metadata.PartID))
	{
		if (*ISMPtr)
		{
			(*ISMPtr)->RemoveInstance(Metadata.InstanceIndex);

			// ?�덱???�정??- ?�거???�스?�스 ?�후??모든 메�??�이???�덱???�데?�트
			for (auto& Pair : BuildingMetadataMap)
			{
				if (Pair.Value.PartID == Metadata.PartID && Pair.Value.InstanceIndex > Metadata.InstanceIndex)
				{
					Pair.Value.InstanceIndex--;
				}
			}
		}
	}

	// ?�이???�거
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

	// ?�구?��? 0???�면 ?�동 ?�괴
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

bool UHarmoniaBuildingInstanceManager::CheckBuildingOverlap(const FVector& Location, const FRotator& Rotation, const FVector& BoundsExtent, EBuildingPartType PlacingPartType, float MinDistance) const
{
	if (!BuildingDataTable)
	{
		UE_LOG(LogBuildingInstanceManager, Warning, TEXT("BuildingDataTable is null - cannot perform overlap check"));
		return false;
	}

	// 배치?�려??건축물의 박스 변??
	FTransform PlacingTransform(Rotation, Location);

	// 모든 배치??건축물과 충돌 검??
	for (const auto& Pair : BuildingMetadataMap)
	{
		const FBuildingInstanceMetadata& ExistingBuilding = Pair.Value;

		// 기존 건축�??�이??조회
		FHarmoniaBuildingPartData* ExistingPartData = BuildingDataTable->FindRow<FHarmoniaBuildingPartData>(ExistingBuilding.PartID, TEXT("CheckOverlap"));
		if (!ExistingPartData)
		{
			continue;
		}

		// ?�?�별 ?�버???�용 규칙 체크
		if (IsOverlapAllowed(PlacingPartType, ExistingPartData->PartType))
		{
			// ??조합?� ?�버???�용 (?? 벽걸???�식??+ �?
			continue;
		}

		// 거리 기반 충돌 검??(MinDistance가 ?�수???�만)
		if (MinDistance >= 0.0f)
		{
			float Distance = FVector::Dist(Location, ExistingBuilding.Location);
			if (Distance < MinDistance)
			{
				UE_LOG(LogBuildingInstanceManager, Warning, TEXT("Building too close: Distance=%.1f, Min=%.1f"), Distance, MinDistance);
				return true; // 충돌 발생
			}
		}

		// 박스 ?�버??검??
		if (BoundsExtent.SizeSquared() > 0.0f && ExistingPartData->BoundsExtent.SizeSquared() > 0.0f)
		{
			// 기존 건축물의 박스 변??
			FTransform ExistingTransform(ExistingBuilding.Rotation, ExistingBuilding.Location);

			// ??박스가 ?�버?�되?��? 검??
			if (DoBoxesOverlap(PlacingTransform, BoundsExtent, ExistingTransform, ExistingPartData->BoundsExtent))
			{
				UE_LOG(LogBuildingInstanceManager, Warning, TEXT("Building box overlap detected at %s"), *Location.ToString());
				return true; // 충돌 발생
			}
		}
	}

	return false; // 충돌 ?�음 - 배치 가??
}

bool UHarmoniaBuildingInstanceManager::IsOverlapAllowed(EBuildingPartType PlacingType, EBuildingPartType ExistingType) const
{
	// 벽걸???�식?��? 벽과 ?�버??가??
	if (PlacingType == EBuildingPartType::WallDecoration && ExistingType == EBuildingPartType::Wall)
	{
		return true;
	}

	// 문�? 벽과 ?�버??가??
	if (PlacingType == EBuildingPartType::Door && ExistingType == EBuildingPartType::Wall)
	{
		return true;
	}

	// 창문?� 벽과 ?�버??가??
	if (PlacingType == EBuildingPartType::Window && ExistingType == EBuildingPartType::Wall)
	{
		return true;
	}

	// 기�? 조합?� ?�버??불�?
	return false;
}

bool UHarmoniaBuildingInstanceManager::DoBoxesOverlap(const FTransform& TransformA, const FVector& ExtentA, const FTransform& TransformB, const FVector& ExtentB) const
{
	// Oriented Bounding Box (OBB) 충돌 검??
	// 간단??구현: �?박스??8�?코너�?변?�하�?분리�??�리(SAT)�??�용

	// 박스 A??로컬 좌표�?�?
	FVector AxisA[3] = {
		TransformA.GetRotation().GetAxisX(),
		TransformA.GetRotation().GetAxisY(),
		TransformA.GetRotation().GetAxisZ()
	};

	// 박스 B??로컬 좌표�?�?
	FVector AxisB[3] = {
		TransformB.GetRotation().GetAxisX(),
		TransformB.GetRotation().GetAxisY(),
		TransformB.GetRotation().GetAxisZ()
	};

	// ??박스 중심 간의 벡터
	FVector T = TransformB.GetLocation() - TransformA.GetLocation();

	// 분리�??�리(SAT) 검??
	// 15개의 축을 검?? A??3�? B??3�? ?�적?�로 만들?�진 9�?

	// 박스 A??�?검??
	for (int i = 0; i < 3; i++)
	{
		float ra = ExtentA[i];
		float rb = ExtentB.X * FMath::Abs(FVector::DotProduct(AxisB[0], AxisA[i])) +
		           ExtentB.Y * FMath::Abs(FVector::DotProduct(AxisB[1], AxisA[i])) +
		           ExtentB.Z * FMath::Abs(FVector::DotProduct(AxisB[2], AxisA[i]));

		if (FMath::Abs(FVector::DotProduct(T, AxisA[i])) > ra + rb)
		{
			return false; // 분리�?발견 - ?�버???�음
		}
	}

	// 박스 B??�?검??
	for (int i = 0; i < 3; i++)
	{
		float ra = ExtentA.X * FMath::Abs(FVector::DotProduct(AxisA[0], AxisB[i])) +
		           ExtentA.Y * FMath::Abs(FVector::DotProduct(AxisA[1], AxisB[i])) +
		           ExtentA.Z * FMath::Abs(FVector::DotProduct(AxisA[2], AxisB[i]));
		float rb = ExtentB[i];

		if (FMath::Abs(FVector::DotProduct(T, AxisB[i])) > ra + rb)
		{
			return false; // 분리�?발견 - ?�버???�음
		}
	}

	// ?�적 �?검??(9�?
	// 간단??구현???�해 ?��? 축만 검??(?�능 최적??가??
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			FVector Axis = FVector::CrossProduct(AxisA[i], AxisB[j]);
			if (Axis.SizeSquared() < 1e-6f)
			{
				continue; // ?�행??축�? ?�킵
			}
			Axis.Normalize();

			float ra = ExtentA.X * FMath::Abs(FVector::DotProduct(AxisA[0], Axis)) +
			           ExtentA.Y * FMath::Abs(FVector::DotProduct(AxisA[1], Axis)) +
			           ExtentA.Z * FMath::Abs(FVector::DotProduct(AxisA[2], Axis));

			float rb = ExtentB.X * FMath::Abs(FVector::DotProduct(AxisB[0], Axis)) +
			           ExtentB.Y * FMath::Abs(FVector::DotProduct(AxisB[1], Axis)) +
			           ExtentB.Z * FMath::Abs(FVector::DotProduct(AxisB[2], Axis));

			if (FMath::Abs(FVector::DotProduct(T, Axis)) > ra + rb)
			{
				return false; // 분리�?발견 - ?�버???�음
			}
		}
	}

	return true; // 모든 �?검???�과 - ?�버??발생
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

	// 모든 배치??건축물을 ?�회
	for (const auto& Pair : BuildingMetadataMap)
	{
		const FBuildingInstanceMetadata& ExistingBuilding = Pair.Value;

		// 거리 체크 - 검??반경 ?�에 ?�는지
		float Distance = FVector::Dist(TargetLocation, ExistingBuilding.Location);
		if (Distance > SearchRadius)
		{
			continue;
		}

		// 기존 건축물의 ?�이??가?�오�?
		FHarmoniaBuildingPartData* ExistingPartData = BuildingDataTable->FindRow<FHarmoniaBuildingPartData>(ExistingBuilding.PartID, TEXT("FindNearbySnapPoint"));
		if (!ExistingPartData || ExistingPartData->SnapPoints.Num() == 0)
		{
			continue;
		}

		// �??�냅 ?�인??검??
		for (const FBuildingSnapPoint& SnapPoint : ExistingPartData->SnapPoints)
		{
			// ?�냅 ?�인?��? ?�재 배치?�려??건축�??�?�을 ?�용?�는지 ?�인
			if (SnapPoint.AcceptsType != EBuildingPartType::None && SnapPoint.AcceptsType != PartType)
			{
				continue;
			}

			// ?�냅 ?�인?�의 ?�드 ?�치 계산
			FTransform BuildingTransform(ExistingBuilding.Rotation, ExistingBuilding.Location);
			FVector SnapWorldLocation = BuildingTransform.TransformPosition(SnapPoint.LocalOffset);
			FRotator SnapWorldRotation = (BuildingTransform.GetRotation() * SnapPoint.LocalRotation.Quaternion()).Rotator();

			// ?��??�치?� ?�냅 ?�인??간의 거리 계산
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
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogBuildingInstanceManager, Error, TEXT("World is null - cannot spawn building actor"));
		return nullptr;
	}

	if (!BuildingDataTable)
	{
		UE_LOG(LogBuildingInstanceManager, Error, TEXT("BuildingDataTable is null - cannot spawn building actor"));
		return nullptr;
	}

	// Get building part data from data table
	FHarmoniaBuildingPartData* PartData = BuildingDataTable->FindRow<FHarmoniaBuildingPartData>(Data.DataId, TEXT("SpawnWorldActor"));
	if (!PartData || !PartData->PreviewMesh.Mesh)
	{
		UE_LOG(LogBuildingInstanceManager, Error, TEXT("Invalid part data or mesh for PartID: %s"), *Data.DataId.ToString());
		return nullptr;
	}

	// Spawn a simple actor to represent the building part
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*FString::Printf(TEXT("BuildingPart_%s"), *Data.InstanceGuid.ToString()));
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = World->SpawnActor<AActor>(AActor::StaticClass(),
		Data.WorldTransform.GetLocation(),
		Data.WorldTransform.GetRotation().Rotator(),
		SpawnParams);

	if (!NewActor)
	{
		UE_LOG(LogBuildingInstanceManager, Error, TEXT("Failed to spawn building actor"));
		return nullptr;
	}

	// Create and attach static mesh component
	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(NewActor,
		UStaticMeshComponent::StaticClass(),
		TEXT("BuildingMesh"));

	if (MeshComponent)
	{
		MeshComponent->SetStaticMesh(PartData->PreviewMesh.Mesh);
		MeshComponent->SetRelativeLocation(PartData->PreviewMesh.RelativeLocation);
		MeshComponent->SetRelativeRotation(PartData->PreviewMesh.RelativeRotation);
		MeshComponent->SetWorldScale3D(PartData->PreviewMesh.PreviewScale);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCastShadow(true);
		MeshComponent->RegisterComponent();

		NewActor->SetRootComponent(MeshComponent);
	}

	// Also hide the corresponding ISM instance (if we want to avoid duplicate visuals)
	// This can be implemented based on requirements

	UE_LOG(LogBuildingInstanceManager, Log, TEXT("Building actor spawned: %s at %s (GUID: %s)"),
		*Data.DataId.ToString(), *Data.WorldTransform.GetLocation().ToString(), *Data.InstanceGuid.ToString());

	return NewActor;
}

void UHarmoniaBuildingInstanceManager::DestroyWorldActor(AActor* Actor)
{
	if (!Actor)
	{
		UE_LOG(LogBuildingInstanceManager, Warning, TEXT("DestroyWorldActor called with null actor"));
		return;
	}

	UE_LOG(LogBuildingInstanceManager, Log, TEXT("Destroying building actor: %s"), *Actor->GetName());

	// If we hid the ISM instance when spawning the actor, we should unhide it here
	// This can be implemented based on requirements when ISM hiding is implemented

	Actor->Destroy();
}

void UHarmoniaBuildingInstanceManager::InitializeISMComponent(const FName& PartID, UStaticMesh* Mesh)
{
	if (!ISMManagerActor || !Mesh)
	{
		UE_LOG(LogBuildingInstanceManager, Warning, TEXT("Cannot initialize ISM component - ISMManagerActor or Mesh is null"));
		return;
	}

	USceneComponent* RootComponent = ISMManagerActor->GetRootComponent();
	if (!RootComponent)
	{
		UE_LOG(LogBuildingInstanceManager, Error, TEXT("ISMManagerActor has no RootComponent - cannot attach ISM components"));
		return;
	}

	// ??ISM 컴포?�트 ?�성
	UInstancedStaticMeshComponent* NewISM = NewObject<UInstancedStaticMeshComponent>(ISMManagerActor,
		UInstancedStaticMeshComponent::StaticClass(),
		*FString::Printf(TEXT("ISM_%s"), *PartID.ToString()));

	if (NewISM)
	{
		NewISM->SetStaticMesh(Mesh);
		NewISM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		NewISM->SetCastShadow(true);
		NewISM->RegisterComponent();
		NewISM->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

		PartToISMMap.Add(PartID, NewISM);

		UE_LOG(LogBuildingInstanceManager, Log, TEXT("ISM Component created for PartID: %s"), *PartID.ToString());
	}
}
