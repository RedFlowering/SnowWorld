// Copyright 2022 HGsofts, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NumberRenderActor.h"
#include "WorldNumberManager.generated.h"
/**
 *
 */
UCLASS()
class NUMBERRENDERER_API UWorldNumberManager : public UObject
{
	GENERATED_BODY()

public:
	UWorldNumberManager();
	virtual ~UWorldNumberManager();

	virtual void BeginDestroy() override;
	virtual UWorld* GetWorld() const override final;

	void Init(UWorld* InWorld);

	ANumberRenderActor* GetRenderActor(ENumberFontType Type = ENumberFontType::NumberFontType_None);

	// the number 0 rendered at the PreSpawnLocation to load material resources.
	void Preload(FVector PreSpawnLocation);

protected:
	UPROPERTY()
	TObjectPtr<UWorld> GameWorld = nullptr;

	UPROPERTY()
	TMap<TEnumAsByte<enum ENumberFontType>, TObjectPtr<ANumberRenderActor>> RenderActors;
};
