// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Block.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/HarmoniaEquipmentComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "NiagaraFunctionLibrary.h"
#include "System/HarmoniaSoundCacheSubsystem.h"

UHarmoniaGameplayAbility_Block::UHarmoniaGameplayAbility_Block(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UHarmoniaGameplayAbility_Block::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (UHarmoniaMeleeCombatComponent* MeleeComp = GetMeleeCombatComponent())
	{
		return MeleeComp->CanBlock();
	}

	return false;
}

void UHarmoniaGameplayAbility_Block::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	MeleeCombatComponent = GetMeleeCombatComponent();
	if (!MeleeCombatComponent)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Check minimum stamina requirement (like Sprint)
	const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>();
	if (AttributeSet && AttributeSet->GetStamina() < MinStaminaToActivate)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Block] ActivateAbility - Not enough stamina (%.1f < %.1f)"), 
			AttributeSet->GetStamina(), MinStaminaToActivate);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Set defense state
	MeleeCombatComponent->SetDefenseState(EHarmoniaDefenseState::Blocking);
	MeleeCombatComponent->OnBlockedAttack.AddDynamic(this, &UHarmoniaGameplayAbility_Block::OnBlockHit);

	// Apply hold stamina cost effect (Duration GE)
	if (CostGameplayEffectClass)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CostGameplayEffectClass, GetAbilityLevel(), EffectContext);
		if (SpecHandle.IsValid())
		{
			BlockHoldCostEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// Bind to stamina depletion event
	if (AttributeSet)
	{
		UHarmoniaAttributeSet* MutableAttributeSet = const_cast<UHarmoniaAttributeSet*>(AttributeSet);
		MutableAttributeSet->OnOutOfStamina.AddUObject(this, &UHarmoniaGameplayAbility_Block::OnOutOfStamina);
	}

	// Play block animation if available
	if (BlockMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			BlockMontage,
			1.0f,
			BlockStartSectionName,
			false,
			1.0f
		);

		if (MontageTask)
		{
			MontageTask->OnCancelled.AddDynamic(this, &UHarmoniaGameplayAbility_Block::OnMontageInterrupted);
			MontageTask->OnInterrupted.AddDynamic(this, &UHarmoniaGameplayAbility_Block::OnMontageInterrupted);
			MontageTask->ReadyForActivation();
		}

		// Set up section flow: Start -> Loop
		if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
			{
				AnimInstance->Montage_SetNextSection(BlockStartSectionName, BlockLoopSectionName, BlockMontage);
			}
		}
	}
}

void UHarmoniaGameplayAbility_Block::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UHarmoniaGameplayAbility_Block::OnMontageInterrupted()
{
	// If montage is interrupted externally (e.g., by stagger), end the ability
	if (IsActive())
	{
		K2_EndAbility();
	}
}

void UHarmoniaGameplayAbility_Block::OnBlockHit(AActor* Attacker, float IncomingDamage, FVector ImpactPoint)
{
	if (!IsActive())
	{
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	// Apply stamina cost GE when successfully blocking an attack
	if (BlockHitStaminaCostEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(BlockHitStaminaCostEffectClass, GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle);
		}
	}

	// === 1. VFX at weapon socket closest to impact ===
	if (BlockVFX)
	{
		FVector VFXLocation = ImpactPoint; // Default to impact point
		FRotator VFXRotation = FRotator::ZeroRotator;

		// Try to find closest weapon socket for more realistic spark placement
		if (UHarmoniaEquipmentComponent* EquipComp = Avatar->FindComponentByClass<UHarmoniaEquipmentComponent>())
		{
			if (USkeletalMeshComponent* WeaponMesh = EquipComp->GetEquippedMeshComponent(EEquipmentSlot::MainHand))
			{
				TArray<FName> SocketNames = WeaponMesh->GetAllSocketNames();
				float ClosestDistSq = FLT_MAX;
				FName ClosestSocket = NAME_None;

				for (const FName& SocketName : SocketNames)
				{
					FVector SocketLocation = WeaponMesh->GetSocketLocation(SocketName);
					float DistSq = FVector::DistSquared(SocketLocation, ImpactPoint);
					if (DistSq < ClosestDistSq)
					{
						ClosestDistSq = DistSq;
						ClosestSocket = SocketName;
					}
				}

				if (ClosestSocket != NAME_None)
				{
					VFXLocation = WeaponMesh->GetSocketLocation(ClosestSocket);
					VFXRotation = WeaponMesh->GetSocketRotation(ClosestSocket);
				}
			}
		}

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			BlockVFX,
			VFXLocation,
			VFXRotation,
			BlockVFXScale,
			true,
			true,
			ENCPoolMethod::AutoRelease
		);
	}

	// === 2. Sound from equipped weapon ===
	if (UHarmoniaEquipmentComponent* EquipComp = Avatar->FindComponentByClass<UHarmoniaEquipmentComponent>())
	{
		FGameplayTag BlockSoundTag = EquipComp->GetMainHandCombatSoundTag(ECombatSoundType::Block);
		if (BlockSoundTag.IsValid())
		{
			if (UHarmoniaSoundCacheSubsystem* SoundSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UHarmoniaSoundCacheSubsystem>())
			{
				SoundSubsystem->PlaySoundAtLocationByTag(Avatar, BlockSoundTag, ImpactPoint);
			}
		}
	}

	// === 3. Camera shake ===
	if (BlockCameraShakeClass)
	{
		if (APawn* OwnerPawn = Cast<APawn>(Avatar))
		{
			if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
			{
				PC->ClientStartCameraShake(BlockCameraShakeClass);
			}
		}
	}
}

void UHarmoniaGameplayAbility_Block::OnOutOfStamina(AActor* Instigator, AActor* Causer, const FGameplayEffectSpec* EffectSpec, float Magnitude, float OldValue, float NewValue)
{
	// EndAbility handles the montage transition
	K2_EndAbility();
}

void UHarmoniaGameplayAbility_Block::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// Transition to End section if montage is playing
	if (BlockMontage && ActorInfo)
	{
		if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
		{
			if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
			{
				if (AnimInstance->Montage_IsPlaying(BlockMontage))
				{
					AnimInstance->Montage_JumpToSection(BlockEndSectionName, BlockMontage);
				}
			}
		}
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	// Remove hold stamina cost effect
	if (ASC && BlockHoldCostEffectHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(BlockHoldCostEffectHandle);
		BlockHoldCostEffectHandle.Invalidate();
	}

	// Unbind from stamina depletion event
	if (ASC)
	{
		if (const UHarmoniaAttributeSet* AttributeSet = ASC->GetSet<UHarmoniaAttributeSet>())
		{
			UHarmoniaAttributeSet* MutableAttributeSet = const_cast<UHarmoniaAttributeSet*>(AttributeSet);
			MutableAttributeSet->OnOutOfStamina.RemoveAll(this);
		}
	}

	// Reset defense state and unsubscribe from delegate
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->OnBlockedAttack.RemoveDynamic(this, &UHarmoniaGameplayAbility_Block::OnBlockHit);
		MeleeCombatComponent->SetDefenseState(EHarmoniaDefenseState::None);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UHarmoniaMeleeCombatComponent* UHarmoniaGameplayAbility_Block::GetMeleeCombatComponent() const
{
	if (MeleeCombatComponent)
	{
		return MeleeCombatComponent;
	}

	// In Lyra, OwnerActor is PlayerState but components are on the Avatar (Character/Pawn)
	if (const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo())
	{
		if (AActor* Avatar = ActorInfo->AvatarActor.Get())
		{
			return Avatar->FindComponentByClass<UHarmoniaMeleeCombatComponent>();
		}
	}

	return nullptr;
}
