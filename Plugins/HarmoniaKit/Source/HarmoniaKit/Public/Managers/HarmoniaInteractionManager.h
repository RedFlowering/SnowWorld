// Copyright 2025 Snow Game Studio.

#pragma once

#include "UObject/Object.h"
#include "Delegates/DelegateCombinations.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "../Definitions/HarmoniaInteractionSystemDefinitions.h"
#include "HarmoniaInteractionManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionCompleted, const FHarmoniaInteractionContext&, Context, const FHarmoniaInteractionResult&, Result);

UCLASS()
class HARMONIAKIT_API UHarmoniaInteractionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    /** 상호작용 시도: Context 정보를 받아 상호작용을 처리 */
    UFUNCTION(BlueprintCallable, Category="Interaction")
    void TryInteract(const FHarmoniaInteractionContext& Context);

    /** 상호작용 완료 시 발생 (블루프린트 등에서 구독 가능) */
    UPROPERTY(BlueprintAssignable, Category="Interaction")
    FOnInteractionCompleted OnInteractionCompleted;

protected:
    /** Handle interaction logic */
    void HandleInteraction(const FHarmoniaInteractionContext& Context);

    /** Send Gameplay Event to Interactor if they have ASC */
    bool SendInteractionEvent(const FHarmoniaInteractionContext& Context);
};