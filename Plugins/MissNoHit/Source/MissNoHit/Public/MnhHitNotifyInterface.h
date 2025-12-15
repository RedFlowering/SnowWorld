// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "MnhHitNotifyInterface.generated.h"

struct FHitResult;

// This class does not need to be modified.
UINTERFACE()
class UMnhHitNotifyInterface : public UInterface
{
	GENERATED_BODY()
};

class MISSNOHIT_API IMnhHitNotifyInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent)
	void OnHitReceived(FGameplayTag TracerTag, const FHitResult& HitResult, float DeltaTime, const UPrimitiveComponent* TracerSource);
};
