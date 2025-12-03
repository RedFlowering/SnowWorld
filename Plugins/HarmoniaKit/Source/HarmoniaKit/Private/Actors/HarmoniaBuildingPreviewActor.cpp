// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaBuildingPreviewActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

AHarmoniaBuildingPreviewActor::AHarmoniaBuildingPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create mesh component
	PreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	RootComponent = PreviewMeshComponent;

	// Disable collision (preview has no collision)
	PreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewMeshComponent->SetCastShadow(false);

	// Set transparency
	PreviewMeshComponent->SetRenderCustomDepth(false);
}

void AHarmoniaBuildingPreviewActor::BeginPlay()
{
	Super::BeginPlay();

	// Set initial state as invalid
	SetIsPlacementValid(false);
}

void AHarmoniaBuildingPreviewActor::ApplyPreviewData(const FHarmoniaBuildingPartData& PartData)
{
	CurrentPartData = PartData;

	if (!PreviewMeshComponent)
		return;

	// Set mesh
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

	// Change material (apply to all material slots)
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
