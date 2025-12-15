// Copyright 2024 Eren Balatkan. All Rights Reserved.


#include "MnhTracerControl.h"

#include "MissNoHit.h"
#include "MnhTracerComponent.h"
#include "MnhTracer.h"


UMnhTracerControl* UMnhTracerControl::OnHitDetectedEvent(UMnhTracerComponent* HitTracerComponent, const FGameplayTagContainer TracerTags,
                                                        UObject* WorldContextObject)
{
	UMnhTracerControl* BlueprintNode = NewObject<UMnhTracerControl>();
	BlueprintNode->WorldContextObject = WorldContextObject;
	BlueprintNode->HitTracerComponent = HitTracerComponent;
	BlueprintNode->TracerTags = TracerTags;
	BlueprintNode->bShouldStartTracers = false;
	BlueprintNode->RegisterWithGameInstance(WorldContextObject);
	return BlueprintNode;
}

UMnhTracerControl* UMnhTracerControl::StartTracersEvent(UMnhTracerComponent* HitTracerComponent, 
                                             const FGameplayTagContainer TracerTags, bool bResetHitCache,
                                             UObject* WorldContextObject)
{
	UMnhTracerControl* BlueprintNode = OnHitDetectedEvent(HitTracerComponent, TracerTags, WorldContextObject);
	BlueprintNode->bShouldStartTracers = true;
	BlueprintNode->bResetHits = bResetHitCache;
	return BlueprintNode;
}


UMnhTracerControl* UMnhTracerControl::StartAllTracersEvent(UMnhTracerComponent* HitTracerComponent, bool bResetHitCache,
																			  UObject* WorldContextObject)
{
	return StartTracersEvent(HitTracerComponent, {}, bResetHitCache, WorldContextObject);
}

void UMnhTracerControl::Activate()
{
	Super::Activate();
	if (TracerTags.Num() == 0)
	{
		if (!HitTracerComponent)
		{
			SetReadyToDestroy();
			return;
		}
		
		for (const auto TracerConfig : HitTracerComponent->TracerConfigs)
		{
			if (TracerConfig.TraceSource != EMnhTraceSource::AnimNotify)
			{
				TracerTags.AddTag(TracerConfig.TracerTag);
			}
		}
	}
	
	if(WorldContextObject == nullptr || !HitTracerComponent)
	{
		const FString ErrorMsg = FString::Printf(TEXT("MissNoHit Tracer Control Node cannot be null!"));
		FMnhHelpers::Mnh_Log(ErrorMsg);
		SetReadyToDestroy();
		return;
	}
	
	if (bShouldStartTracers)
	{
		HitTracerComponent->StartTracers(TracerTags, bResetHits);
	}
	HitTracerComponent->OnHitDetected.AddDynamic(this, &UMnhTracerControl::OnHitDetected);
	HitTracerComponent->OnDestroyed.AddDynamic(this, &UMnhTracerControl::OnComponentDestroyed);
	
	bIsActive = true;
	OnInitialized.Broadcast({}, {}, 0.f, nullptr, this);
}

void UMnhTracerControl::StopEvent()
{
	bIsActive = false;
	HitTracerComponent->OnHitDetected.RemoveDynamic(this, &UMnhTracerControl::OnHitDetected);
	SetReadyToDestroy();
}

void UMnhTracerControl::OnHitDetected(FGameplayTag TracerTag, const FHitResult& HitResult, float DeltaTime, const UPrimitiveComponent* Component)
{
	SCOPE_CYCLE_COUNTER(STAT_MnhTracerHitDetectedAsyncNode)
	if (bIsActive)
	{
		OnHit.Broadcast(TracerTag, HitResult, DeltaTime, Component, this);
	}
}

void UMnhTracerControl::OnComponentDestroyed()
{
	StopEvent();
}
