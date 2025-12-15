// Copyright 2024 Eren Balatkan. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MnhTracerComponent.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "MnhTracerControl.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FMnhOnHitDetectedBP, FGameplayTag, TracerTag, FHitResult, HitResult, float, DeltaTime,
	const UPrimitiveComponent*, TracerSource, UMnhTracerControl*, TracerControlNode);
class FMnhOnHitDetected;
class UMnhTracerComponent;

/**
 * 
 */
UCLASS()
class MISSNOHIT_API UMnhTracerControl : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FMnhOnHitDetectedBP OnInitialized;
	
	UPROPERTY(BlueprintAssignable)
	FMnhOnHitDetectedBP OnHit;
	
	TObjectPtr<UMnhTracerComponent> HitTracerComponent;
	FGameplayTagContainer TracerTags;
	bool bResetHits;
	bool bIsActive;
	bool bShouldStartTracers;

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "MissNoHit")
	static UMnhTracerControl* OnHitDetectedEvent(UMnhTracerComponent* HitTracerComponent,
		const FGameplayTagContainer TracerTags,
		UObject* WorldContextObject=nullptr);
	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "MissNoHit")
	static UMnhTracerControl* StartTracersEvent(UMnhTracerComponent* HitTracerComponent,
		const FGameplayTagContainer TracerTags, bool bResetHitCache=true,
		UObject* WorldContextObject=nullptr);

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "MissNoHit")
	static UMnhTracerControl* StartAllTracersEvent(UMnhTracerComponent* HitTracerComponent, bool bResetHitCache=true,
		UObject* WorldContextObject=nullptr);

	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, Category="MissNoHit")
	void StopEvent();

protected:
	UFUNCTION()
	void OnHitDetected(FGameplayTag TracerTag, const FHitResult& HitResult, float DeltaTime, const UPrimitiveComponent* Component);

	UFUNCTION()
	void OnComponentDestroyed();
    
private:
	UObject* WorldContextObject;
};
