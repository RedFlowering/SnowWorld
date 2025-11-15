// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NumberRendererDefine.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "NumberISMComponent.generated.h"

// The position of the parent actor must be set to zero
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NUMBERRENDERER_API UNumberISMComponent : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:
	UNumberISMComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ResetInstancesTransforms(const TArray<FNumberInstance>& Instances, int32 MaxCount);

	virtual void ClearInstances() override;

protected:
	bool UpdateInstancesMesh(const TArray<FNumberInstance>& Instances);

	int32 MaxDrawCount = 300;
};
