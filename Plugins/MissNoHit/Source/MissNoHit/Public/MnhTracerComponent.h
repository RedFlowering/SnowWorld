// Copyright 2024 Eren Balatkan. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MnhHelpers.h"
#include "MnhTracer.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "MnhTracerComponent.generated.h"

struct FGameplayTagContainer;
class UMnhTracer;
struct FGameplayTag;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FMnhOnHitDetected, FGameplayTag, TracerTag, const FHitResult&, HitResult, float, DeltaTime, const UPrimitiveComponent*, TracerSource);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMnhOnTraceFinished, FGameplayTag, TracerTag, const TArray<FHitResult>&, HitResult, const float, DeltaTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMnhOnTracerStopped, FGameplayTag, TracerTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMnhOnTracerStarted, FGameplayTag, TracerTag);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMnhTracerComponentDestroyed);

struct FMnhHitCache
{
	FHitResult HitResult;
	FGameplayTag TracerTag;
	int TickIdx;
};

struct FTracerInitializationData
{
	FGameplayTagContainer TracerTags;
	UPrimitiveComponent* TracerSource;
};

UENUM(BlueprintType)
enum class EMnhFilterType : uint8
{
    FilterSameActorAcrossAllTracers UMETA(DisplayName="FilterSameActorAcrossAllTracers"),
    FilterSameActorPerTracer UMETA(DisplayName="FilterSameActorPerTracer"),
	None UMETA(DisplayName="None")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent),
	HideCategories=(Sockets, Navigation, Tags, ComponentTick, ComponentReplication,
		Cooking, AssetUserData, Replication))
class MISSNOHIT_API UMnhTracerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMnhTracerComponent();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintAssignable)
	FMnhOnHitDetected OnHitDetected;
	
	FMnhOnTracerStarted OnTracerStarted;
	FMnhOnTracerStopped OnTracerStopped;
	FMnhTracerComponentDestroyed OnDestroyed;

	FCriticalSection TraceDoneScopeLock;

	TArray<FMnhHitCache> HitCache;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="MissNoHit", meta=(FullyExpand=true, TitleProperty="TracerTag"))
	TArray<FMnhTracerConfig> TracerConfigs;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="MissNoHit")
	EMnhFilterType FilterType;

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void InitializeTracers(const FGameplayTagContainer TracerTags, UPrimitiveComponent* TracerSource);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void StopAllTracers();

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void ResetHitCache();

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void ResetHitCacheByTags(const FGameplayTagContainer TracerTags);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void AddToIgnoredActors(const FGameplayTagContainer TracerTags, AActor* Actor);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void ResetIgnoredActors(const FGameplayTagContainer TracerTags);
	
	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void AddToIgnoredActorsArray(const FGameplayTagContainer TracerTags, TArray<AActor*> Actors);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void SetTracerTickRate(const FGameplayTagContainer TracerTags, const int32 TicksPerSecond);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void StartTracers(const FGameplayTagContainer TracerTags, const bool bResetHitCache = true);

	void StartTracersInternal(const FGameplayTagContainer TracerTags, bool bResetHitCache, bool AllowAnimNotify);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void StopTracers(FGameplayTagContainer TracerTags);

	void StopTracersDelayed(const FGameplayTagContainer TracerTags);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void StartTracersByComponent(const UPrimitiveComponent* Component);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void StopTracersByComponent(const UPrimitiveComponent* Component);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void UpdateDebugDrawState(const FGameplayTagContainer TracerTags, TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType);

	int32 FindTracerConfig(FGameplayTag TracerTag);
	
	int32 AddNewTracer(const FGameplayTag TracerTag, const EMnhTraceSource TraceSource, const FMnhTraceSettings& TraceSettings,
		EMnhTracerTickType TracerTickType, int TargetFps, int TargetDistanceTraveled,
		EDrawDebugTrace::Type DrawDebugType, float DrawDebugLifetime=0.5);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	int32 AddNewTracerConfig(const FMnhTracerConfig& TracerConfig);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	int32 AddNewTracerConfigInitialized(const FMnhTracerConfig& TracerConfig, UPrimitiveComponent* TracerSource);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void AddNewMnhComponentTracer(const FGameplayTag TracerTag,
		FMnhTraceSettings TraceSettings, EMnhTracerTickType TracerTickType, int TargetFps, int TargetDistanceTraveled,
		EDrawDebugTrace::Type DrawDebugType, float DrawDebugLifetime=0.5);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void AddNewPhysicsAssetTracer(const FGameplayTag TracerTag,
		FName SocketOrBoneName,
		FMnhTraceSettings TraceSettings, EMnhTracerTickType TracerTickType, int TargetFps, int TargetDistanceTraveled,
		EDrawDebugTrace::Type DrawDebugType, float DrawDebugLifetime=0.5);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void AddNewAnimNotifyTracer(const FGameplayTag TracerTag,
		FMnhTraceSettings TraceSettings, EMnhTracerTickType TracerTickType, int TargetFps, int TargetDistanceTraveled,
		EDrawDebugTrace::Type DrawDebugType, float DrawDebugLifetime=0.5);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void AddNewStaticMeshSocketsTracer(const FGameplayTag TracerTag,
		FName MeshSocket1, FName MeshSocket2, float MeshSocketTracerRadius, float MeshSocketTracerLengthOffset,
		FMnhTraceSettings TraceSettings, EMnhTracerTickType TracerTickType, int TargetFps, int TargetDistanceTraveled,
		EDrawDebugTrace::Type DrawDebugType, float DrawDebugLifetime=0.5);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void AddNewSkeletalMeshSocketsTracer(const FGameplayTag TracerTag,
		FName MeshSocket1, FName MeshSocket2, float MeshSocketTracerRadius, float MeshSocketTracerLengthOffset,
		FMnhTraceSettings TraceSettings, EMnhTracerTickType TracerTickType, int TargetFps, int TargetDistanceTraveled,
		EDrawDebugTrace::Type DrawDebugType, float DrawDebugLifetime=0.5);

	void RemoveTracerConfigsByPredicate(const TFunctionRef<bool(const FMnhTracerConfig& ConfigToRemove)> Predicate);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void RemoveTracerConfigsByTags(const FGameplayTagContainer TracerTags);

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void RemoveTracerConfigsByComponent(const UPrimitiveComponent* TracerSource);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual bool CheckFilters(const FHitResult& HitResult, FGameplayTag TracerTag, int TickIdxArg);

public:
	void OnTracerHitDetected(FGameplayTag TracerTag, const TArray<FHitResult>& HitResults, const float DeltaTime, const int TickIdx, const UPrimitiveComponent* Component);

private:
	bool bIsInitialized = false;
	TArray<FTracerInitializationData> EarlyTracerInitializations;
};
