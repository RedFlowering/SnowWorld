// Copyright 2024 HGsofts, All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "CosmeticSkeletalMeshComponent.h"
#include "CosmeticGroomComponent.h"
#include "CosmeticSystemSettings.generated.h"

UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "CosmeticSystem"))
class COSMETICSYSTEM_API UCosmeticSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static UCosmeticSystemSettings* Get() { return GetMutableDefault<UCosmeticSystemSettings>(); }
		
public:
	virtual void PostInitProperties() override;
	virtual FName GetCategoryName() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	// Prevent duplicate generation of the same merged mesh.
	UPROPERTY(EditAnywhere, config, Category = "CosmeticSystem")
	bool bUseMergedMeshStorage = false;

	// If there are more stored numbers than CheckClearCount before adding a new merged mesh, remove all merged meshes in the Storage
	UPROPERTY(EditAnywhere, config, Category = "CosmeticSystem|Storage")
	bool bCheckClear = true;

	UPROPERTY(EditAnywhere, config, Category = "CosmeticSystem|Storage")
	int32 CheckClearCount = 50;
};
