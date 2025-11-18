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
	/** �þ߿� �ִ� ���ͷ��ͺ� ���� (Ʈ���̽�) */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetTargetActor(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
    AActor* GetTargetActor();

	/** ���� ��ȣ�ۿ� �õ� (�Է� ��� ȣ��) */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TryInteract(const FHarmoniaInteractionContext& Context);

protected:
    /** ���� Ʈ���̽��� ��ȣ�ۿ� ��� (ĳ��) */
    UPROPERTY()
    TObjectPtr<AActor> TargetActor = nullptr;

    /** Ʈ���̽� ��������/ä�� ���� (�ʿ�� Ŀ����) */
    UPROPERTY(EditAnywhere, Category="Interaction")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    /** Ʈ���̽� �Ÿ� */
    UPROPERTY(EditAnywhere, Category="Interaction")
    float TraceDistance = 300.f;

    /** Ʈ���̽� �� ������ ���� ������ (�ʿ��) */
    UPROPERTY(EditAnywhere, Category="Interaction")
    FVector TraceStartOffset = FVector::ZeroVector;

    /** �Ŵ��� ���� */
    UPROPERTY()
    TObjectPtr<UHarmoniaInteractionManager> InteractionManager = nullptr;
};
