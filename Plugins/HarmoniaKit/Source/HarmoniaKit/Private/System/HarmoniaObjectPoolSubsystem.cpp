// Copyright 2025 Snow Game Studio.

#include "System/HarmoniaObjectPoolSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "HarmoniaLogCategories.h"

// ============================================================================
// Subsystem Lifecycle
// ============================================================================

void UHarmoniaObjectPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogHarmoniaKit, Log, TEXT("HarmoniaObjectPoolSubsystem: Initializing..."));

	// Register default pools from config
	for (const FHarmoniaPoolConfig& Config : DefaultPoolConfigs)
	{
		if (Config.ActorClass)
		{
			RegisterPool(Config);
		}
	}

	// Start maintenance timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			MaintenanceTimerHandle,
			FTimerDelegate::CreateUObject(this, &UHarmoniaObjectPoolSubsystem::TickPoolMaintenance, MaintenanceInterval),
			MaintenanceInterval,
			true  // Looping
		);
	}

	UE_LOG(LogHarmoniaKit, Log, TEXT("HarmoniaObjectPoolSubsystem: Initialized with %d default pools"), DefaultPoolConfigs.Num());
}

void UHarmoniaObjectPoolSubsystem::Deinitialize()
{
	UE_LOG(LogHarmoniaKit, Log, TEXT("HarmoniaObjectPoolSubsystem: Deinitializing..."));

	// Clear maintenance timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MaintenanceTimerHandle);
		
		// Clear delayed release timers
		for (auto& Pair : DelayedReleaseHandles)
		{
			World->GetTimerManager().ClearTimer(Pair.Value);
		}
	}
	DelayedReleaseHandles.Empty();

	// Destroy all pooled actors
	ClearAllPools();

	Super::Deinitialize();
}

bool UHarmoniaObjectPoolSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Only create for game worlds
	if (UWorld* World = Cast<UWorld>(Outer))
	{
		return World->IsGameWorld();
	}
	return false;
}

// ============================================================================
// Pool Registration
// ============================================================================

bool UHarmoniaObjectPoolSubsystem::RegisterPool(const FHarmoniaPoolConfig& Config)
{
	if (!Config.ActorClass)
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("ObjectPool: Cannot register pool with null ActorClass"));
		return false;
	}

	if (Pools.Contains(Config.ActorClass))
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("ObjectPool: Pool already exists for class %s"), *Config.ActorClass->GetName());
		return false;
	}

	FHarmoniaActorPool NewPool;
	NewPool.Config = Config;
	NewPool.LastAcquireTime = FPlatformTime::Seconds();
	NewPool.LastShrinkCheckTime = FPlatformTime::Seconds();

	Pools.Add(Config.ActorClass, NewPool);

	// Register tag mapping if provided
	if (Config.PoolTag.IsValid())
	{
		TagToClassMap.Add(Config.PoolTag, Config.ActorClass);
	}

	// Pre-warm the pool
	WarmPool(Config.ActorClass);

	UE_LOG(LogHarmoniaKit, Log, TEXT("ObjectPool: Registered pool for %s (Initial: %d, Max: %d)"),
		*Config.ActorClass->GetName(), Config.InitialSize, Config.MaxSize);

	return true;
}

bool UHarmoniaObjectPoolSubsystem::RegisterPoolSimple(TSubclassOf<AActor> ActorClass, int32 InitialSize, int32 MaxSize)
{
	FHarmoniaPoolConfig Config;
	Config.ActorClass = ActorClass;
	Config.InitialSize = InitialSize;
	Config.MaxSize = MaxSize;
	return RegisterPool(Config);
}

void UHarmoniaObjectPoolSubsystem::UnregisterPool(TSubclassOf<AActor> ActorClass, bool bDestroyActors)
{
	if (FHarmoniaActorPool* Pool = FindPool(ActorClass))
	{
		if (bDestroyActors)
		{
			// Destroy all actors in pool
			for (TWeakObjectPtr<AActor>& ActorPtr : Pool->InactiveActors)
			{
				if (AActor* Actor = ActorPtr.Get())
				{
					Actor->Destroy();
				}
			}
			for (TWeakObjectPtr<AActor>& ActorPtr : Pool->ActiveActors)
			{
				if (AActor* Actor = ActorPtr.Get())
				{
					Actor->Destroy();
				}
			}
		}

		// Remove tag mapping
		if (Pool->Config.PoolTag.IsValid())
		{
			TagToClassMap.Remove(Pool->Config.PoolTag);
		}

		Pools.Remove(ActorClass);
		UE_LOG(LogHarmoniaKit, Log, TEXT("ObjectPool: Unregistered pool for %s"), *ActorClass->GetName());
	}
}

bool UHarmoniaObjectPoolSubsystem::HasPool(TSubclassOf<AActor> ActorClass) const
{
	return Pools.Contains(ActorClass);
}

// ============================================================================
// Actor Acquisition & Release
// ============================================================================

AActor* UHarmoniaObjectPoolSubsystem::AcquireActor(TSubclassOf<AActor> ActorClass, const FTransform& Transform, bool bAutoActivate)
{
	FHarmoniaActorPool* Pool = FindPool(ActorClass);
	if (!Pool)
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("ObjectPool: No pool registered for class %s"), *ActorClass->GetName());
		return nullptr;
	}

	AActor* AcquiredActor = nullptr;
	Pool->LastAcquireTime = FPlatformTime::Seconds();

	// Try to get from inactive pool
	while (Pool->InactiveActors.Num() > 0 && !AcquiredActor)
	{
		TWeakObjectPtr<AActor> ActorPtr = Pool->InactiveActors.Pop();
		if (AActor* Actor = ActorPtr.Get())
		{
			AcquiredActor = Actor;
		}
	}

	// If no inactive actor available, try to create new one
	if (!AcquiredActor)
	{
		Pool->Stats.PoolMisses++;

		if (Pool->Config.bCanGrow)
		{
			// Check max size
			if (Pool->Config.MaxSize == 0 || Pool->Stats.TotalCount < Pool->Config.MaxSize)
			{
				AcquiredActor = CreatePooledActor(Pool->Config);
				if (AcquiredActor)
				{
					Pool->Stats.TotalCount++;
					Pool->Stats.TotalCreated++;
				}
			}
		}
	}

	if (!AcquiredActor)
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("ObjectPool: Failed to acquire actor of class %s (pool exhausted)"), *ActorClass->GetName());
		return nullptr;
	}

	// Move to active list
	Pool->ActiveActors.Add(AcquiredActor);
	Pool->Stats.ActiveCount = Pool->ActiveActors.Num();
	Pool->Stats.InactiveCount = Pool->InactiveActors.Num();
	Pool->Stats.TotalAcquired++;

	// Activate the actor
	if (bAutoActivate)
	{
		ActivateActor(AcquiredActor, Transform);
	}
	else
	{
		AcquiredActor->SetActorTransform(Transform);
	}

	// Call interface callback
	if (AcquiredActor->Implements<UHarmoniaPoolableInterface>())
	{
		IHarmoniaPoolableInterface::Execute_OnAcquiredFromPool(AcquiredActor);
	}

	return AcquiredActor;
}

AActor* UHarmoniaObjectPoolSubsystem::AcquireActorByTag(FGameplayTag PoolTag, const FTransform& Transform, bool bAutoActivate)
{
	if (TSubclassOf<AActor>* ClassPtr = TagToClassMap.Find(PoolTag))
	{
		return AcquireActor(*ClassPtr, Transform, bAutoActivate);
	}

	UE_LOG(LogHarmoniaKit, Warning, TEXT("ObjectPool: No pool found for tag %s"), *PoolTag.ToString());
	return nullptr;
}

bool UHarmoniaObjectPoolSubsystem::ReleaseActor(AActor* Actor)
{
	if (!Actor)
	{
		return false;
	}

	// Cancel any delayed release
	if (FTimerHandle* Handle = DelayedReleaseHandles.Find(Actor))
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(*Handle);
		}
		DelayedReleaseHandles.Remove(Actor);
	}

	// Find the pool for this actor
	TSubclassOf<AActor> ActorClass = Actor->GetClass();
	FHarmoniaActorPool* Pool = FindPool(ActorClass);
	
	if (!Pool)
	{
		// Try to find pool for parent class
		for (auto& Pair : Pools)
		{
			if (Actor->IsA(Pair.Key))
			{
				Pool = &Pair.Value;
				break;
			}
		}
	}

	if (!Pool)
	{
		UE_LOG(LogHarmoniaKit, Verbose, TEXT("ObjectPool: No pool for actor %s, destroying"), *Actor->GetName());
		Actor->Destroy();
		return false;
	}

	// Check if actor can return to pool
	bool bCanReturn = true;
	if (Actor->Implements<UHarmoniaPoolableInterface>())
	{
		bCanReturn = IHarmoniaPoolableInterface::Execute_CanReturnToPool(Actor);
	}

	if (!bCanReturn)
	{
		Actor->Destroy();
		Pool->Stats.TotalCount--;
		return false;
	}

	// Remove from active list
	Pool->ActiveActors.RemoveSingle(Actor);

	// Reset and deactivate
	if (Actor->Implements<UHarmoniaPoolableInterface>())
	{
		IHarmoniaPoolableInterface::Execute_ResetPooledState(Actor);
		IHarmoniaPoolableInterface::Execute_OnReleasedToPool(Actor);
	}
	DeactivateActor(Actor);

	// Add to inactive list
	Pool->InactiveActors.Add(Actor);
	Pool->Stats.ActiveCount = Pool->ActiveActors.Num();
	Pool->Stats.InactiveCount = Pool->InactiveActors.Num();
	Pool->Stats.TotalReleased++;

	return true;
}

void UHarmoniaObjectPoolSubsystem::ReleaseActorDelayed(AActor* Actor, float Delay)
{
	if (!Actor || Delay <= 0.0f)
	{
		ReleaseActor(Actor);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		ReleaseActor(Actor);
		return;
	}

	// Cancel existing delayed release if any
	if (FTimerHandle* ExistingHandle = DelayedReleaseHandles.Find(Actor))
	{
		World->GetTimerManager().ClearTimer(*ExistingHandle);
	}

	FTimerHandle NewHandle;
	World->GetTimerManager().SetTimer(
		NewHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, WeakActor = TWeakObjectPtr<AActor>(Actor)]()
		{
			if (AActor* ActorToRelease = WeakActor.Get())
			{
				DelayedReleaseHandles.Remove(ActorToRelease);
				ReleaseActor(ActorToRelease);
			}
		}),
		Delay,
		false
	);

	DelayedReleaseHandles.Add(Actor, NewHandle);
}

void UHarmoniaObjectPoolSubsystem::ReleaseAllActorsOfClass(TSubclassOf<AActor> ActorClass)
{
	if (FHarmoniaActorPool* Pool = FindPool(ActorClass))
	{
		TArray<TWeakObjectPtr<AActor>> ActorsToRelease = Pool->ActiveActors;
		for (TWeakObjectPtr<AActor>& ActorPtr : ActorsToRelease)
		{
			if (AActor* Actor = ActorPtr.Get())
			{
				ReleaseActor(Actor);
			}
		}
	}
}

// ============================================================================
// Pool Management
// ============================================================================

void UHarmoniaObjectPoolSubsystem::WarmPool(TSubclassOf<AActor> ActorClass)
{
	FHarmoniaActorPool* Pool = FindPool(ActorClass);
	if (!Pool)
	{
		return;
	}

	int32 CurrentCount = Pool->InactiveActors.Num() + Pool->ActiveActors.Num();
	int32 ToSpawn = Pool->Config.InitialSize - CurrentCount;

	for (int32 i = 0; i < ToSpawn; ++i)
	{
		if (AActor* NewActor = CreatePooledActor(Pool->Config))
		{
			DeactivateActor(NewActor);
			Pool->InactiveActors.Add(NewActor);
			Pool->Stats.TotalCount++;
			Pool->Stats.TotalCreated++;
		}
	}

	Pool->Stats.InactiveCount = Pool->InactiveActors.Num();
	UE_LOG(LogHarmoniaKit, Log, TEXT("ObjectPool: Warmed pool %s with %d actors"), *ActorClass->GetName(), ToSpawn);
}

void UHarmoniaObjectPoolSubsystem::WarmAllPools()
{
	for (auto& Pair : Pools)
	{
		WarmPool(Pair.Key);
	}
}

void UHarmoniaObjectPoolSubsystem::ShrinkPool(TSubclassOf<AActor> ActorClass)
{
	FHarmoniaActorPool* Pool = FindPool(ActorClass);
	if (!Pool)
	{
		return;
	}

	int32 TargetSize = Pool->Config.InitialSize;
	int32 CurrentInactive = Pool->InactiveActors.Num();
	int32 ToRemove = CurrentInactive - TargetSize;

	for (int32 i = 0; i < ToRemove; ++i)
	{
		if (Pool->InactiveActors.Num() > 0)
		{
			TWeakObjectPtr<AActor> ActorPtr = Pool->InactiveActors.Pop();
			if (AActor* Actor = ActorPtr.Get())
			{
				Actor->Destroy();
				Pool->Stats.TotalCount--;
			}
		}
	}

	Pool->Stats.InactiveCount = Pool->InactiveActors.Num();
	UE_LOG(LogHarmoniaKit, Log, TEXT("ObjectPool: Shrunk pool %s, removed %d actors"), *ActorClass->GetName(), ToRemove);
}

void UHarmoniaObjectPoolSubsystem::ClearAllPools()
{
	for (auto& Pair : Pools)
	{
		FHarmoniaActorPool& Pool = Pair.Value;
		
		for (TWeakObjectPtr<AActor>& ActorPtr : Pool.InactiveActors)
		{
			if (AActor* Actor = ActorPtr.Get())
			{
				Actor->Destroy();
			}
		}
		for (TWeakObjectPtr<AActor>& ActorPtr : Pool.ActiveActors)
		{
			if (AActor* Actor = ActorPtr.Get())
			{
				Actor->Destroy();
			}
		}
		
		Pool.InactiveActors.Empty();
		Pool.ActiveActors.Empty();
		Pool.Stats = FHarmoniaPoolStats();
	}

	Pools.Empty();
	TagToClassMap.Empty();

	UE_LOG(LogHarmoniaKit, Log, TEXT("ObjectPool: Cleared all pools"));
}

// ============================================================================
// Statistics & Debugging
// ============================================================================

bool UHarmoniaObjectPoolSubsystem::GetPoolStats(TSubclassOf<AActor> ActorClass, FHarmoniaPoolStats& OutStats) const
{
	if (const FHarmoniaActorPool* Pool = FindPool(ActorClass))
	{
		OutStats = Pool->Stats;
		return true;
	}
	return false;
}

TArray<TSubclassOf<AActor>> UHarmoniaObjectPoolSubsystem::GetAllPooledClasses() const
{
	TArray<TSubclassOf<AActor>> Result;
	Pools.GetKeys(Result);
	return Result;
}

int32 UHarmoniaObjectPoolSubsystem::GetTotalPooledActorCount() const
{
	int32 Total = 0;
	for (const auto& Pair : Pools)
	{
		Total += Pair.Value.Stats.TotalCount;
	}
	return Total;
}

void UHarmoniaObjectPoolSubsystem::DebugDrawPoolStats(bool bEnabled)
{
	bDebugDrawEnabled = bEnabled;
	
	if (bEnabled)
	{
		UE_LOG(LogHarmoniaKit, Log, TEXT("=== Object Pool Statistics ==="));
		for (const auto& Pair : Pools)
		{
			const FHarmoniaPoolStats& Stats = Pair.Value.Stats;
			UE_LOG(LogHarmoniaKit, Log, TEXT("%s: Total=%d, Active=%d, Inactive=%d, Misses=%d"),
				*Pair.Key->GetName(), Stats.TotalCount, Stats.ActiveCount, Stats.InactiveCount, Stats.PoolMisses);
		}
	}
}

// ============================================================================
// Internal Functions
// ============================================================================

void UHarmoniaObjectPoolSubsystem::TickPoolMaintenance(float DeltaTime)
{
	CleanupNullReferences();
	ProcessPoolShrinking(DeltaTime);
}

AActor* UHarmoniaObjectPoolSubsystem::CreatePooledActor(const FHarmoniaPoolConfig& Config)
{
	UWorld* World = GetWorld();
	if (!World || !Config.ActorClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = World->SpawnActor<AActor>(Config.ActorClass, FTransform::Identity, SpawnParams);
	
	if (NewActor)
	{
		// Start deactivated
		DeactivateActor(NewActor);
	}

	return NewActor;
}

void UHarmoniaObjectPoolSubsystem::DeactivateActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);

	// Disable all primitive components
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* Comp : PrimitiveComponents)
	{
		Comp->SetVisibility(false);
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Move to a safe location
	Actor->SetActorLocation(FVector(0, 0, -10000));
}

void UHarmoniaObjectPoolSubsystem::ActivateActor(AActor* Actor, const FTransform& Transform)
{
	if (!Actor)
	{
		return;
	}

	Actor->SetActorTransform(Transform);
	Actor->SetActorHiddenInGame(false);
	Actor->SetActorEnableCollision(true);
	Actor->SetActorTickEnabled(true);

	// Enable all primitive components
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* Comp : PrimitiveComponents)
	{
		Comp->SetVisibility(true);
		// Note: Collision should be set by the actor's default state
	}
}

FHarmoniaActorPool* UHarmoniaObjectPoolSubsystem::FindPool(TSubclassOf<AActor> ActorClass)
{
	return Pools.Find(ActorClass);
}

const FHarmoniaActorPool* UHarmoniaObjectPoolSubsystem::FindPool(TSubclassOf<AActor> ActorClass) const
{
	return Pools.Find(ActorClass);
}

FHarmoniaActorPool* UHarmoniaObjectPoolSubsystem::FindPoolByTag(FGameplayTag Tag)
{
	if (TSubclassOf<AActor>* ClassPtr = TagToClassMap.Find(Tag))
	{
		return FindPool(*ClassPtr);
	}
	return nullptr;
}

void UHarmoniaObjectPoolSubsystem::ProcessPoolShrinking(float DeltaTime)
{
	double CurrentTime = FPlatformTime::Seconds();

	for (auto& Pair : Pools)
	{
		FHarmoniaActorPool& Pool = Pair.Value;
		
		// Skip if shrink timeout is disabled
		if (Pool.Config.ShrinkTimeout <= 0.0f)
		{
			continue;
		}

		// Check if pool should shrink
		float TimeSinceLastAcquire = CurrentTime - Pool.LastAcquireTime;
		float TimeSinceLastShrink = CurrentTime - Pool.LastShrinkCheckTime;

		if (TimeSinceLastAcquire > Pool.Config.ShrinkTimeout && TimeSinceLastShrink > Pool.Config.ShrinkTimeout)
		{
			int32 CurrentInactive = Pool.InactiveActors.Num();
			if (CurrentInactive > Pool.Config.InitialSize)
			{
				// Shrink gradually
				int32 ToRemove = FMath::Min(Pool.Config.GrowthAmount, CurrentInactive - Pool.Config.InitialSize);
				for (int32 i = 0; i < ToRemove; ++i)
				{
					if (Pool.InactiveActors.Num() > Pool.Config.InitialSize)
					{
						TWeakObjectPtr<AActor> ActorPtr = Pool.InactiveActors.Pop();
						if (AActor* Actor = ActorPtr.Get())
						{
							Actor->Destroy();
							Pool.Stats.TotalCount--;
						}
					}
				}
				Pool.Stats.InactiveCount = Pool.InactiveActors.Num();
			}
			Pool.LastShrinkCheckTime = CurrentTime;
		}
	}
}

void UHarmoniaObjectPoolSubsystem::CleanupNullReferences()
{
	for (auto& Pair : Pools)
	{
		FHarmoniaActorPool& Pool = Pair.Value;

		// Clean inactive actors
		Pool.InactiveActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr) {
			return !ActorPtr.IsValid();
		});

		// Clean active actors
		Pool.ActiveActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr) {
			return !ActorPtr.IsValid();
		});

		// Update stats
		Pool.Stats.TotalCount = Pool.InactiveActors.Num() + Pool.ActiveActors.Num();
		Pool.Stats.ActiveCount = Pool.ActiveActors.Num();
		Pool.Stats.InactiveCount = Pool.InactiveActors.Num();
	}
}
