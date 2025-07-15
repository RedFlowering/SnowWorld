// Copyright 2025 Snow Game Studio.

#pragma once

#include "Components/ActorComponent.h"
#include "../Managers/HarmoniaInteractionManager.h"
#include "HarmoniaInteractionComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HARMONIAKIT_API UHarmoniaInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

protected:
    UHarmoniaInteractionComponent();

    virtual void BeginPlay() override;

public:
	/** 시야에 있는 인터랙터블 감지 (트레이스) */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetTargetActor(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
    AActor* GetTargetActor();

	/** 실제 상호작용 시도 (입력 등에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TryInteract(const FHarmoniaInteractionContext& Context);

	UFUNCTION(Client, Reliable)
	void Client_OnInteractionResult(const FHarmoniaInteractionResult& Result);

protected:
    /** 현재 트레이스된 상호작용 대상 (캐싱) */
    UPROPERTY()
    TObjectPtr<AActor> TargetActor = nullptr;

    /** 트레이스 프로파일/채널 지정 (필요시 커스텀) */
    UPROPERTY(EditAnywhere, Category="Interaction")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    /** 트레이스 거리 */
    UPROPERTY(EditAnywhere, Category="Interaction")
    float TraceDistance = 300.f;

    /** 트레이스 시 소유자 시점 오프셋 (필요시) */
    UPROPERTY(EditAnywhere, Category="Interaction")
    FVector TraceStartOffset = FVector::ZeroVector;

    /** 매니저 연동 */
    UPROPERTY()
    TObjectPtr<UHarmoniaInteractionManager> InteractionManager = nullptr;
};
