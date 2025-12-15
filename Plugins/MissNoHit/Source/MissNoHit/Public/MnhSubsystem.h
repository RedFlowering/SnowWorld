// Copyright 2024 Eren Balatkan. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MnhData.h"
#include "Subsystems/WorldSubsystem.h"
#include "MnhSubsystem.generated.h"

UCLASS()
class MISSNOHIT_API UMnhSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static UMnhSubsystem* GetMnhSubsystem(const UWorld* World);
	FMnhTracerData* GetTracerDataAt(int32 Index);
	int32 RequestNewTracerData();
	void MarkTracerDataForRemoval(int32 TracerDataIdx, FGuid Guid);

protected:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

private:
	FCriticalSection CriticalSection;
	TArray<FMnhTracerData> TracerDatas;
	bool RemovalLock = false;
	uint32 TickIdx = 0;

	void RemoveTracerDataAt(int32 TracerDataIdx, FGuid Guid);
	void UpdateTracerTransforms(const float DeltaTime);
	void PerformTraces(const float DeltaTime);
	void NotifyTraceResults();
};
