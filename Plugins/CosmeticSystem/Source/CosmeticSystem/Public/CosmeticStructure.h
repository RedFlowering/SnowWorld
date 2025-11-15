// Copyright 2024 HGsofts, All Rights Reserved.

#pragma once

#include "Engine/DataTable.h"
#include "Engine/CollisionProfile.h"
#include "GameplayTagContainer.h"
#include "CosmeticItemID.h"
#include "CosmeticSkeletalMeshComponent.h"
#include "CosmeticGroomComponent.h"
#include "Animation/AnimInstance.h"
#include "CosmeticStructure.generated.h"

class USkeletalMesh;
class UPhysicsAsset;
class USkeleton;
class UGroomAsset;

/**
 * FCosmeticData
 */
USTRUCT(BlueprintType)
struct COSMETICSYSTEM_API FCosmeticData
{
	GENERATED_BODY()

public:
	// server has a retarget mesh for attaching weapons
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CosmeticSystem")
	FCosmeticItemID RetargetMeshPresetID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CosmeticSystem")
	TArray<FCosmeticItemID> CosmeticMeshPartIDs;
};

/**
 * FRetargetMeshData
 */
USTRUCT(BlueprintType)
struct COSMETICSYSTEM_API FRetargetMeshData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	FGameplayTag CharacterTypeTag;

	// The visual mesh is a face mesh or body mesh(not divided into upper and lower body)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	TSoftObjectPtr<USkeletalMesh> VisualMeshObject = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	TSubclassOf<UAnimInstance> AnimClass = nullptr;

	/* This is for mobile support.
	 * Max bones count is limited to 75 on mobile
	 * Need to set CosmeticActor
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	bool bUseSeparateFaceMesh = false;

	/* If the body mesh is not divided into upper and lower bodies, the body mesh may be used as a visual mesh.
	 * Otherwise, use the face mesh as the visual mesh
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	bool bIsFaceMeshMasterMesh = true;

	/* I don't recommend this because it's too slow during runtime.
	 * By default, it uses the skeleton of the Visual Mesh.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	bool bMergeSkeleton = false;

	// for physics asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic|Physics")
	bool bEnableCollision = false;

	/*
	* If this value is nullptr when merging and the mesh's physic asset value is nullptr, Merge fails
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic|Physics", meta = (EditCondition = "bEnableCollision", DisplayName = "NewPhysicsAssetObject"))
	TSoftObjectPtr<UPhysicsAsset> NewPhysicsAssetClass = nullptr;

	/*
	* There must be at least two meshes to which PhysAsset is assigned.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic|Physics", meta = (EditCondition = "bEnableCollision"))
	bool bMergePhysAsset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic|Physics", meta = (EditCondition = "bEnableCollision"))
	FName CollisionProfileName = UCollisionProfile::NoCollision_ProfileName;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic|Chracter Mesh")
	// float CapsuleRadius = 40.f;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic|Chracter Mesh")
	//float CapsuleHalfHeight = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic|Scale")
	FVector ParentMeshScale = FVector(1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic|Scale")
	FVector VisualMeshScale = FVector(1.0f);

	// Although the visual mesh is a face mesh, you must set the default face mesh		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic", meta = (Categories = "CosmeticSystem"))
	TMap<FGameplayTag, FCosmeticItemID> DefaultPartTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	TMap<FName, TSoftObjectPtr<UMaterialInterface>> Materials;
};

/**
 * FBodyMeshPartData
 */
USTRUCT(BlueprintType)
struct COSMETICSYSTEM_API FBodyMeshPartData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic", meta = (Categories = "CosmeticSystem"))
	FGameplayTag CharacterPartTag;

	// If the mesh has clothing data, it will not merge.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	bool bMergeable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	FName MergeTargetMeshTag = FName("MergeTargetRoot");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	TSoftObjectPtr<USkeletalMesh> BodyMeshObject;
};

/**
 * FMeshPartData
 */
USTRUCT(BlueprintType)
struct COSMETICSYSTEM_API FMeshPartData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// for multiple parts in one mesh 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic", meta = (Categories = "CosmeticSystem"))
	FGameplayTagContainer CharacterPartTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic", meta = (Categories = "CosmeticSystem"))
	TArray<FCosmeticItemID> RemovePartIDs;

	// If the mesh has clothing data, it will not merge.	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mergeable")
	bool bMergeable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	bool bUseMasterPoseComponent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic", meta = (EditCondition = "!bMergeable"))
	bool bUseCustomTransform = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic", meta = (EditCondition = "bUseCustomTransform"))
	FTransform AttachTransform;

	// use physics asset
	// Need to turn on the "Enable Collision" of FRetargetMeshData
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	//bool bSimulatePhysics = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic", meta = (EditCondition = "!bMergeable"))
	TSubclassOf<UCosmeticSkeletalMeshComponent> OverrideCosmeticSkeletalMeshComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic", meta = (EditCondition = "!bMergeable"))
	TSubclassOf<UCosmeticGroomComponent> OverrideCosmeticGroomComponent = nullptr;

	// if not Merge skeletal mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic", meta = (EditCondition = "!bMergeable"))
	FName AttachSocketName = NAME_None;

	// multiple targets for translucency, mask meshes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mergeable", meta = (EditCondition = "bMergeable"))
	FName MergeTargetMeshTag = FName("MergeTargetRoot");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic", meta = (EditCondition = "!bMergeable"))
	FName CollisionProfileName = UCollisionProfile::NoCollision_ProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic", meta = (DisplayName = "MeshPartObject"))
	TSoftObjectPtr<USkeletalMesh> MeshPartClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic", meta = (EditCondition = "!bMergeable"))
	TSubclassOf<UAnimInstance> AnimClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	TMap<FName, TSoftObjectPtr<UMaterialInterface>> Materials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic", meta = (EditCondition = "!bMergeable", DisplayName = "PhysicsAssetObject"))
	TSoftObjectPtr<UPhysicsAsset> PhysicsAssetClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Groom")
	TSoftObjectPtr<UGroomAsset> GroomAsset = nullptr;
};

/**
 * FCachedMeshPartData
 */
USTRUCT(BlueprintType)
struct COSMETICSYSTEM_API FCachedMeshPartData
{
	GENERATED_BODY()

public:
	FCachedMeshPartData() = default;

	FCosmeticItemID MeshPartID;

	UPROPERTY()
	TObjectPtr<USkeletalMesh> Mesh = nullptr;

	UPROPERTY()
	TObjectPtr<UGroomAsset> Groom = nullptr;

	UPROPERTY()
	TObjectPtr<UPhysicsAsset> PhysicsAsset = nullptr;

	FName MergeTargetMeshTag;

	FName AttachSocketName = NAME_None;

	FName CollisionProfileName = UCollisionProfile::NoCollision_ProfileName;

	bool bMergeable = true;

	bool bFakeData = false;

	bool bUseMasterPoseComponent = true;

	/* ClothSimulated is controlled by CosmeticSkeletalMeshComponent
	*
	bool bClothSimulation = false;
	TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled = ECollisionEnabled::PhysicsOnly;
	*/

	// use physics asset
	//bool bSimulatePhysics = false;

	bool bUseCustomTransform = false;

	UPROPERTY()
	TSubclassOf<UAnimInstance> AnimClass = nullptr;

	UPROPERTY()
	TMap<FName, TObjectPtr<UMaterialInterface>> Materials;

	FTransform AttachTransform = FTransform::Identity;

	TSubclassOf<UCosmeticSkeletalMeshComponent> OverrideSkeletalMeshComponent = nullptr;

	TSubclassOf<UCosmeticGroomComponent> OverrideGroomComponent = nullptr;
};

/**
 * FMeshesToMergeData
 */
USTRUCT(BlueprintType)
struct COSMETICSYSTEM_API FMeshesToMergeData
{
	GENERATED_BODY()

public:
	FGameplayTag PartTag;
	FCachedMeshPartData MeshPartData; // mesh
};

/**
 * FMeshesToMergeSkeletalMeshes
 */
USTRUCT(BlueprintType)
struct COSMETICSYSTEM_API FMeshesToMergeSkeletalMeshes
{
	GENERATED_BODY()

public:
	TSet<FCosmeticItemID> MeshPartIDs;
	TMap<FName, UMaterialInterface*> OverrideMaterials;
	TArray<USkeletalMesh*> Meshes;
};

/**
 * FMeshesToMergeSkeletalMeshes
 */
USTRUCT(BlueprintType)
struct COSMETICSYSTEM_API FMergedMeshHandle
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<USkeletalMesh> MergedMesh = nullptr;

	TSet<FCosmeticItemID> MeshPartIDs;
};

/**
 * FCosmeticMeshMergeParams
 */
USTRUCT(BlueprintType)
struct COSMETICSYSTEM_API FCosmeticMeshMergeParams
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	TArray<TObjectPtr<USkeletalMesh>> SkeletalMeshesList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	int32 RemoveTopLODs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	bool bUseCpuAccess;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	bool bUpdateSkeletonBefore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	TObjectPtr<UPhysicsAsset> MeshPhysAssetOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	TObjectPtr<USkeleton> SkeletonOverride;

	FCosmeticMeshMergeParams()
	{
		SkeletalMeshesList = TArray<USkeletalMesh*>();
		RemoveTopLODs = 0;
		bUseCpuAccess = false;
		bUpdateSkeletonBefore = false;
		MeshPhysAssetOverride = nullptr;
		SkeletonOverride = nullptr;
	}
};

USTRUCT(BlueprintType)
struct COSMETICSYSTEM_API FChangedCosmeticMeshes
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	TSet<FCosmeticItemID> DefaultMeshPartIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cosmetic")
	TSet<FCosmeticItemID> MeshPartIDs;
};