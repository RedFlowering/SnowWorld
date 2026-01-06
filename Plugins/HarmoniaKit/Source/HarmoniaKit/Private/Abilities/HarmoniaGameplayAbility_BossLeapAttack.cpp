// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_BossLeapAttack.h"
#include "Core/HarmoniaCharacterMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "Animation/AnimMontage.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Monsters/HarmoniaBossMonster.h"
#include "GameFramework/Character.h"

UHarmoniaGameplayAbility_BossLeapAttack::UHarmoniaGameplayAbility_BossLeapAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LeapAngle = 45.0f;
	MinLeapDistance = 200.0f;
	MaxLeapDistance = 1500.0f;
	TargetLocationOffset = 150.0f;
	LandingDamageMultiplier = 1.5f;
}

bool UHarmoniaGameplayAbility_BossLeapAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check if leap montage is set
	if (!LeapMontage)
	{
		return false;
	}

	// Check distance to target
	APawn* OwnerPawn = Cast<APawn>(ActorInfo->AvatarActor.Get());
	if (OwnerPawn)
	{
		AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
		if (AIController)
		{
			UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
			if (BlackboardComp)
			{
				AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(FName("TargetActor")));
				if (TargetActor)
				{
					float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), TargetActor->GetActorLocation());
					if (Distance < MinLeapDistance || Distance > MaxLeapDistance)
					{
						return false;
					}
				}
			}
		}
	}

	return true;
}

void UHarmoniaGameplayAbility_BossLeapAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UHarmoniaCharacterMovementComponent* MoveComp = GetHarmoniaMovementComponent();
	if (!MoveComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossLeapAttack: No HarmoniaCharacterMovementComponent"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Cache target location
	CachedTargetLocation = GetLeapTargetLocation();

	// Clamp distance
	FVector StartLocation = Character->GetActorLocation();
	FVector ToTarget = CachedTargetLocation - StartLocation;
	float Distance = ToTarget.Size2D();

	if (Distance < MinLeapDistance)
	{
		// Too close, use minimum distance in forward direction
		CachedTargetLocation = StartLocation + Character->GetActorForwardVector() * MinLeapDistance;
	}
	else if (Distance > MaxLeapDistance)
	{
		// Too far, clamp to max distance
		FVector Direction = ToTarget.GetSafeNormal2D();
		CachedTargetLocation = StartLocation + Direction * MaxLeapDistance;
		CachedTargetLocation.Z = StartLocation.Z;
	}

	// Apply target offset (land in front of target, not on top)
	if (TargetLocationOffset > 0.0f)
	{
		FVector Direction = (CachedTargetLocation - StartLocation).GetSafeNormal2D();
		CachedTargetLocation -= Direction * TargetLocationOffset;
	}

	// Calculate leap duration from montage length
	float LeapDuration = 1.0f;
	if (LeapMontage)
	{
		LeapDuration = LeapMontage->GetPlayLength();
	}

	// Start leaping (CustomMovementMode)
	MoveComp->StartLeaping(CachedTargetLocation, LeapAngle, LeapDuration);

	// Play montage
	if (LeapMontage)
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &UHarmoniaGameplayAbility_BossLeapAttack::OnMontageCompleted);

				FOnMontageBlendingOutStarted BlendOutDelegate;
				BlendOutDelegate.BindUObject(this, &UHarmoniaGameplayAbility_BossLeapAttack::OnMontageBlendingOut);

				float PlayResult = AnimInstance->Montage_Play(LeapMontage);
				if (PlayResult > 0.0f)
				{
					AnimInstance->Montage_SetEndDelegate(EndDelegate, LeapMontage);
					AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, LeapMontage);
				}
				else
				{
					// Montage failed to play - use timer fallback
					if (UWorld* World = GetWorld())
					{
						FTimerHandle TimerHandle;
						World->GetTimerManager().SetTimer(
							TimerHandle,
							FTimerDelegate::CreateWeakLambda(this, [this, Handle, ActorInfo, ActivationInfo]()
							{
								EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
							}),
							LeapDuration,
							false
						);
					}
				}
			}
		}
	}
	else
	{
		// No montage - use timer fallback
		if (UWorld* World = GetWorld())
		{
			FTimerHandle TimerHandle;
			World->GetTimerManager().SetTimer(
				TimerHandle,
				FTimerDelegate::CreateWeakLambda(this, [this, Handle, ActorInfo, ActivationInfo]()
				{
					EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
				}),
				LeapDuration,
				false
			);
		}
	}
}

void UHarmoniaGameplayAbility_BossLeapAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Stop leaping if still in progress
	if (UHarmoniaCharacterMovementComponent* MoveComp = GetHarmoniaMovementComponent())
	{
		if (MoveComp->IsLeaping())
		{
			MoveComp->StopLeaping();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_BossLeapAttack::OnMontageCompleted(UAnimMontage* Montage, bool bInterrupted)
{
	// LandingAttackMontage가 끝나면 어빌리티 종료
	// LeapMontage Blend Out 시점에서 LandingAttackMontage 시작하므로
	// 여기서는 LandingAttackMontage 완료 여부 체크
	if (Montage == LandingAttackMontage || !LandingAttackMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bInterrupted);
	}
}

void UHarmoniaGameplayAbility_BossLeapAttack::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	// LeapMontage 블렌드 아웃 시점 = 착지 시점
	if (Montage != LeapMontage)
	{
		return;
	}

	// Stop leaping movement
	if (UHarmoniaCharacterMovementComponent* MoveComp = GetHarmoniaMovementComponent())
	{
		MoveComp->StopLeaping();
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bInterrupted);
		return;
	}

	// NOTE: Rotation is handled by Motion Warping AnimNotifyState in LandingAttackMontage
	// - For players: LockOnComponent sets the warp target
	// - For AI: BT sets TargetActor, MotionWarpingComponent handles rotation

	// Play landing attack montage if set
	if (LandingAttackMontage)
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &UHarmoniaGameplayAbility_BossLeapAttack::OnMontageCompleted);

				AnimInstance->Montage_Play(LandingAttackMontage);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, LandingAttackMontage);
				return; // Don't end ability yet, wait for landing montage
			}
		}
	}

	// No landing montage or failed to play - END ABILITY NOW!
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

FVector UHarmoniaGameplayAbility_BossLeapAttack::GetLeapTargetLocation() const
{
	APawn* OwnerPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!OwnerPawn)
	{
		return FVector::ZeroVector;
	}

	AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
	if (!AIController)
	{
		return OwnerPawn->GetActorLocation();
	}

	UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return OwnerPawn->GetActorLocation();
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(FName("TargetActor")));
	if (!TargetActor)
	{
		return OwnerPawn->GetActorLocation();
	}

	// Calculate landing position (offset from target)
	FVector Direction = (OwnerPawn->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal();
	return TargetActor->GetActorLocation() + Direction * TargetLocationOffset;
}

UHarmoniaCharacterMovementComponent* UHarmoniaGameplayAbility_BossLeapAttack::GetHarmoniaMovementComponent() const
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		return Cast<UHarmoniaCharacterMovementComponent>(Character->GetCharacterMovement());
	}
	return nullptr;
}

UMotionWarpingComponent* UHarmoniaGameplayAbility_BossLeapAttack::GetMotionWarpingComponent() const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (Avatar)
	{
		return Avatar->FindComponentByClass<UMotionWarpingComponent>();
	}
	return nullptr;
}
