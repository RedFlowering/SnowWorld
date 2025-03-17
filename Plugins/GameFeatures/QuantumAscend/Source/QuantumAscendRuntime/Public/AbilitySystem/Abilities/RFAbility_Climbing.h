// Copyright 2025 RedFlowering.

#pragma once 

#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "RFAbility_Climbing.generated.h"

struct FInputActionValue;
class ARFCharacter;
class URFCharacterMovementComponent;
class UAnimMontage;

UCLASS(Abstract)
class QUANTUMASCENDRUNTIME_API URFAbility_Climbing : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	URFAbility_Climbing(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
   
   // 어빌리티 실행
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    // 어빌리티 종료
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
    void TickAbility(float DeltaTime);

protected:
	UPROPERTY()
	TObjectPtr<ARFCharacter> OwnerCharacter = nullptr;

	UPROPERTY()
	TObjectPtr<URFCharacterMovementComponent> OwnerMovementComponent = nullptr;

protected:
    // 벽 감지 함수
    bool DetectClimbableWall(FHitResult& OutHit);

    // 벽 방향 입력 감지
    bool IsMovingTowardWall();

	UFUNCTION()
    void MoveAlongWall(float DeltaTime);

	UFUNCTION()
    void MoveInputVector(const FInputActionValue& InputActionValue);

	UFUNCTION()
    void StopInputVector(const FInputActionValue& InputActionValue);

    // 벽 위로 오르기 체크
    bool CanClimbOver();

    void CancelClimbing();

protected:
    FVector WallNormal = FVector::ZeroVector;
    FVector WallImpactPoint = FVector::ZeroVector;
    FVector ClimbMoveVector = FVector::ZeroVector;
    bool StartClimbing = false;

public:
	UPROPERTY(EditAnywhere, Category = "Climbing|Setup")
    TObjectPtr<UAnimMontage> ClimbingMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Climbing|Setup")
    float ClimbSpeed = 10.0f;

};

