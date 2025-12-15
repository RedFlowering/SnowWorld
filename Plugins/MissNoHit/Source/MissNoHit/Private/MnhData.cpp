// Copyright 2024 Eren Balatkan. All Rights Reserved.

#include "MnhData.h"

#include "MnhTracerComponent.h"
#include "Kismet/KismetMathLibrary.h"

void FMnhTracerData::ChangeTracerState(const bool bIsTracerActiveArg, const bool bStopImmediate)
{
	if (bIsTracerActiveArg)
	{
		this->TracerState = EMnhTracerState::Active;
		this->DeltaTimeLastTick = 0;
		if (SourceComponent.IsValid())
		{
			UpdatePreviousTransform(GetCurrentTracerTransform());
		}
	}
	else
	{
		if (this->TracerState == EMnhTracerState::Active && !bStopImmediate)
		{
			this->TracerState = EMnhTracerState::PendingStop;
		}
		else
		{
			this->TracerState = EMnhTracerState::Stopped;
		}
	}
}

void FMnhTracerData::DoTrace(const UWorld* World, const uint32 Substeps, const uint32 TickIdx)
{
	SubstepHits.Reset();
	if (TracerTransformsOverTime.Num() > 1)
	{
		const float SubstepRatio = 1.0 / Substeps;
		const FTransform CurrentTransform = TracerTransformsOverTime.Last();
		const FTransform PreviousTransform = TracerTransformsOverTime[0];

		for (uint32 i = 0; i<Substeps; i++)
		{
			// Respect cancellations by user-defined code immediately.
			if (TracerState == EMnhTracerState::Stopped)
			{
				break;
			}

			const FTransform& StartTransform = UKismetMathLibrary::TLerp(PreviousTransform, CurrentTransform, SubstepRatio * i, ELerpInterpolationMode::DualQuatInterp);
			const FTransform& EndTransform = UKismetMathLibrary::TLerp(PreviousTransform, CurrentTransform, SubstepRatio * (i+1), ELerpInterpolationMode::DualQuatInterp);
			const auto& AverageTransform = UKismetMathLibrary::TLerp(StartTransform, EndTransform, 0.5, ELerpInterpolationMode::DualQuatInterp);

			TArray<FHitResult> OutHits;
			FMnhHelpers::PerformTrace(StartTransform, EndTransform, AverageTransform,
				OutHits, World, TraceSettings, ShapeData, CollisionParams, FCollisionResponseParams(), ObjectQueryParams);

			SubstepHits.Add(
				{
					StartTransform.GetLocation(),
					EndTransform.GetLocation(),
					AverageTransform.GetScale3D(),
					AverageTransform.GetRotation(),
					OutHits
				});
		}
	}
	else
	{
		const FString& DebugMessage = FString::Printf(TEXT("MissNoHit Warning: Tracer [%s] has less than 2 transforms in TracerTransformsOverTime array!"),
			*OwnerTracerComponent->TracerConfigs[OwnerTracerConfigIdx].TracerTag.ToString());
		FMnhHelpers::Mnh_Log(DebugMessage);
	}
}