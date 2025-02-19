// Copyright 2024 HGsofts, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CosmeticItemID.h"
#include "CosmeticStructure.h"
#include "CosmeticMeshStorage.generated.h"

/**
* Prevent duplicate generation of the same merged mesh.
* Depending on the project, you should call Clear at the right time.
**/
UCLASS(BlueprintType)
class COSMETICSYSTEM_API UCosmeticMeshStorage : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UCosmeticMeshStorage* Get(const UWorld* InWorld);

	USkeletalMesh* GetMergedMesh(const TSet<FCosmeticItemID>& MeshPartIDs);

	bool Equal(const TSet<FCosmeticItemID>& lhs, const TSet<FCosmeticItemID>& rhs);
	
	void RegisterMergedMesh(FMergedMeshHandle& Data);

	// remove sored merged meshes
	UFUNCTION(BlueprintCallable, Category = "CosmeticMeshStorage")
	void Clear();

	UFUNCTION(BlueprintCallable, Category = "CosmeticMeshStorage")
	int32 GetStoredMergedMeshCount() { return MergedMeshes.Num(); }

protected:
	UPROPERTY()
	TArray<FMergedMeshHandle> MergedMeshes;
};

