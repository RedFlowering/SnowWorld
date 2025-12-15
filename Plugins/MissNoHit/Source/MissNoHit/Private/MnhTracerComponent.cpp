// Copyright 2024 Eren Balatkan. All Rights Reserved.


#include "MnhTracerComponent.h"

#include "GameplayTagContainer.h"
#include "MnhHitNotifyInterface.h"
#include "MnhSubsystem.h"
#include "MnhTracer.h"

DEFINE_LOG_CATEGORY(LogMnh)

class FMissNoHitModule;
// Sets default values for this component's properties
UMnhTracerComponent::UMnhTracerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMnhTracerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (const auto World = GetWorld())
	{
		OnDestroyed.Broadcast();
		for (const auto& TracerConfig : TracerConfigs)
		{
			TracerConfig.MarkTracerDataForRemoval(World);
		}
	}
}

void UMnhTracerComponent::InitializeTracers(const FGameplayTagContainer TracerTags, UPrimitiveComponent* TracerSource)
{
	if (TracerTags.IsEmpty())
	{
		const FString Message = FString::Printf(TEXT("MissNoHit Warning: Tracer Tags are empty on Initialize Tracers function"));
		FMnhHelpers::Mnh_Log(Message);
		return;
	}
	
	if (!bIsInitialized)
	{
		EarlyTracerInitializations.Add(FTracerInitializationData{TracerTags, TracerSource});
		return;
	}
	
	if (!IsValid(TracerSource))
	{
		const auto Owner = GetOwner();
		FString Message;
		if (!Owner)
		{
			Message = FString::Printf(TEXT("MissNoHit Warning: Tracer Source is not valid"));
		}
		else
		{
			Message = FString::Printf(TEXT("MissNoHit Warning: Tracer Source is not valid on object: %s"), *Owner->GetName());
		}
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message);
		return;
	}

	int InitializedTracerCount = 0;
	for (auto& TracerConfig : TracerConfigs)
	{
		if(TracerTags.HasTagExact(TracerConfig.TracerTag))
		{
			TracerConfig.InitializeParameters(TracerSource);
			InitializedTracerCount++;
		}
	}
	if (InitializedTracerCount == 0)
	{
		const FString Message = FString::Printf(TEXT("MissNoHit Warning: No Tracer with Tag [%s] found during initialization"), *TracerTags.ToString());
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message);
	}
}


void UMnhTracerComponent::StopAllTracers()
{
	for (auto& TracerConfig : TracerConfigs)
	{
		if (TracerConfig.TracerDataIdx == INDEX_NONE)
		{
			continue;
		}
		TracerConfig.ChangeTracerState(GetWorld(), false);
		OnTracerStopped.Broadcast(TracerConfig.TracerTag);
	}
}

void UMnhTracerComponent::ResetHitCache()
{
	HitCache.Reset();
}

void UMnhTracerComponent::ResetHitCacheByTags(FGameplayTagContainer TracerTags)
{
	if (TracerTags.IsEmpty() || HitCache.IsEmpty())
	{
		return;
	}

	HitCache.RemoveAll([&TracerTags](const FMnhHitCache& HitRecord)
	{
		return HitRecord.TracerTag.MatchesAny(TracerTags);
	});
}

void UMnhTracerComponent::AddToIgnoredActors(const FGameplayTagContainer TracerTags, AActor* Actor)
{
	if (Actor == nullptr)
	{
		FMnhHelpers::Mnh_Log("MissNoHit Warning: Actor is null, cannot add to ignored actors");
		return;
	}
	for (auto& TracerConfig: TracerConfigs)
	{
		if (!TracerConfig.TracerTag.MatchesAny(TracerTags))
		{
			continue;
		}
		
		TracerConfig.CollisionParams.AddIgnoredActor(Actor);
		if (TracerConfig.TracerDataIdx != INDEX_NONE)
		{
			const auto MnhSubsystem = UMnhSubsystem::GetMnhSubsystem(GetWorld());
			const auto TracerData = MnhSubsystem->GetTracerDataAt(TracerConfig.TracerDataIdx);
			TracerData->CollisionParams.AddIgnoredActor(Actor);
		}
	}
}

void UMnhTracerComponent::ResetIgnoredActors(const FGameplayTagContainer TracerTags)
{
	for (auto& TracerConfig: TracerConfigs)
	{
		if (!TracerConfig.TracerTag.MatchesAny(TracerTags))
		{
			continue;
		}

		TracerConfig.CollisionParams.ClearIgnoredActors();
		// TracerConfig.CollisionParams.ClearIgnoredSourceObjects(); // 5.7+

		if (TracerConfig.TracerDataIdx != INDEX_NONE)
		{
			const auto MnhSubsystem = UMnhSubsystem::GetMnhSubsystem(GetWorld());
			auto* TracerData = MnhSubsystem->GetTracerDataAt(TracerConfig.TracerDataIdx);

			TracerData->CollisionParams.ClearIgnoredActors();
			// TracerData.CollisionParams.ClearIgnoredSourceObjects();  // 5.7+
		}
	}
}

void UMnhTracerComponent::AddToIgnoredActorsArray(const FGameplayTagContainer TracerTags, TArray<AActor*> Actors)
{
	for (const auto Actor : Actors)
	{
		AddToIgnoredActors(TracerTags, Actor);
	}
}

void UMnhTracerComponent::SetTracerTickRate(const FGameplayTagContainer TracerTags, const int32 TicksPerSecond)
{
	for (auto& TracerConfig: TracerConfigs)
	{
		if (!TracerConfig.TracerTag.MatchesAny(TracerTags))
		{
			continue;
		}

		TracerConfig.TargetFps = TicksPerSecond;
		if (TracerConfig.TracerDataIdx != INDEX_NONE)
		{
			const auto MnhSubsystem = UMnhSubsystem::GetMnhSubsystem(GetWorld());
			auto* TracerData = MnhSubsystem->GetTracerDataAt(TracerConfig.TracerDataIdx);

			TracerData->TickInterval = 1.0f/static_cast<float>(TicksPerSecond);
		}
	}
}

void UMnhTracerComponent::StartTracers(const FGameplayTagContainer TracerTags, const bool bResetHitCache)
{
	StartTracersInternal(TracerTags, bResetHitCache, false);
}

int32 UMnhTracerComponent::AddNewTracer(const FGameplayTag TracerTag, const EMnhTraceSource TraceSource,
	const FMnhTraceSettings& TraceSettings, const EMnhTracerTickType TracerTickType,
	const int TargetFps, const int TargetDistanceTraveled, const EDrawDebugTrace::Type DrawDebugType, const float DrawDebugLifetime)
{
	int32 TracerConfigIdx = FindTracerConfig(TracerTag);
	if (TracerConfigIdx != INDEX_NONE)
	{
		const FMnhTracerConfig& ExistingTracerConfig = TracerConfigs[TracerConfigIdx];
		if (ExistingTracerConfig.TraceSource != TraceSource)
		{
			const FString Message = FString::Printf(TEXT("MissNoHit Warning: Tracer with Tag [%s] already exists with different Trace Source."
												"You can only have multiple tracers with same tag if they also have same trace sources"), *TracerTag.ToString());
			FMnhHelpers::Mnh_Log(Message);
			return INDEX_NONE;
		}
	}

	TracerConfigIdx = TracerConfigs.AddDefaulted();
	auto& TracerConfig = TracerConfigs[TracerConfigIdx];
	TracerConfig.OwnerTracerConfigIdx = TracerConfigIdx;
	TracerConfig.TracerTag = TracerTag;
	TracerConfig.TraceSource = TraceSource;
	TracerConfig.TraceSettings = TraceSettings;
	TracerConfig.TracerTickType = TracerTickType;
	TracerConfig.TargetFps = TargetFps;
	TracerConfig.TickDistanceTraveled = TargetDistanceTraveled;
	TracerConfig.DrawDebugType = DrawDebugType;
	TracerConfig.DebugDrawTime = DrawDebugLifetime;
	TracerConfig.OwnerTracerComponent = this;
	return TracerConfigIdx;
}

int32 UMnhTracerComponent::AddNewTracerConfig(const FMnhTracerConfig& TracerConfig)
{
	int TracerConfigIdx = FindTracerConfig(TracerConfig.TracerTag);
	if (TracerConfigIdx != INDEX_NONE)
	{
		const FMnhTracerConfig& ExistingTracerConfig = TracerConfigs[TracerConfigIdx];
		if (ExistingTracerConfig.TraceSource != TracerConfig.TraceSource and TracerConfig.TraceSource != EMnhTraceSource::MnhShapeComponent)
		{
			const FString Message = FString::Printf(TEXT("MissNoHit Warning: Tracer with Tag [%s] already exists with different Trace Source."
												"You can only have multiple tracers with same tag if they also have same trace sources"), *TracerConfig.TracerTag.ToString());
			FMnhHelpers::Mnh_Log(Message);
			return INDEX_NONE;
		}
	}

	TracerConfigIdx = TracerConfigs.Add(TracerConfig);
	auto& AddedTracerConfig = TracerConfigs[TracerConfigIdx];
	AddedTracerConfig.OwnerTracerConfigIdx = TracerConfigIdx;
	AddedTracerConfig.OwnerTracerComponent = this;
	AddedTracerConfig.RegisterTracerData(GetWorld());
	return TracerConfigIdx;
}

int32 UMnhTracerComponent::AddNewTracerConfigInitialized(const FMnhTracerConfig& TracerConfig, UPrimitiveComponent* TracerSource)
{
	if (!IsValid(TracerSource))
	{
		const auto Owner = GetOwner();
		FString Message;
		if (!Owner)
		{
			Message = FString::Printf(TEXT("MissNoHit Warning: Tracer Source is not valid"));
		}
		else
		{
			Message = FString::Printf(TEXT("MissNoHit Warning: Tracer Source is not valid on object: %s"), *Owner->GetName());
		}

		FMnhHelpers::Mnh_Log(Message);
		return INDEX_NONE;
	}

	const int32 TracerConfigIdx = AddNewTracerConfig(TracerConfig);
	TracerConfigs[TracerConfigIdx].InitializeParameters(TracerSource);
	return TracerConfigIdx;
}

void UMnhTracerComponent::AddNewMnhComponentTracer(const FGameplayTag TracerTag, const FMnhTraceSettings TraceSettings, const EMnhTracerTickType TracerTickType,
                                                   const int TargetFps, const int TargetDistanceTraveled, const EDrawDebugTrace::Type DrawDebugType, const float DrawDebugLifetime)
{
	const auto TracerIdx =
		AddNewTracer(TracerTag, EMnhTraceSource::MnhShapeComponent, TraceSettings, TracerTickType, TargetFps, TargetDistanceTraveled, DrawDebugType, DrawDebugLifetime);
	if (TracerIdx != INDEX_NONE)
	{
		TracerConfigs[TracerIdx].RegisterTracerData(GetWorld());
	}
}

void UMnhTracerComponent::AddNewPhysicsAssetTracer(const FGameplayTag TracerTag, const FName SocketOrBoneName, const FMnhTraceSettings TraceSettings,
                                                   const EMnhTracerTickType TracerTickType, const int TargetFps, const int TargetDistanceTraveled,
                                                   const EDrawDebugTrace::Type DrawDebugType, const float DrawDebugLifetime)
{
	const auto TracerIdx =
		AddNewTracer(TracerTag, EMnhTraceSource::PhysicsAsset, TraceSettings, TracerTickType, TargetFps, TargetDistanceTraveled, DrawDebugType, DrawDebugLifetime);
	if (TracerIdx != INDEX_NONE)
	{
		auto& TracerConfig = TracerConfigs[TracerIdx];
		TracerConfig.SocketOrBoneName = SocketOrBoneName;
		TracerConfig.RegisterTracerData(GetWorld());
	}
}

void UMnhTracerComponent::AddNewAnimNotifyTracer(const FGameplayTag TracerTag, const FMnhTraceSettings TraceSettings, const EMnhTracerTickType TracerTickType,
                                                 const int TargetFps, const int TargetDistanceTraveled, const EDrawDebugTrace::Type DrawDebugType, const float DrawDebugLifetime)
{
	const auto TracerIdx =
		AddNewTracer(TracerTag, EMnhTraceSource::AnimNotify, TraceSettings, TracerTickType, TargetFps, TargetDistanceTraveled, DrawDebugType, DrawDebugLifetime);
	if (TracerIdx != INDEX_NONE)
	{
		TracerConfigs[TracerIdx].RegisterTracerData(GetWorld());
	}
}

void UMnhTracerComponent::AddNewStaticMeshSocketsTracer(const FGameplayTag TracerTag, const FName MeshSocket1, const FName MeshSocket2, const float MeshSocketTracerRadius,
                                                        const float MeshSocketTracerLengthOffset, const FMnhTraceSettings TraceSettings,
                                                        const EMnhTracerTickType TracerTickType, const int TargetFps, const int TargetDistanceTraveled,
                                                        const EDrawDebugTrace::Type DrawDebugType, const float DrawDebugLifetime)
{
	const auto TracerIdx =
		AddNewTracer(TracerTag, EMnhTraceSource::StaticMeshSockets, TraceSettings, TracerTickType, TargetFps, TargetDistanceTraveled, DrawDebugType, DrawDebugLifetime);
	if (TracerIdx != INDEX_NONE)
	{
		auto& TracerConfig = TracerConfigs[TracerIdx];
		TracerConfig.MeshSocket_1 = MeshSocket1;
		TracerConfig.MeshSocket_2 = MeshSocket2;
		TracerConfig.MeshSocketTracerRadius = MeshSocketTracerRadius;
		TracerConfig.MeshSocketTracerLengthOffset = MeshSocketTracerLengthOffset;
		TracerConfig.RegisterTracerData(GetWorld());
	}
}

void UMnhTracerComponent::AddNewSkeletalMeshSocketsTracer(const FGameplayTag TracerTag, const FName MeshSocket1, const FName MeshSocket2, const float MeshSocketTracerRadius,
                                                          const float MeshSocketTracerLengthOffset, const FMnhTraceSettings TraceSettings,
                                                          const EMnhTracerTickType TracerTickType, const int TargetFps, const int TargetDistanceTraveled,
                                                          const EDrawDebugTrace::Type DrawDebugType, const float DrawDebugLifetime)
{
	const auto TracerIdx =
		AddNewTracer(TracerTag, EMnhTraceSource::SkeletalMeshSockets, TraceSettings, TracerTickType, TargetFps, TargetDistanceTraveled, DrawDebugType, DrawDebugLifetime);
	if (TracerIdx != INDEX_NONE)
	{
		auto& TracerConfig = TracerConfigs[TracerIdx];
		TracerConfig.MeshSocket_1 = MeshSocket1;
		TracerConfig.MeshSocket_2 = MeshSocket2;
		TracerConfig.MeshSocketTracerRadius = MeshSocketTracerRadius;
		TracerConfig.MeshSocketTracerLengthOffset = MeshSocketTracerLengthOffset;
		TracerConfig.RegisterTracerData(GetWorld());
	}
}

void UMnhTracerComponent::RemoveTracerConfigsByPredicate(const TFunctionRef<bool(const FMnhTracerConfig& ConfigToRemove)> Predicate)
{
	if (TracerConfigs.IsEmpty())
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TracerConfigs.RemoveAll(Predicate);

	const auto MnhSubsystem = UMnhSubsystem::GetMnhSubsystem(GetWorld());
	for (int32 i = 0; i < TracerConfigs.Num(); ++i)
	{
		TracerConfigs[i].OwnerTracerConfigIdx = i;

		if (TracerConfigs[i].TracerDataIdx != INDEX_NONE)
		{
			auto* TracerData = MnhSubsystem->GetTracerDataAt(TracerConfigs[i].TracerDataIdx);
			TracerData->OwnerTracerConfigIdx = i;
		}
	}
}

void UMnhTracerComponent::RemoveTracerConfigsByTags(const FGameplayTagContainer TracerTags)
{
	if (TracerTags.IsEmpty())
	{
		return;
	}

	const UWorld* World = GetWorld();
	RemoveTracerConfigsByPredicate([&](const FMnhTracerConfig& ConfigToRemove)
	{
		if (ConfigToRemove.TracerTag.MatchesAny(TracerTags))
		{
			ConfigToRemove.MarkTracerDataForRemoval(World);
			return true;
		}
		return false;
	});
}

void UMnhTracerComponent::RemoveTracerConfigsByComponent(const UPrimitiveComponent* TracerSource)
{
	if (!IsValid(TracerSource))
	{
		return;
	}

	const UWorld* World = GetWorld();
	RemoveTracerConfigsByPredicate([&](const FMnhTracerConfig& ConfigToRemove)
	{
		if (ConfigToRemove.SourceComponent == TracerSource)
		{
			ConfigToRemove.MarkTracerDataForRemoval(World);
			return true;
		}
		return false;
	});
}

void UMnhTracerComponent::StartTracersInternal(const FGameplayTagContainer TracerTags, const bool bResetHitCache, const bool AllowAnimNotify=false)
{
	SCOPE_CYCLE_COUNTER(STAT_MNHStartTracer);

	if (bResetHitCache)
	{
	    for (int i = HitCache.Num() - 1; i >= 0; --i)
	    {
	        const auto &HitRecord = HitCache[i];
	        for (const auto TracerTag : TracerTags)
	        {
	            if (HitRecord.TracerTag.MatchesTag(TracerTag))
	            {
	                HitCache.RemoveAt(i);
	                break;
	            }
	        }
	    }
	}

	for (auto& TracerConfig: TracerConfigs)
	{
		if (TracerConfig.SourceComponent == nullptr && TracerConfig.TraceSource != EMnhTraceSource::AnimNotify)
		{
			FMnhHelpers::Mnh_Log("MissNoHit Warning: Tracer Source is not initialized for Tracer Config: "
						"" + TracerConfig.TracerTag.ToString() + " On Actor: " + GetOwner()->GetName());
			continue;
		}
		
		if (TracerConfig.TracerTag.MatchesAny(TracerTags))
		{
			if (!AllowAnimNotify)
			{
				if (TracerConfig.TraceSource == EMnhTraceSource::AnimNotify)
				{
					const FString DebugMessage = FString::Printf(TEXT("MissNoHit Warning: "
															   "AnimNotifyTracer [%s] on Owner [%s] cannot be started manually"),
															   *TracerConfig.TracerTag.ToString(), *GetOwner()->GetName());
					FMnhHelpers::Mnh_Log(DebugMessage);
					continue;
				}
			}

			if (TracerConfig.TracerDataIdx == INDEX_NONE)
			{
				continue;
			}
			
			TracerConfig.ChangeTracerState(GetWorld(), true);
			OnTracerStarted.Broadcast(TracerConfig.TracerTag);
		}
	}
}

void UMnhTracerComponent::StopTracers(const FGameplayTagContainer TracerTags)
{
	for (auto& TracerConfig : TracerConfigs)
	{
		if (TracerConfig.TracerTag.MatchesAny(TracerTags) && TracerConfig.TracerDataIdx != INDEX_NONE)
		{
			TracerConfig.ChangeTracerState(GetWorld(), false);
			OnTracerStopped.Broadcast(TracerConfig.TracerTag);
		}
	}
}

void UMnhTracerComponent::StopTracersDelayed(const FGameplayTagContainer TracerTags)
{
	for (auto& TracerConfig : TracerConfigs)
	{
		if (TracerConfig.TracerTag.MatchesAny(TracerTags) && TracerConfig.TracerDataIdx != INDEX_NONE)
		{
			TracerConfig.ChangeTracerState(GetWorld(), false, false);
			OnTracerStopped.Broadcast(TracerConfig.TracerTag);
		}
	}
}

void UMnhTracerComponent::StartTracersByComponent(const UPrimitiveComponent* Component)
{
	if (!IsValid(Component))
	{
		const FString Message = FString::Printf(TEXT("MissNoHit Warning: Attempted to start tracers with a null or invalid component on Actor: %s"),
			*GetOwner()->GetName());
		FMnhHelpers::Mnh_Log(Message);
		return;
	}

	for (auto& TracerConfig : TracerConfigs)
	{
		if (TracerConfig.SourceComponent == Component && TracerConfig.TracerDataIdx != INDEX_NONE)
		{
			TracerConfig.ChangeTracerState(GetWorld(), true);
			OnTracerStarted.Broadcast(TracerConfig.TracerTag);
		}
	}
}

void UMnhTracerComponent::StopTracersByComponent(const UPrimitiveComponent* Component)
{
	if (!IsValid(Component))
	{
		const FString Message = FString::Printf(TEXT("MissNoHit Warning: Attempted to stop tracers with a null or invalid component on Actor: %s"),
			*GetOwner()->GetName());
		FMnhHelpers::Mnh_Log(Message);
		return;
	}

	for (auto& TracerConfig : TracerConfigs)
	{
		if (TracerConfig.SourceComponent == Component && TracerConfig.TracerDataIdx != INDEX_NONE)
		{
			TracerConfig.ChangeTracerState(GetWorld(), false);
			OnTracerStopped.Broadcast(TracerConfig.TracerTag);
		}
	}
}

void UMnhTracerComponent::UpdateDebugDrawState(const FGameplayTagContainer TracerTags, const TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType)
{
	for (auto& TracerConfig : TracerConfigs)
	{
		if (TracerConfig.TracerTag.MatchesAny(TracerTags))
		{
			TracerConfig.DrawDebugType = DrawDebugType;
		}
	}
}

int32 UMnhTracerComponent::FindTracerConfig(const FGameplayTag TracerTag)
{
	for (int32 Idx = 0; Idx < TracerConfigs.Num(); Idx++)
	{
		const FMnhTracerConfig& TracerConfig = TracerConfigs[Idx];
		if (TracerConfig.TracerTag.MatchesTagExact(TracerTag))
		{
			return Idx;
		}
	}
	return INDEX_NONE;
}

// Called when the game starts
void UMnhTracerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	for (int TracerConfigIdx = 0; TracerConfigIdx < TracerConfigs.Num(); TracerConfigIdx++)
	{
		auto& TracerConfig = TracerConfigs[TracerConfigIdx];
		TracerConfig.OwnerTracerConfigIdx = TracerConfigIdx;
		TracerConfig.OwnerTracerComponent = this;
		TracerConfig.RegisterTracerData(GetWorld());
	}
	
	bIsInitialized = true;
	
	for (const auto& [TracerTags, TracerSource] : EarlyTracerInitializations)
	{
		InitializeTracers(TracerTags, TracerSource);
	}
}

bool UMnhTracerComponent::CheckFilters(const FHitResult& HitResult, const FGameplayTag TracerTag, int TickIdxArg)
{
	SCOPE_CYCLE_COUNTER(STAT_MNHCheckFilters);

	if (HitCache.Num() > 2048)
	{
		HitCache.Reset();
		const FString Message = FString::Printf(TEXT("MissNoHit Warning: Your Tracer Hit Cache"
													 " contains over 2048 elements, it is very likely that you are not resetting"
													 "your Hit Cache, this can lead to performance and memory leak problems!"
													 " Please enable MissNoHit tracers only when they are needed"));
		FMnhHelpers::Mnh_Log(Message);
	}

	for (auto& HitRecord : HitCache)
    {
        if (FilterType == EMnhFilterType::FilterSameActorPerTracer)
        {
        	if (HitRecord.TracerTag.MatchesTag(TracerTag) && HitRecord.HitResult.GetActor() == HitResult.GetActor())
        	{
        		return false;
        	}
        }
        else if (FilterType == EMnhFilterType::FilterSameActorAcrossAllTracers)
        {
        	if (HitRecord.HitResult.GetActor() == HitResult.GetActor())
        	{
        		return false;
        	}
        }
    }
	
	return true;
}

void UMnhTracerComponent::OnTracerHitDetected(const FGameplayTag TracerTag, const TArray<FHitResult>& HitResults, const float DeltaTime, const int TickIdx, const UPrimitiveComponent* Component)
{
	SCOPE_CYCLE_COUNTER(STAT_MnhTracerComponentHitDetected)
	FScopeLock ScopedLock(&TraceDoneScopeLock);

	auto NotifyHitDetection = [=, this](const FHitResult& HitResult)
	{
		OnHitDetected.Broadcast(TracerTag, HitResult, DeltaTime, Component);
		const auto HitActor = HitResult.GetActor();
		if (HitActor and HitActor->Implements<UMnhHitNotifyInterface>())
		{
			IMnhHitNotifyInterface::Execute_OnHitReceived(
					HitActor,
					TracerTag,
					HitResult,
					DeltaTime,
					Component
				);
		}
	};

	for (auto& HitResult : HitResults)
	{
		if (FilterType == EMnhFilterType::None)
		{
			NotifyHitDetection(HitResult);
		}
		else if(CheckFilters(HitResult, TracerTag, TickIdx))
		{
			HitCache.Add(FMnhHitCache{HitResult, TracerTag, TickIdx});
			NotifyHitDetection(HitResult);
		}
	}
}

