// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Respawn.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/HarmoniaDeathPenaltyComponent.h"
#include "System/HarmoniaCheckpointSubsystem.h"
#include "Actors/HarmoniaCrystalResonator.h"
#include "HarmoniaLogCategories.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaGameplayAbility_Respawn)

UHarmoniaGameplayAbility_Respawn::UHarmoniaGameplayAbility_Respawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set instancing policy
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// Net execution policy
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// Block activation if not dead
	ActivationRequiredTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
	
	// Clear death tags on activation
	// Note: These will be removed when ability ends
}

void UHarmoniaGameplayAbility_Respawn::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// Execute respawn sequence
	// 1. Teleport to checkpoint
	TeleportToCheckpoint();
	
	// 2. Reset character state (physics, collision)
	ResetCharacterState();
	
	// 3. Remove death tags BEFORE restoring stats (so healing isn't blocked)
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Death.InProgress")));
	}
	
	// 4. Restore stats
	RestorePlayerStats();
	
	// 4. Apply penalties (if enabled)
	if (bApplyDeathPenalties)
	{
		ApplyDeathPenalties();
	}
	
	// 5. Start fade in
	StartFadeIn();
}

void UHarmoniaGameplayAbility_Respawn::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Clear timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FadeInTimerHandle);
		World->GetTimerManager().ClearTimer(InputEnableTimerHandle);
	}
	
	// Remove death tags
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Death.InProgress")));
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_Respawn::TeleportToCheckpoint()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		return;
	}
	
	FVector RespawnLocation = FallbackRespawnLocation;
	FRotator RespawnRotation = FRotator::ZeroRotator;
	
	// Try to get checkpoint location
	if (bUseCheckpointSystem)
	{
		GetCheckpointLocation(RespawnLocation, RespawnRotation);
	}
	
	// Teleport character
	Character->SetActorLocationAndRotation(RespawnLocation, RespawnRotation, false, nullptr, ETeleportType::ResetPhysics);
}

void UHarmoniaGameplayAbility_Respawn::RestorePlayerStats()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("GA_Respawn: No ASC found"));
		return;
	}
	
	// Method 1: Use GameplayEffect if assigned (preferred)
	if (RestoreStatsEffect)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());
		
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(RestoreStatsEffect, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
		return;
	}
	
	// Method 2: Use Healing meta attribute (Lyra-compatible fallback)
	// LyraHealthSet.Health has HideFromModifiers, so we must use Healing attribute
	// PostGameplayEffectExecute in LyraHealthSet will convert Healing to +Health
	const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
	if (!AttributeSet)
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("GA_Respawn: No HarmoniaAttributeSet found"));
		return;
	}
	
	// Restore health using the helper function (uses correct SetBaseValue + SetCurrentValue pattern)
	if (bRestoreHealthToMax)
	{
		UHarmoniaAttributeSet* MutableAttributeSet = const_cast<UHarmoniaAttributeSet*>(AttributeSet);
		MutableAttributeSet->RestoreHealthToMax();
	}
	
	// Restore stamina using the helper function
	if (bRestoreStaminaToMax)
	{
		UHarmoniaAttributeSet* MutableAttributeSet = const_cast<UHarmoniaAttributeSet*>(AttributeSet);
		MutableAttributeSet->RestoreStaminaToMax();
	}
	
	// Restore mana
	if (bRestoreManaToMax)
	{
		UHarmoniaAttributeSet* MutableAttributeSet = const_cast<UHarmoniaAttributeSet*>(AttributeSet);
		MutableAttributeSet->RestoreManaToMax();
	}
	
	// Restore poise
	if (bRestorePoiseToMax)
	{
		UHarmoniaAttributeSet* MutableAttributeSet = const_cast<UHarmoniaAttributeSet*>(AttributeSet);
		MutableAttributeSet->RestorePoiseToMax();
	}
}

void UHarmoniaGameplayAbility_Respawn::ApplyDeathPenalties()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return;
	}
	
	// Get death penalty component
	UHarmoniaDeathPenaltyComponent* DeathPenaltyComp = AvatarActor->FindComponentByClass<UHarmoniaDeathPenaltyComponent>();
	if (DeathPenaltyComp)
	{
		// Death location is already handled by death ability
		// OnPlayerDeath should have been called during death
		// Here we just ensure ethereal state is applied
		// DeathPenaltyComp handles this internally
	}
}

void UHarmoniaGameplayAbility_Respawn::ResetCharacterState()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		return;
	}
	
	// Re-enable capsule collision
	if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	
	// Reset mesh to animated state (disable ragdoll)
	if (USkeletalMeshComponent* Mesh = Character->GetMesh())
	{
		Mesh->SetSimulatePhysics(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetAllBodiesPhysicsBlendWeight(0.0f);
		
		// Reset collision responses
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
		Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		
		// Attach mesh back to capsule (in case it was detached)
		Mesh->AttachToComponent(Character->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

void UHarmoniaGameplayAbility_Respawn::StartFadeIn()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
		{
			if (APlayerCameraManager* CameraManager = PC->PlayerCameraManager)
			{
				// Fade from black to clear
				CameraManager->StartCameraFade(1.0f, 0.0f, FadeInDuration, FLinearColor::Black, false, false);
			}
		}
	}
	
	// Schedule input enable after fade in
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			InputEnableTimerHandle,
			this,
			&ThisClass::EnablePlayerInput,
			FadeInDuration + InputEnableDelay,
			false
		);
	}
}

void UHarmoniaGameplayAbility_Respawn::EnablePlayerInput()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character)
	{
		if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
		{
			Character->EnableInput(PC);
		}
	}
	
	OnRespawnComplete();
}

void UHarmoniaGameplayAbility_Respawn::OnRespawnComplete()
{
	// End the ability
	K2_EndAbility();
}

bool UHarmoniaGameplayAbility_Respawn::GetCheckpointLocation(FVector& OutLocation, FRotator& OutRotation) const
{
	// Get checkpoint subsystem
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	if (!GameInstance)
	{
		return false;
	}
	
	UHarmoniaCheckpointSubsystem* CheckpointSubsystem = GameInstance->GetSubsystem<UHarmoniaCheckpointSubsystem>();
	if (!CheckpointSubsystem)
	{
		return false;
	}
	
	// Get last activated checkpoint for this player
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		return false;
	}
	
	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC)
	{
		return false;
	}
	
	// Get respawn checkpoint (last checkpoint the player visited)
	FName LastCheckpointID = CheckpointSubsystem->GetPlayerLastCheckpoint(PC);
	if (LastCheckpointID.IsNone())
	{
		// Try to get any activated checkpoint
		TArray<AHarmoniaCrystalResonator*> ActiveCheckpoints = CheckpointSubsystem->GetActivatedCheckpoints();
		if (ActiveCheckpoints.Num() > 0 && ActiveCheckpoints[0])
		{
			OutLocation = ActiveCheckpoints[0]->GetActorLocation();
			OutRotation = ActiveCheckpoints[0]->GetActorRotation();
			return true;
		}
		return false;
	}
	
	// Find checkpoint by ID
	AHarmoniaCrystalResonator* Checkpoint = CheckpointSubsystem->FindCheckpoint(LastCheckpointID);
	if (Checkpoint)
	{
		OutLocation = Checkpoint->GetActorLocation();
		OutRotation = Checkpoint->GetActorRotation();
		return true;
	}
	
	return false;
}
