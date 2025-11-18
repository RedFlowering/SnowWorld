// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaBuildingPreviewActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

AHarmoniaBuildingPreviewActor::AHarmoniaBuildingPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 메시 컴포넌트 생성
	PreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	RootComponent = PreviewMeshComponent;

	// 콜리전 비활성화 (프리뷰는 충돌하지 않음)
	PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMeshComponent->SetCastShadow(false);

	// 반투명 설정
	PreviewMeshComponent->SetRenderCustomDepth(false);
}

void AHarmoniaBuildingPreviewActor::BeginPlay()
{
	Super::BeginPlay();

	// 초기 상태는 유효하지 않음으로 설정
	SetIsPlacementValid(false);
}

void AHarmoniaBuildingPreviewActor::ApplyPreviewData(const FBuildingPartData& PartData)
{
	CurrentPartData = PartData;

	if (!PreviewMeshComponent)
		return;

	// 메시 설정
	if (PartData.PreviewMesh.Mesh)
	{
		PreviewMeshComponent->SetStaticMesh(PartData.PreviewMesh.Mesh);
		PreviewMeshComponent->SetRelativeLocation(PartData.PreviewMesh.RelativeLocation);
		PreviewMeshComponent->SetRelativeRotation(PartData.PreviewMesh.RelativeRotation);
		PreviewMeshComponent->SetRelativeScale3D(PartData.PreviewMesh.PreviewScale);
	}
}

void AHarmoniaBuildingPreviewActor::SetIsPlacementValid(bool bIsValid)
{
	if (bIsCurrentlyValid == bIsValid)
		return;

	bIsCurrentlyValid = bIsValid;

	if (!PreviewMeshComponent)
		return;

	// 머티리얼 변경 (모든 머티리얼 슬롯에 적용)
	UMaterialInterface* MaterialToUse = bIsValid ? ValidPlacementMaterial : InvalidPlacementMaterial;
	if (MaterialToUse)
	{
		int32 NumMaterials = PreviewMeshComponent->GetNumMaterials();
		for (int32 i = 0; i < NumMaterials; ++i)
		{
			PreviewMeshComponent->SetMaterial(i, MaterialToUse);
		}
	}
}

void AHarmoniaBuildingPreviewActor::RotatePreview(float DeltaYaw)
{
	FRotator CurrentRotation = GetActorRotation();
	CurrentRotation.Yaw += DeltaYaw;
	SetActorRotation(CurrentRotation);
}
