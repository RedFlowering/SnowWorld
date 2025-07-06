// Copyright 2025 Snow Game Studio.

#include "AbilitySystem/Abilities/QAAbility_Climbing.h"
#include "Character/BaseCharacter.h"
#include "Character/BaseCharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Character/LyraPawnExtensionComponent.h"
#include "Character/LyraPawnData.h"
#include "Input/LyraInputComponent.h"
#include "Tags/QAGameplayTags.h"
#include "AbilitySystem/Abilities/QAAbilityTask_WaitTick.h"
#include "Niagara/Classes/NiagaraSystem.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Niagara/Public/NiagaraComponent.h"

UQAAbility_Climbing::UQAAbility_Climbing(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UQAAbility_Climbing::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    if (ActorInfo && ActorInfo->AvatarActor.IsValid())
    {
        ABaseCharacter* AbilityCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
        if (AbilityCharacter)
        {
            UBaseCharacterMovementComponent* AbilityMovementComponent = Cast<UBaseCharacterMovementComponent>(AbilityCharacter->GetCharacterMovement());

            if (AbilityMovementComponent)
            {
                return true;
            }
        }
    }

    return false;
}

void UQAAbility_Climbing::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    OwnerCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());

    if (OwnerCharacter && IsLocallyControlled())
    {
        OwnerMovementComponent = Cast<UBaseCharacterMovementComponent>(OwnerCharacter->GetCharacterMovement());

        const ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(OwnerCharacter);
        const ULyraPawnData* PawnData = PawnExtComp->GetPawnData<ULyraPawnData>();
        const ULyraInputConfig* InputConfig = PawnData->InputConfig;
        ULyraInputComponent* LyraIC = Cast<ULyraInputComponent>(OwnerCharacter->InputComponent);

        if (PawnExtComp && PawnData && InputConfig && LyraIC)
        {
            LyraIC->BindNativeAction(InputConfig, QAGameplayTags::Ability_Climbing, ETriggerEvent::Triggered, this, &UQAAbility_Climbing::MoveInputVector, /*bLogIfNotFound=*/ false);
            LyraIC->BindNativeAction(InputConfig, QAGameplayTags::Ability_Climbing, ETriggerEvent::Completed, this, &UQAAbility_Climbing::StopInputVector, /*bLogIfNotFound=*/ false);
        }

        // Creating a custom AbilityTask and binding a Tick delegate
        UQAAbilityTask_WaitTick* TickTask = UQAAbilityTask_WaitTick::WaitTick(this);

        if (TickTask && !TickTask->IsActive())
        {
            TickTask->OnTick.AddDynamic(this, &UQAAbility_Climbing::TickAbility);
            TickTask->ReadyForActivation();
        }
    }
}

void UQAAbility_Climbing::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UQAAbility_Climbing::TickAbility(float DeltaTime)
{
    MoveAlongWall(DeltaTime);
}

bool UQAAbility_Climbing::DetectClimbableWall(FHitResult& OutHit)
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

bool UQAAbility_Climbing::IsMovingTowardWall()
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

void UQAAbility_Climbing::MoveAlongWall(float DeltaTime)
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

                if (ClimbingDownNiagara)
                {
                    ClimbingDownNiagara->DestroyComponent();
                }
            }
            else
            {
                // Move
                if (UpMoveInput > 0.0f && ClimbingUpMontage && !AnimInstance->Montage_IsPlaying(ClimbingUpMontage))
                {
                    // Up Move
                    OwnerMovementComponent->SetClimbingMovementVector(ClimbUpSpeed * UpVector * UpMoveInput);
                    AnimInstance->Montage_Play(ClimbingUpMontage);

                    if (ClimbingDownNiagara)
                    {
                        ClimbingDownNiagara->DestroyComponent();
                    }
                }
                else if(UpMoveInput < 0.0f && ClimbingDownMontage && !AnimInstance->Montage_IsPlaying(ClimbingDownMontage))
                {
                    // Down Move
                    OwnerMovementComponent->SetClimbingMovementVector(ClimbDownSpeed * UpVector * UpMoveInput);
                    AnimInstance->Montage_Play(ClimbingDownMontage);

                    if (ClimbingDownEffect && GetWorld())
                    {
                        FFXSystemSpawnParameters Params;
                        Params.WorldContextObject = GetWorld();
                        Params.SystemTemplate = ClimbingDownEffect;
                        Params.Location = DownEffectLocation;
                        Params.Rotation = FRotator(DownEffectRotation.X, DownEffectRotation.Z, DownEffectRotation.Y);
                        Params.Scale = DownEffectScale;
                        Params.AttachToComponent = OwnerCharacter->GetRootComponent();
                        Params.AttachPointName = NAME_None;
                        Params.LocationType = EAttachLocation::SnapToTarget;
                        ClimbingDownNiagara = UNiagaraFunctionLibrary::SpawnSystemAttachedWithParams(Params);
                    }
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

void UQAAbility_Climbing::MoveInputVector(const FInputActionValue& InputActionValue)
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

void UQAAbility_Climbing::StopInputVector(const FInputActionValue& InputActionValue)
{
    if (OwnerMovementComponent && OwnerMovementComponent->GetCustomMovementMode() == ERFCustomMovementMode::MOVE_Climbing && OwnerMovementComponent->GetClimbingMovementVector() != FVector::ZeroVector)
    {
        ClimbInputVector = FVector::ZeroVector;
        OwnerMovementComponent->Velocity = FVector::ZeroVector;
    }
}

bool UQAAbility_Climbing::CanClimbOver()
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

void UQAAbility_Climbing::CancelClimbing()
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

        if (ClimbingDownNiagara)
        {
            ClimbingDownNiagara->DestroyComponent();
        }
    }

    // EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, /*bReplicateEndAbility=*/ true, /*bWasCancelled=*/ false);
}
