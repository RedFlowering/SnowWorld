// Copyright 2025 Snow Game Studio.

#pragma once

#include "Engine/DataTable.h"
#include "HarmoniaBuildingSystemDefinitions.generated.h"

UENUM(BlueprintType)
enum class EBuildingPartType : uint8
{
    None        UMETA(DisplayName = "None"),
    Floor       UMETA(DisplayName = "Floor"),
    Wall        UMETA(DisplayName = "Wall"),
    Roof        UMETA(DisplayName = "Roof"),
    Pillar      UMETA(DisplayName = "Pillar"),
    Door        UMETA(DisplayName = "Door"),
    Window      UMETA(DisplayName = "Window"),
    Stairs      UMETA(DisplayName = "Stairs"),
    Fence       UMETA(DisplayName = "Fence")
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
struct FItemID
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ID;

    FItemID() : ID(NAME_None) {}
    explicit FItemID(FName InID) : ID(InID) {}

    FORCEINLINE bool operator==(const FItemID& Other) const { return ID == Other.ID; }
    FORCEINLINE bool operator!=(const FItemID& Other) const { return ID != Other.ID; }

    friend FORCEINLINE uint32 GetTypeHash(const FItemID& Item)
    {
        return GetTypeHash(Item.ID);
    }
};

USTRUCT(BlueprintType)
struct FItemCount
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemID Item;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count;

    FItemCount() : Count(0) {}
    FItemCount(FName InItemID, int32 InCount)
        : Item(InItemID), Count(InCount) {}
};

USTRUCT(BlueprintType)
struct FBuildingSnapPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap")
    FName SocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap")
    FVector LocalOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap")
    FRotator LocalRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snap")
    EBuildingPartType AcceptsType;
};

USTRUCT(BlueprintType)
struct FBuildingPreviewMesh
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* Mesh;

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
    FName ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingPartType PartType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FBuildingPreviewMesh PreviewMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector BoundsExtent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FBuildingSnapPoint> SnapPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FItemCount> RequiredResources;
};

USTRUCT(BlueprintType)
struct FBuildingInstanceMetadata
{
    GENERATED_BODY()

    UPROPERTY()
    int32 InstanceIndex;

    UPROPERTY()
    FName PartID;

    UPROPERTY()
    FVector Location;

    UPROPERTY()
    FRotator Rotation;

    UPROPERTY()
    float Health;

    UPROPERTY()
    FString OwnerPlayerID;
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