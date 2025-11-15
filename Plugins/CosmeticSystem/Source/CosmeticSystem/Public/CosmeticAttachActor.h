// Copyright 2024 HGsofts, All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CosmeticAttachActor.generated.h"

USTRUCT(BlueprintType)
struct COSMETICSYSTEM_API FCosmeticAttachActorInfo
{
	GENERATED_BODY()

	FCosmeticAttachActorInfo()	{}

	UPROPERTY(EditAnywhere, Category="CosmeticAttachActorInfo")
	FName AttachSocket;

	UPROPERTY(EditAnywhere, Category="CosmeticAttachActorInfo")
	FTransform AttachTransform = FTransform::Identity;
};

UCLASS(Abstract)
class COSMETICSYSTEM_API ACosmeticAttachActor : public AActor
{
	GENERATED_BODY()

public:
	ACosmeticAttachActor(const FObjectInitializer& ObjectInitializer);

	void SetAttachSocket(FName Socket, FTransform AttachTransform);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_AttachInfo(FCosmeticAttachActorInfo OldValue);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CosmeticAttachActor")
	TObjectPtr<USceneComponent> LinkSceneComponent = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_AttachInfo)
	FCosmeticAttachActorInfo AttachInfo;
};

