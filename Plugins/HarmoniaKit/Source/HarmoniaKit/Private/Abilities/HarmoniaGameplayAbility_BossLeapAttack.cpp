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
	// Phase 1: Jump settings
	JumpAngle = 70.0f;
	JumpHeight = 400.0f;
	JumpDuration = 0.5f;

	// Phase 2: Dash Attack settings
	DashSpeed = 2000.0f;
	MaxDashDistance = 1500.0f;
	TargetStopDistance = 100.0f;
}

bool UHarmoniaGameplayAbility_BossLeapAttack::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Need at least one montage
	if (!JumpMontage && !DashAttackMontage)
	{
		return false;
	}

	// Check if target exists
	APawn* OwnerPawn = Cast<APawn>(ActorInfo->AvatarActor.Get());
	if (OwnerPawn)
	{
		AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
		if (AIController)
		{
			UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
			if (BlackboardComp)
			{
				// Note: AI Controller sets "Target", not "TargetActor"
				AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(FName("Target")));
				if (!TargetActor)
				{
					return false; // No target
				}

				// Optional: Check distance if needed
				float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), TargetActor->GetActorLocation());
				if (Distance > MaxDashDistance * 2.0f)
				{
					return false; // Too far
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

	// Cache target actor
	CachedTargetActor = GetTargetActor();

	// Start Phase 1: Jump
	CurrentPhase = 1;
	StartJumpPhase();
}

void UHarmoniaGameplayAbility_BossLeapAttack::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Clear timer
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		Avatar->GetWorldTimerManager().ClearTimer(PhaseTransitionTimerHandle);
	}

	// Stop any custom movement
	if (UHarmoniaCharacterMovementComponent* MoveComp = GetHarmoniaMovementComponent())
	{
		if (MoveComp->IsLeaping())
		{
			MoveComp->StopLeaping();
		}
	}

	CurrentPhase = 0;
	CachedTargetActor.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

//~=============================================================================
// Phase 1: Jump
//~=============================================================================

void UHarmoniaGameplayAbility_BossLeapAttack::StartJumpPhase()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UHarmoniaCharacterMovementComponent* MoveComp = GetHarmoniaMovementComponent();

	// Calculate jump target (upward from current position)
	FVector StartLocation = Character->GetActorLocation();
	FVector JumpDirection = FRotator(JumpAngle, Character->GetActorRotation().Yaw, 0.0f).Vector();
	FVector JumpTarget = StartLocation + JumpDirection * JumpHeight;

	// Start jump movement if movement component supports it
	if (MoveComp)
	{
		// Use leaping with short distance for upward jump
		MoveComp->StartLeaping(JumpTarget, JumpAngle, JumpDuration);
	}

	// Play jump montage
	if (JumpMontage)
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				FOnMontageBlendingOutStarted BlendOutDelegate;
				BlendOutDelegate.BindUObject(this, &UHarmoniaGameplayAbility_BossLeapAttack::OnJumpMontageBlendingOut);

				float PlayResult = AnimInstance->Montage_Play(JumpMontage);
				if (PlayResult > 0.0f)
				{
					AnimInstance->Montage_SetBlendingOutDelegate(BlendOutDelegate, JumpMontage);
					return; // Wait for montage blend out to transition to Phase 2
				}
			}
		}
	}

	// No montage or failed to play - use timer for phase transition
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		Avatar->GetWorldTimerManager().SetTimer(
			PhaseTransitionTimerHandle,
			this,
			&UHarmoniaGameplayAbility_BossLeapAttack::StartDashAttackPhase,
			JumpDuration,
			false
		);
	}
}

void UHarmoniaGameplayAbility_BossLeapAttack::OnJumpMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != JumpMontage)
	{
		return;
	}

	if (bInterrupted)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Transition to Phase 2
	StartDashAttackPhase();
}

//~=============================================================================
// Phase 2: Dash Attack
//~=============================================================================

void UHarmoniaGameplayAbility_BossLeapAttack::StartDashAttackPhase()
{
	CurrentPhase = 2;

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UHarmoniaCharacterMovementComponent* MoveComp = GetHarmoniaMovementComponent();

	// Stop previous movement
	if (MoveComp && MoveComp->IsLeaping())
	{
		MoveComp->StopLeaping();
	}

	// Setup Motion Warping target BEFORE playing montage
	SetupMotionWarpingTarget();

	// Calculate dash target (towards target actor)
	AActor* TargetActor = CachedTargetActor.Get();
	if (TargetActor && MoveComp)
	{
		FVector CurrentLocation = Character->GetActorLocation();
		FVector TargetLocation = TargetActor->GetActorLocation();
		
		// Stop before reaching target
		FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
		FVector DashTarget = TargetLocation - Direction * TargetStopDistance;
		
		// Calculate dash duration based on distance and speed
		float Distance = FVector::Dist(CurrentLocation, DashTarget);
		Distance = FMath::Min(Distance, MaxDashDistance);
		float DashDuration = Distance / DashSpeed;
		DashDuration = FMath::Max(DashDuration, 0.3f); // Minimum duration

		// Start dash movement (descending towards target)
		// Use negative angle for descending
		MoveComp->StartLeaping(DashTarget, -30.0f, DashDuration);
	}

	// Play dash attack montage
	if (DashAttackMontage)
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &UHarmoniaGameplayAbility_BossLeapAttack::OnDashAttackMontageCompleted);

				float PlayResult = AnimInstance->Montage_Play(DashAttackMontage);
				if (PlayResult > 0.0f)
				{
					AnimInstance->Montage_SetEndDelegate(EndDelegate, DashAttackMontage);
					return; // Wait for montage completion
				}
			}
		}
	}

	// No montage - end ability after a short delay
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		float FallbackDuration = 0.5f;
		Avatar->GetWorldTimerManager().SetTimer(
			PhaseTransitionTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			}),
			FallbackDuration,
			false
		);
	}
}

void UHarmoniaGameplayAbility_BossLeapAttack::OnDashAttackMontageCompleted(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != DashAttackMontage)
	{
		return;
	}

	// Stop movement
	if (UHarmoniaCharacterMovementComponent* MoveComp = GetHarmoniaMovementComponent())
	{
		if (MoveComp->IsLeaping())
		{
			MoveComp->StopLeaping();
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bInterrupted);
}

//~=============================================================================
// Motion Warping
//~=============================================================================

void UHarmoniaGameplayAbility_BossLeapAttack::SetupMotionWarpingTarget()
{
	if (!bEnableMotionWarping)
	{
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return;
	}

	UMotionWarpingComponent* WarpComp = GetMotionWarpingComponent();
	if (!WarpComp)
	{
		return;
	}

	AActor* TargetActor = CachedTargetActor.Get();
	if (!TargetActor)
	{
		return;
	}

	// Use component tracking for automatic target following during dash
	if (USceneComponent* TargetRoot = TargetActor->GetRootComponent())
	{
		WarpComp->AddOrUpdateWarpTargetFromComponent(
			WarpTargetName,
			TargetRoot,
			NAME_None,  // No specific bone (Warp to Feet Location handles height)
			true,       // Follow rotation as well
			FVector::ZeroVector,  // Location offset
			FRotator::ZeroRotator // Rotation offset
		);
	}
}

//~=============================================================================
// Helper Functions
//~=============================================================================

AActor* UHarmoniaGameplayAbility_BossLeapAttack::GetTargetActor() const
{
	APawn* OwnerPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!OwnerPawn)
	{
		return nullptr;
	}

	AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
	if (!AIController)
	{
		return nullptr;
	}

	UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return nullptr;
	}

	// Note: AI Controller sets "Target", not "TargetActor"
	return Cast<AActor>(BlackboardComp->GetValueAsObject(FName("Target")));
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
