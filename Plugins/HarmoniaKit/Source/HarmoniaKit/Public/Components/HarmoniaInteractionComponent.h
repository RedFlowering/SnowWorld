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
	/** Set currently targeted interactable actor (trace) */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetTargetActor(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
    AActor* GetTargetActor();

	/** Try to interact with target (called from input) */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TryInteract(const FHarmoniaInteractionContext& Context);

protected:
    /** Currently traced interactable target (cached) */
    UPROPERTY()
    TObjectPtr<AActor> TargetActor = nullptr;

    /** Trace collision channel (customize if needed) */
    UPROPERTY(EditAnywhere, Category="Interaction")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    /** Trace distance */
    UPROPERTY(EditAnywhere, Category="Interaction")
    float TraceDistance = 300.f;

    /** Trace start offset from camera (optional) */
    UPROPERTY(EditAnywhere, Category="Interaction")
    FVector TraceStartOffset = FVector::ZeroVector;

    /** Manager reference */
    UPROPERTY()
    TObjectPtr<UHarmoniaInteractionManager> InteractionManager = nullptr;
};
