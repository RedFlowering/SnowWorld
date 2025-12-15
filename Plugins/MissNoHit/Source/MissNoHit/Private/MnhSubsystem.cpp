// Copyright 2024 Eren Balatkan. All Rights Reserved.

#include "MnhSubsystem.h"
#include "Async/ParallelFor.h"
#include "MnhTracerComponent.h"

UMnhSubsystem* UMnhSubsystem::GetMnhSubsystem(const UWorld* World)
{
	check(World)
	return World->GetSubsystem<UMnhSubsystem>();
}

FMnhTracerData* UMnhSubsystem::GetTracerDataAt(const int32 Index)
{
	if (TracerDatas.IsValidIndex(Index))
	{
		return &TracerDatas[Index];
	}
	return nullptr;
}

int32 UMnhSubsystem::RequestNewTracerData()
{
	FScopeLock ScopeLock(&CriticalSection);
	return TracerDatas.AddDefaulted();
}

void UMnhSubsystem::MarkTracerDataForRemoval(const int32 TracerDataIdx, const FGuid Guid)
{
	if (RemovalLock)
	{
		if (TracerDatas.IsValidIndex(TracerDataIdx) and TracerDatas[TracerDataIdx].Guid == Guid)
		{
			TracerDatas[TracerDataIdx].IsPendingRemoval = true;
		}
	}
	else
	{
		RemoveTracerDataAt(TracerDataIdx, Guid);
	}
}

void UMnhSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	this->TracerDatas.Reserve(4096);
}

void UMnhSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SCOPE_CYCLE_COUNTER(STAT_MnhTickTracers);

	TickIdx++;
	// Lock removals so we don't get any modifications to the array while we are iterating
	RemovalLock = true;

	UpdateTracerTransforms(DeltaTime);
	PerformTraces(DeltaTime);
	NotifyTraceResults();

	// Reverse iterate, remove pending removals
	RemovalLock = false;
	for (int TracerDataIdx = TracerDatas.Num() - 1; TracerDataIdx >= 0; TracerDataIdx--)
	{
		const auto& TracerData = TracerDatas[TracerDataIdx];
		if (!TracerDatas.IsValidIndex(TracerDataIdx))
		{
			continue;
		}

		if (TracerData.IsPendingRemoval)
		{
			RemoveTracerDataAt(TracerDataIdx, TracerData.Guid);
		}
	}
}

TStatId UMnhSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMnhSubsystem, STATGROUP_Tickables);
}

void UMnhSubsystem::RemoveTracerDataAt(const int32 TracerDataIdx, const FGuid Guid)
{
	SCOPE_CYCLE_COUNTER(STAT_MnhRemoveTracer)
	if (TracerDatas.IsValidIndex(TracerDataIdx) && TracerDatas[TracerDataIdx].Guid == Guid)
	{
		FScopeLock ScopeLock(&CriticalSection);
		TracerDatas.RemoveAtSwap(TracerDataIdx);

		// If we removed the last element in array we don't need to update the index of the tracer
		if (TracerDatas.IsValidIndex(TracerDataIdx))
		{
			const FMnhTracerData& TracerData = TracerDatas[TracerDataIdx];

			if (TracerData.OwnerTracerComponent.IsValid())
			{
				if (TracerData.OwnerTracerComponent->TracerConfigs.IsValidIndex(TracerData.OwnerTracerConfigIdx))
				{
					TracerData.OwnerTracerComponent->TracerConfigs[TracerData.OwnerTracerConfigIdx].TracerDataIdx = TracerDataIdx;
				}
				else
				{
					ensureAlwaysMsgf(false, TEXT("Tried to remove tracer with invalid Owner Index, shouldn't happen."));
				}
			}
		}
	}
}

void UMnhSubsystem::UpdateTracerTransforms(const float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_MnhTracerUpdateTransforms);
	ParallelFor(TracerDatas.Num(), [&](const int32 TracerDataIdx)
	{
		if (!TracerDatas.IsValidIndex(TracerDataIdx)){
			return;
		}
		auto& TracerData = TracerDatas[TracerDataIdx];
		if (TracerData.TracerState == EMnhTracerState::Stopped)
		{
			return;
		}

		if(!TracerData.SourceComponent.IsValid())
		{
			TracerData.TracerState = EMnhTracerState::Stopped;
			TracerData.bShouldTickThisFrame = false;
			return;
		}

		check(TracerData.bShouldTickThisFrame == false)

		const auto CurrentTransform = TracerData.GetCurrentTracerTransform();

		if (TracerData.TracerTransformsOverTime.Num() == 0)
		{
			TracerData.TracerTransformsOverTime.Add(CurrentTransform);
		}

		if (TracerData.TracerState == EMnhTracerState::PendingStop)
		{
			TracerData.bShouldTickThisFrame = true;
			TracerData.TracerTransformsOverTime.Add(CurrentTransform);
			return;
		}

		switch (TracerData.TracerTickType)
		{
		case EMnhTracerTickType::MatchGameTick:
			TracerData.TracerTransformsOverTime.Add(CurrentTransform);
			TracerData.bShouldTickThisFrame = true;
			return;
		case EMnhTracerTickType::DistanceTick:
			if ((TracerData.TracerTransformsOverTime[0].GetLocation() - CurrentTransform.GetLocation()).Length() >= TracerData.TickInterval)
			{
				TracerData.TracerTransformsOverTime.Add(CurrentTransform);
				TracerData.bShouldTickThisFrame = true;
			}
			return;
		case EMnhTracerTickType::FixedRateTick:
			if (TracerData.DeltaTimeLastTick + DeltaTime > TracerData.TickInterval / 2){
				TracerData.TracerTransformsOverTime.Add(CurrentTransform);
				TracerData.bShouldTickThisFrame = true;
			}
			return;
		}
	});
}

void UMnhSubsystem::PerformTraces(const float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_MnhTracerDoTrace)
	const UWorld* World = GetWorld();

	ParallelFor(TracerDatas.Num(), [&](const int32 TracerDataIdx)
	{
		if (!TracerDatas.IsValidIndex(TracerDataIdx)){
			return;
		}
		auto& TracerData = TracerDatas[TracerDataIdx];
		if (TracerData.bShouldTickThisFrame)
		{
			int SubSteps = 1;
			if (TracerData.TracerTickType == EMnhTracerTickType::DistanceTick)
			{
				SubSteps = FMath::CeilToInt((TracerData.TracerTransformsOverTime[0].GetLocation() - TracerData.TracerTransformsOverTime[1].GetLocation()).Length() / TracerData.TickInterval);
			}
			else if (TracerData.TracerTickType == EMnhTracerTickType::FixedRateTick)
			{
				SubSteps = FMath::CeilToInt(DeltaTime / TracerData.TickInterval);
			}
			SubSteps = FMath::Min(10, SubSteps);
			TracerData.DoTrace(World, SubSteps, TickIdx);
		}

		TracerData.DeltaTimeLastTick += DeltaTime;
	});
}

void UMnhSubsystem::NotifyTraceResults()
{
	const auto World = GetWorld();
	size_t i = 0;
	while (i < TracerDatas.Num())
	{
		FMnhTracerData& TracerData = TracerDatas[i];

		if (!TracerData.bShouldTickThisFrame)
		{
			++i;
			continue;
		}

		for (const auto& SubstepResults : TracerData.SubstepHits)
		{
			// Respect cancellations by user-defined code immediately.
			if (TracerData.TracerState == EMnhTracerState::Stopped)
			{
				break;
			}

			auto& TracerConfig = TracerData.OwnerTracerComponent->TracerConfigs[TracerData.OwnerTracerConfigIdx];
			if (TracerConfig.DrawDebugType != EDrawDebugTrace::None)
			{
				FMnhHelpers::DrawDebug(SubstepResults.StartLocation,
					SubstepResults.EndLocation,
					SubstepResults.Scale,
					SubstepResults.Rotation,
					SubstepResults.HitResults, TracerData.ShapeData, World,
					TracerConfig.DrawDebugType,
					TracerConfig.DrawDebugType == EDrawDebugTrace::ForOneFrame ? TracerData.DeltaTimeLastTick : TracerConfig.DebugDrawTime,
					TracerConfig.DebugTraceColor, TracerConfig.DebugTraceBlockColor, TracerConfig.DebugTraceHitColor);
			}

			TracerData.OwnerTracerComponent->OnTracerHitDetected(TracerConfig.TracerTag, SubstepResults.HitResults,
				TracerData.DeltaTimeLastTick / TracerData.SubstepHits.Num(), TickIdx, TracerData.SourceComponent.Get());
		}

		TracerData.DeltaTimeLastTick = 0;
		TracerData.bShouldTickThisFrame = false;

		if (TracerData.TracerState == EMnhTracerState::PendingStop)
		{
			TracerData.TracerState = EMnhTracerState::Stopped;
			TracerData.TracerTransformsOverTime.Empty();
		}
		else
		{
			TracerData.TracerTransformsOverTime.RemoveAtSwap(0);
		}

		++i;
	}
}
