// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaBuildingPreviewActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

AHarmoniaBuildingPreviewActor::AHarmoniaBuildingPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 메시 컴포?�트 ?�성
	PreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	RootComponent = PreviewMeshComponent;

	// 콜리??비활?�화 (?�리뷰는 충돌?��? ?�음)
	PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMeshComponent->SetCastShadow(false);

	// 반투�??�정
	PreviewMeshComponent->SetRenderCustomDepth(false);
}

void AHarmoniaBuildingPreviewActor::BeginPlay()
{
	Super::BeginPlay();

	// 초기 ?�태???�효?��? ?�음?�로 ?�정
	SetIsPlacementValid(false);
}

void AHarmoniaBuildingPreviewActor::ApplyPreviewData(const FHarmoniaBuildingPartData& PartData)
{
	CurrentPartData = PartData;

	if (!PreviewMeshComponent)
		return;

	// 메시 ?�정
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

	// 머티리얼 변�?(모든 머티리얼 ?�롯???�용)
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
