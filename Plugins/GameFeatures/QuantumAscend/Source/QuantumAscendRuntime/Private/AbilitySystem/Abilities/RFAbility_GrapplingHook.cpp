// Copyright 2025 RedFlowering.

#include "AbilitySystem/Abilities/RFAbility_GrapplingHook.h"
#include "Character/RFCharacter.h"
#include "Character/RFCharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Actors/HookActor.h"
#include "Actors/RopeActor.h"
#include "Player/LyraPlayerController.h"
#include "Character/LyraPawnExtensionComponent.h"
#include "Character/LyraPawnData.h"
#include "Input/LyraInputComponent.h"
#include "Tags/RFGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RFAbility_GrapplingHook)

URFAbilityTask_WaitTick* URFAbilityTask_WaitTick::WaitTick(UGameplayAbility* OwningAbility)
{
	URFAbilityTask_WaitTick* TickTask = NewAbilityTask<URFAbilityTask_WaitTick>(OwningAbility);
	return TickTask;
}

void URFAbilityTask_WaitTick::Activate()
{
	// Set to receive a tick every frame
	bTickingTask = true;
}

void URFAbilityTask_WaitTick::TickTask(float DeltaTime)
{
	// If the Ability is valid, it calls the OnTick delegate.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnTick.Broadcast(DeltaTime);
	}
}

void URFAbilityTask_WaitTick::OnDestroy(bool AbilityEnded)
{
	// Stop Tick when AbilityTask is destroyed
	bTickingTask = false;
	Super::OnDestroy(AbilityEnded);
}

URFAbility_GrapplingHook::URFAbility_GrapplingHook(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool URFAbility_GrapplingHook::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		ARFCharacter* AbilityCharacter = Cast<ARFCharacter>(ActorInfo->AvatarActor.Get());
		if (AbilityCharacter)
		{
			URFCharacterMovementComponent* AbilityMovementComponent =
				Cast<URFCharacterMovementComponent>(AbilityCharacter->GetCharacterMovement());

			if (AbilityMovementComponent && (AbilityMovementComponent->IsMovingOnGround() || AbilityMovementComponent->IsFalling()))
			{
				return true;
			}
		}
	}

	return false;
}

void URFAbility_GrapplingHook::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	OwnerCharacter = Cast<ARFCharacter>(ActorInfo->AvatarActor.Get());

	if (OwnerCharacter)
	{
		OwnerMovementComponent = Cast<URFCharacterMovementComponent>(OwnerCharacter->GetCharacterMovement());
	}

	ALyraPlayerController* OwnerPlayerController = Cast<ALyraPlayerController>(ActorInfo->PlayerController);

	if (OwnerPlayerController && OwnerCharacter && OwnerMovementComponent)
	{
		FHitResult HitResult;

		if (OwnerPlayerController->GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
		{
			// Fire the hook using the ImpactPoint as is
			FVector TargetLocation = HitResult.ImpactPoint;
			TargetLocation = OwnerMovementComponent->ConstrainLocationToPlane(TargetLocation);

			FHitResult FinalHit;
			FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(GrapplingHookTrace), /*bTraceComplex=*/true, OwnerCharacter);
			TraceParams.bReturnPhysicalMaterial = false;
			TraceParams.AddIgnoredActor(OwnerCharacter);

			UWorld* World = GetWorld();
			if (World)
			{
				bool IsHit = World->LineTraceSingleByChannel(FinalHit, OwnerCharacter->GetActorLocation(), TargetLocation, CollisionChannel, TraceParams);

#if WITH_EDITOR
				if (UseDebugMode)
				{
					DrawDebugLine(World, OwnerCharacter->GetActorLocation(), TargetLocation, IsHit ? FColor::Green : FColor::Red, /*bPersistentLines=*/false, DebugTraceLifeTime);
				}
#endif
			}

			if (FinalHit.bBlockingHit)
			{
				GrapplingTargetLocation = FinalHit.ImpactPoint;
				GrapplingTargetDistance = FVector::Distance(GrapplingTargetLocation, OwnerCharacter->GetActorLocation());
				if (PerformTrace(GrapplingTargetLocation))
				{
					ShootGrapplingHook(GrapplingTargetLocation);
				}
			}

			const ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(OwnerCharacter);
			const ULyraPawnData* PawnData = PawnExtComp->GetPawnData<ULyraPawnData>();
			const ULyraInputConfig* InputConfig = PawnData->InputConfig;
			ULyraInputComponent* LyraIC = Cast<ULyraInputComponent>(OwnerCharacter->InputComponent);

			if (PawnExtComp && PawnData && InputConfig && LyraIC)
			{
				LyraIC->BindNativeAction(InputConfig, RFGameplayTags::Ability_GrapplingHook_Pulling, ETriggerEvent::Started, this, &URFAbility_GrapplingHook::StartMoveToTarget, /*bLogIfNotFound=*/ false);
				LyraIC->BindNativeAction(InputConfig, RFGameplayTags::Ability_GrapplingHook_Cancel, ETriggerEvent::Completed, this, &URFAbility_GrapplingHook::CancelGrapplingHook, /*bLogIfNotFound=*/ false);
			}
		}

		// Creating a custom AbilityTask and binding a Tick delegate
		URFAbilityTask_WaitTick* TickTask = URFAbilityTask_WaitTick::WaitTick(this);

		if (TickTask)
		{
			TickTask->OnTick.AddDynamic(this, &URFAbility_GrapplingHook::TickAbility);
			TickTask->ReadyForActivation();
		}
	}
}

void URFAbility_GrapplingHook::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URFAbility_GrapplingHook::TickAbility(float DeltaTime)
{
	SwingMovement(GrapplingTargetLocation);
}

bool URFAbility_GrapplingHook::PerformTrace(FVector TargetPos)
{
	UWorld* World = GetWorld();

	if (World && OwnerCharacter)
	{
		FVector StartLocation = OwnerCharacter->GetMesh()->GetBoneLocation(FName("hand_r"));
		FVector EndLocation = TargetPos;

		FHitResult HitResult;
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(OwnerCharacter);  // 자신의 액터는 무시

		bool IsHit = World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, CollisionChannel, TraceParams);

#if WITH_EDITOR
		if(UseDebugMode)
		{
			if (IsHit)
			{
				// 타깃에 명중한 경우 녹색 라인 출력
				DrawDebugLine(GetWorld(), StartLocation, HitResult.Location, FColor::Green, false, DebugTraceLifeTime, 0, 2.f);
				UE_LOG(LogTemp, Log, TEXT("Hit at location: %s"), *HitResult.Location.ToString());
			}
			else
			{
				// 명중하지 않은 경우 빨간색 라인 출력
				DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, DebugTraceLifeTime, 0, 2.f);
				UE_LOG(LogTemp, Log, TEXT("No hit"));
			}
		}
#endif
		return IsHit;
	}

	return false;
}

void URFAbility_GrapplingHook::ShootGrapplingHook(FVector TargetPos)
{
	if (OwnerCharacter)
	{
		OwnerCharacter->SetRightHandIK(FTransform(FRotator::ZeroRotator, TargetPos, FVector(1.0f, 1.0f, 1.0f)));
		OwnerCharacter->SetUseRightHandIK(true);
	}

	ServerShootGrpplingHook(TargetPos);
}

void URFAbility_GrapplingHook::ServerShootGrpplingHook_Implementation(FVector TargetPos)
{
	UWorld* World = GetWorld();

	if (World && OwnerCharacter && RopeActorClass && HookActorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerCharacter;
		SpawnParams.Instigator = OwnerCharacter->GetInstigator();
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Using the player's right hand socket position as the Hook spawn location
		FVector SpawnLocation = OwnerCharacter->GetMesh()->GetSocketLocation(FName("hand_r"));

		// Compute rotation from SpawnLocation towards TargetLocation
		FRotator HookRotation = (TargetPos - SpawnLocation).Rotation();

		// Hook Actor
		CachedHookActor = World->SpawnActor<AHookActor>(HookActorClass, SpawnLocation, HookRotation, SpawnParams);

		if (CachedHookActor)
		{
			CachedHookActor->SetReplicates(true);
			CachedHookActor->MoveToTarget(TargetPos);
			CachedHookActor->OnHookArrived.AddDynamic(this, &URFAbility_GrapplingHook::OnHookArrivedHandler);
		}

		//Rope Actor
		CachedRopeActor = World->SpawnActor<ARopeActor>(RopeActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

		if (CachedRopeActor && CachedHookActor)
		{
			CachedRopeActor->SetReplicates(true);
			CachedRopeActor->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("hand_r"));
			CachedRopeActor->UpdateCable(OwnerCharacter->GetMesh(), FName("hand_r"), Cast<UPrimitiveComponent>(CachedHookActor->Collision), NAME_None);
		}
	}
}

void URFAbility_GrapplingHook::ReleaseGrpplingHook()
{
	ServerReleaseGrapplingHook();
}

void URFAbility_GrapplingHook::ServerReleaseGrapplingHook_Implementation()
{
	UWorld* World = GetWorld();

	if (World)
	{
		if (CachedHookActor)
		{
			World->DestroyActor(CachedHookActor);
			CachedHookActor = nullptr;
		}

		if (CachedRopeActor)
		{
			World->DestroyActor(CachedRopeActor);
			CachedRopeActor = nullptr;
		}
	}
}

void URFAbility_GrapplingHook::CancelGrapplingHook()
{
	Step = EGrappleStep::Idle;

	ReleaseGrpplingHook();

	if (OwnerCharacter && OwnerMovementComponent)
	{
		OwnerCharacter->SetRightHandIK(FTransform::Identity);
		OwnerCharacter->SetUseRightHandIK(false);

		OwnerMovementComponent->AirControl = 0.15f;
		GrapplingTargetLocation = FVector::ZeroVector;
		GrapplingTargetDistance = 0.0f;

		OwnerMovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, /*bReplicateEndAbility=*/ true, /*bWasCancelled=*/ false);
}

void URFAbility_GrapplingHook::OnHookArrivedHandler()
{
	Step = EGrappleStep::Ready;
}

void URFAbility_GrapplingHook::SwingMovement(FVector TargetPos)
{
	if (TargetPos.Z > OwnerCharacter->GetActorLocation().Z)
	{
		switch (Step)
		{
		case EGrappleStep::Ready:
		{
			if (OwnerCharacter && OwnerMovementComponent)
			{
				// 훅 위치 바라보기
				FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(OwnerCharacter->GetActorLocation(), TargetPos);
				FRotator NewRot = FRotator(OwnerCharacter->GetActorRotation().Pitch, LookAt.Yaw, OwnerCharacter->GetActorRotation().Roll);
				OwnerCharacter->SetActorRotation(NewRot);

				// 세팅
				OwnerMovementComponent->AirControl = 2.0f;
				GrapplingTargetDistance = FVector::Distance(TargetPos, OwnerCharacter->GetActorLocation());

				if (OwnerMovementComponent->IsFalling())
				{
					Step = EGrappleStep::Start;
				}
			}
			else
			{
				Step = EGrappleStep::Finish;
			}
		}
		break;
		case EGrappleStep::Start:
		{
			if (OwnerCharacter && OwnerMovementComponent)
			{
				// 이동 계산
				FVector MoveDir = OwnerCharacter->GetActorLocation() - TargetPos;
				float SwingDot = FVector::DotProduct(OwnerMovementComponent->Velocity, MoveDir);
				FVector SwingMove = -2.0f * MoveDir.GetSafeNormal() * SwingDot;

				OwnerMovementComponent->SetMovementMode(EMovementMode::MOVE_Custom, 1);
				OwnerMovementComponent->AddForce(SwingMove);

				// 이동 방향 바라보기
				FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(OwnerCharacter->GetActorLocation(), OwnerMovementComponent->Velocity.GetSafeNormal() * 1000.0f);
				FRotator NewRot = FRotator(OwnerCharacter->GetActorRotation().Pitch, LookAt.Yaw, OwnerCharacter->GetActorRotation().Roll);
				OwnerCharacter->SetActorRotation(NewRot);

				// 중력 보정
				float Distance = FVector::Distance(GrapplingTargetLocation, OwnerCharacter->GetActorLocation());
				float AdjustSize = Distance - GrapplingTargetDistance;

				if (TargetPos.Z > OwnerCharacter->GetActorLocation().Z && AdjustSize > 0.0f)
				{
					FVector AdjustVector = (TargetPos - OwnerCharacter->GetActorLocation()).GetSafeNormal() * AdjustSize;
					OwnerMovementComponent->SetGrapplingHookMovementVector(AdjustVector);
					GEngine->AddOnScreenDebugMessage(1, 0.5f, FColor::Blue, FString::Printf(TEXT("UP : %f, OriginDist : %f, AdjustDist : %f"), AdjustSize, GrapplingTargetDistance, Distance));
				}
				else
				{
					OwnerMovementComponent->SetGrapplingHookMovementVector(FVector(0.0f, 0.0f, 0.0f));
					GEngine->AddOnScreenDebugMessage(2, 0.5f, FColor::Red, FString::Printf(TEXT("DOWN : %f, OriginDist : %f, AdjustDist : %f"), AdjustSize, GrapplingTargetDistance, Distance));
				}
			}
			else
			{
				Step = EGrappleStep::Finish;
			}
		}
		break;
		case EGrappleStep::Finish:
		{
			CancelGrapplingHook();
		}
		break;
		default:
			break;
		}
	}
}

void URFAbility_GrapplingHook::StartMoveToTarget()
{

}
