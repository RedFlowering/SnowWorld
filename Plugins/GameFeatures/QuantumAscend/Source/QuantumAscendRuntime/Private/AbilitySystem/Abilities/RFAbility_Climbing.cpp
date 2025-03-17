// Copyright 2025 RedFlowering.

#include "AbilitySystem/Abilities/RFAbility_Climbing.h"
#include "Character/RFCharacter.h"
#include "Character/RFCharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Character/LyraPawnExtensionComponent.h"
#include "Character/LyraPawnData.h"
#include "Input/LyraInputComponent.h"
#include "Tags/RFGameplayTags.h"
#include "AbilitySystem/Abilities/RFAbilityTask_WaitTick.h"

URFAbility_Climbing::URFAbility_Climbing(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool URFAbility_Climbing::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        ARFCharacter* AbilityCharacter = Cast<ARFCharacter>(ActorInfo->AvatarActor.Get());
        if (AbilityCharacter)
        {
            URFCharacterMovementComponent* AbilityMovementComponent =
                Cast<URFCharacterMovementComponent>(AbilityCharacter->GetCharacterMovement());

            if (AbilityMovementComponent)
            {
                return true;
            }
        }
    }

    return false;
}

void URFAbility_Climbing::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    OwnerCharacter = Cast<ARFCharacter>(ActorInfo->AvatarActor.Get());

    if (OwnerCharacter)
    {
        OwnerMovementComponent = Cast<URFCharacterMovementComponent>(OwnerCharacter->GetCharacterMovement());

        const ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(OwnerCharacter);
        const ULyraPawnData* PawnData = PawnExtComp->GetPawnData<ULyraPawnData>();
        const ULyraInputConfig* InputConfig = PawnData->InputConfig;
        ULyraInputComponent* LyraIC = Cast<ULyraInputComponent>(OwnerCharacter->InputComponent);

        if (PawnExtComp && PawnData && InputConfig && LyraIC)
        {
            LyraIC->BindNativeAction(InputConfig, RFGameplayTags::Ability_Climbing, ETriggerEvent::Triggered, this, &URFAbility_Climbing::MoveInputVector, /*bLogIfNotFound=*/ false);
            LyraIC->BindNativeAction(InputConfig, RFGameplayTags::Ability_Climbing, ETriggerEvent::Completed, this, &URFAbility_Climbing::StopInputVector, /*bLogIfNotFound=*/ false);
        }

        // Creating a custom AbilityTask and binding a Tick delegate
        URFAbilityTask_WaitTick* TickTask = URFAbilityTask_WaitTick::WaitTick(this);

        if (TickTask && !TickTask->IsActive())
        {
            TickTask->OnTick.AddDynamic(this, &URFAbility_Climbing::TickAbility);
            TickTask->ReadyForActivation();
        }
    }
}

void URFAbility_Climbing::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URFAbility_Climbing::TickAbility(float DeltaTime)
{
    MoveAlongWall(DeltaTime);
}

bool URFAbility_Climbing::DetectClimbableWall(FHitResult& OutHit)
{
    UWorld* World = GetWorld();

    if (World && OwnerCharacter)
    {
        FVector Start = OwnerCharacter->GetActorLocation();
        FVector Forward = OwnerCharacter->GetActorForwardVector();
        FVector End = Start + (Forward * OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius()*1.5f); // 캡슐 간격 감지
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(OwnerCharacter);

        // 벽 감지
        bool bHit = World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);

        if (bHit && OutHit.Normal.Z < 0.1f) // 수직 벽인지 체크
        {
            WallNormal = OutHit.Normal;
            return true;
        }
    }

    return false;
}

bool URFAbility_Climbing::IsMovingTowardWall()
{
    float MoveWall = ClimbInputVector.X;

    // WallNormal == 벽의 법선 벡터
    // MoveWall == 플레이어의 이동방향
    if ((WallNormal.X < 0.0f && MoveWall < 0.0f) || (WallNormal.X > 0.0f && MoveWall > 0.0f))
    {
        return true;
    }

	return false; // 벽과 유사한 방향이면 true
}

void URFAbility_Climbing::MoveAlongWall(float DeltaTime)
{
    FHitResult WallHit;

    if (DetectClimbableWall(WallHit) && OwnerCharacter && OwnerMovementComponent)
    {
        StartClimbing = true;
        if (OwnerMovementComponent->GetCustomMovementMode() != ERFCustomMovementMode::MOVE_Climbing)
        {
            // 클라이밍 모드 활성화
            OwnerMovementComponent->SetMovementMode(EMovementMode::MOVE_Custom, 2); // Climbing Movement Mode
            OwnerMovementComponent->Velocity = FVector::ZeroVector;
        }

        // 벽을 따라 이동
        //FVector RightVector = FVector::CrossProduct(FVector::UpVector, WallNormal);
        //float RightMoveInput = OwnerCharacter->GetLastMovementInputVector().Y;

        //OwnerMovementComponent->SetClimbingMovementVector(RightVector * RightMoveInput);

        // 벽을 따라 위/아래 이동
        FVector UpVector = FVector::UpVector;
        float UpMoveInput = ClimbInputVector.Z; // 위/아래 입력 감지


        // 클라이밍 애니메이션 재생 (몽타주 사용 가능)
        UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();

        if (AnimInstance)
        {
            if (ClimbInputVector == FVector::ZeroVector && ClimbingIdleMontage && !AnimInstance->Montage_IsPlaying(ClimbingIdleMontage))
            {
                // Idle
                OwnerMovementComponent->SetClimbingMovementVector(FVector::ZeroVector);
                AnimInstance->Montage_Play(ClimbingIdleMontage);
            }
            else
            {
                // Move
                if (UpMoveInput > 0.0f && ClimbingUpMontage && !AnimInstance->Montage_IsPlaying(ClimbingUpMontage))
                {
                    // Up Move
                    OwnerMovementComponent->SetClimbingMovementVector(ClimbUpSpeed * UpVector * UpMoveInput);
                    AnimInstance->Montage_Play(ClimbingUpMontage);
                }
                else if(UpMoveInput < 0.0f && ClimbingDownMontage && !AnimInstance->Montage_IsPlaying(ClimbingDownMontage))
                {
                    // Down Move
                    OwnerMovementComponent->SetClimbingMovementVector(ClimbDownSpeed * UpVector * UpMoveInput);
                    AnimInstance->Montage_Play(ClimbingDownMontage);
                }
            }
        }
    }
    else
    {
        if (StartClimbing)
        {
            CancelClimbing();
        }
    }
}

void URFAbility_Climbing::MoveInputVector(const FInputActionValue& InputActionValue)
{
    if (OwnerMovementComponent->GetMovementMode() == ERFMovementMode::MOVE_Custom && OwnerMovementComponent->GetCustomMovementMode() == ERFCustomMovementMode::MOVE_Climbing)
    {
        FVector2D InputValue = InputActionValue.Get<FVector2D>(); // 2D 축 입력 받기

        ClimbInputVector.Z = InputValue.Y;
        ClimbInputVector.X = InputValue.X;

        if (IsMovingTowardWall())
        {
            CancelClimbing();
        }
    }
}

void URFAbility_Climbing::StopInputVector(const FInputActionValue& InputActionValue)
{
    if (OwnerMovementComponent && OwnerMovementComponent->GetCustomMovementMode() == ERFCustomMovementMode::MOVE_Climbing && OwnerMovementComponent->GetClimbingMovementVector() != FVector::ZeroVector)
    {
        ClimbInputVector = FVector::ZeroVector;
        OwnerMovementComponent->Velocity = FVector::ZeroVector;
    }
}

bool URFAbility_Climbing::CanClimbOver()
{
    if (OwnerCharacter)
    {
        FVector ClimbCheckStart = WallImpactPoint + FVector(0, 0, OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.0f); // 캡슐 위 체크
        FVector ClimbCheckEnd = ClimbCheckStart + FVector(0, 0, OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius()); // 캡슐 옆 체크

        FHitResult OverHit;
        return !GetWorld()->LineTraceSingleByChannel(OverHit, ClimbCheckStart, ClimbCheckEnd, ECC_Visibility);
    }
   
   return false;
}

void URFAbility_Climbing::CancelClimbing()
{
    if (OwnerCharacter && OwnerMovementComponent)
    {
        StartClimbing = false;
        WallNormal = FVector::ZeroVector;
        WallImpactPoint = FVector::ZeroVector;
        ClimbInputVector = FVector::ZeroVector;

        OwnerMovementComponent->SetMovementMode(MOVE_Falling); // 원래 상태로 복원
        OwnerMovementComponent->SetClimbingMovementVector(FVector::ZeroVector);

        UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();

        if (AnimInstance)
        {
            AnimInstance->StopAllMontages(1.0f);
        }
    }

    // EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, /*bReplicateEndAbility=*/ true, /*bWasCancelled=*/ false);
}
