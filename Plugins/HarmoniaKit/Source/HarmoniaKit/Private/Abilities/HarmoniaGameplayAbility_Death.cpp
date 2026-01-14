// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Death.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaGameplayAbility_Death)

UHarmoniaGameplayAbility_Death::UHarmoniaGameplayAbility_Death(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set instancing policy
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// Net execution policy
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// Death ability should block most other abilities
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
	
	// Add death tag during activation
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Death.InProgress")));
	
	// Default respawn ability tag
	RespawnAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Respawn"));
}

void UHarmoniaGameplayAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// Disable player input immediately
	DisablePlayerInput();
	
	// Get character and cache mesh
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	CachedMesh = Character->GetMesh();
	
	// Store original collision settings before any changes
	StoreOriginalCollisionSettings();
	
	// Disable capsule collision so ragdoll can fall
	if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	// Play death montage if configured
	if (DeathMontage)
	{
		// Play montage and bind callbacks
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
			if (Spec)
			{
				// Play the montage
				if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
				{
					float Duration = AnimInstance->Montage_Play(DeathMontage);
					
					if (Duration > 0.0f)
					{
						// Set up montage ended delegate using lambda to match FOnMontageEnded signature
						FOnMontageEnded EndedDelegate;
						EndedDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
						{
							if (bInterrupted)
							{
								OnMontageInterrupted();
							}
							else
							{
								OnMontageCompleted();
							}
						});
						AnimInstance->Montage_SetEndDelegate(EndedDelegate, DeathMontage);
						
						return; // Wait for montage to complete
					}
				}
			}
		}
	}
	
	// No montage or montage failed - go directly to ragdoll
	if (bEnableRagdoll)
	{
		ActivateStableRagdoll();
	}
	else
	{
		StartFadeOut();
	}
}

void UHarmoniaGameplayAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Clear all timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RagdollTimerHandle);
		World->GetTimerManager().ClearTimer(FadeOutTimerHandle);
		World->GetTimerManager().ClearTimer(RespawnTimerHandle);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_Death::OnMontageCompleted()
{
	if (bEnableRagdoll)
	{
		// Delay before ragdoll activation
		if (RagdollDelay > 0.0f)
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					RagdollTimerHandle,
					this,
					&ThisClass::ActivateStableRagdoll,
					RagdollDelay,
					false
				);
			}
		}
		else
		{
			ActivateStableRagdoll();
		}
	}
	else
	{
		StartFadeOut();
	}
}

void UHarmoniaGameplayAbility_Death::OnMontageInterrupted()
{
	// If montage is interrupted, still proceed with death sequence
	OnMontageCompleted();
}

void UHarmoniaGameplayAbility_Death::ActivateStableRagdoll()
{
	if (USkeletalMeshComponent* Mesh = CachedMesh.Get())
	{
		ApplyStableRagdollSettings(Mesh);
	}
	
	// Schedule fade out after ragdoll duration
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FadeOutTimerHandle,
			this,
			&ThisClass::StartFadeOut,
			RagdollDuration,
			false
		);
	}
}

void UHarmoniaGameplayAbility_Death::StartFadeOut()
{
	// Get player controller for camera fade
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
		{
			if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
			{
				CameraManager->StartCameraFade(0.0f, 1.0f, FadeOutDuration, FadeColor, false, true);
			}
		}
	}
	
	// Schedule death sequence completion
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RespawnTimerHandle,
			this,
			&ThisClass::OnDeathSequenceComplete,
			FadeOutDuration + RespawnDelay,
			false
		);
	}
}

void UHarmoniaGameplayAbility_Death::OnDeathSequenceComplete()
{
	if (bAutoTriggerRespawn)
	{
		TriggerRespawnAbility();
	}
	
	// End death ability
	K2_EndAbility();
}

void UHarmoniaGameplayAbility_Death::TriggerRespawnAbility()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC && RespawnAbilityTag.IsValid())
	{
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(RespawnAbilityTag);
		ASC->TryActivateAbilitiesByTag(TagContainer);
	}
}

void UHarmoniaGameplayAbility_Death::DisablePlayerInput()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
		{
			// Disable input
			Character->DisableInput(PC);
		}
	}
}

void UHarmoniaGameplayAbility_Death::StoreOriginalCollisionSettings()
{
	// Future expansion: store collision settings for restoration on respawn
	// Currently handled by respawn ability
}

void UHarmoniaGameplayAbility_Death::ApplyStableRagdollSettings(USkeletalMeshComponent* Mesh)
{
	if (!Mesh)
	{
		return;
	}
	
	// Enable physics simulation
	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	
	// CRITICAL: Make ragdoll stable - ignore all impulses/hits
	// This prevents the body from flying away when hit during death
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Destructible, ECR_Ignore);
	
	// Only collide with world static/dynamic for ground contact
	Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	
	// Ensure gravity is enabled
	Mesh->SetEnableGravity(true);
	
	// Stop any ongoing animation
	if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.0f);
	}
	
	// Set physics blend weight to full physics
	Mesh->SetAllBodiesPhysicsBlendWeight(1.0f);
}
