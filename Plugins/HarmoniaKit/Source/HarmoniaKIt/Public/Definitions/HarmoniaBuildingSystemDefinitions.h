// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "Definitions/HarmoniaCoreDefinitions.h"

#include "HarmoniaBuildingSystemDefinitions.generated.h"

UENUM(BlueprintType)
enum class EBuildingPartType : uint8
{
    None            UMETA(DisplayName = "None"),
    Floor           UMETA(DisplayName = "Floor"),
    Wall            UMETA(DisplayName = "Wall"),
    Roof            UMETA(DisplayName = "Roof"),
    Pillar          UMETA(DisplayName = "Pillar"),
    Door            UMETA(DisplayName = "Door"),
    Window          UMETA(DisplayName = "Window"),
    Stairs          UMETA(DisplayName = "Stairs"),
    Fence           UMETA(DisplayName = "Fence"),
    WallDecoration  UMETA(DisplayName = "Wall Decoration")  // 벽걸이 장식품 (액자, 그림, 벽걸이 등)
};

UENUM(BlueprintType)
enum class EBuildingMode : uint8
{
    None        UMETA(DisplayName = "None"),
    Build       UMETA(DisplayName = "Build"),
    Destroy     UMETA(DisplayName = "Destroy"),
    Repair      UMETA(DisplayName = "Repair"),
    Rotate      UMETA(DisplayName = "Rotate"),
    Inspect     UMETA(DisplayName = "Inspect")
};

USTRUCT(BlueprintType)
struct FBuildingResourceCost
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FHarmoniaID Item = FHarmoniaID();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 0;

    FBuildingResourceCost() : Count(0) {}
    FBuildingResourceCost(FName InItemID, int32 InCount)
        : Item(InItemID), Count(InCount) {}
};

USTRUCT(BlueprintType)
struct FBuildingSnapPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap")
    FName SocketName = FName();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap")
    FVector LocalOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap")
    FRotator LocalRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap")
    EBuildingPartType AcceptsType = EBuildingPartType::None;
};

USTRUCT(BlueprintType)
struct FBuildingPreviewMesh
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<UStaticMesh> Mesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreviewScale = FVector(1.0f);
};

USTRUCT(BlueprintType)
struct FBuildingPartData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ID = FName();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingPartType PartType = EBuildingPartType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBuildingPreviewMesh PreviewMesh = FBuildingPreviewMesh();

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector BoundsExtent = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FBuildingSnapPoint> SnapPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FBuildingResourceCost> RequiredResources;
};

USTRUCT(BlueprintType)
struct FBuildingInstanceMetadata
{
    GENERATED_BODY()

    UPROPERTY()
    int32 InstanceIndex = 0;

    UPROPERTY()
    FName PartID = FName();

    UPROPERTY()
    FVector Location = FVector::ZeroVector;

    UPROPERTY()
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY()
    float Health = 0.0f;

    UPROPERTY()
    FString OwnerPlayerID = FString();
};

USTRUCT(BlueprintType)
struct FBuildingSaveRecord
{
    GENERATED_BODY()

    UPROPERTY()
    FName PartID;

    UPROPERTY()
    FVector SavedLocation;

    UPROPERTY()
    FRotator SavedRotation;

    UPROPERTY()
    float Health;

    UPROPERTY()
    FString OwnerPlayerID;
};