// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_UseRecoveryItem.h"
#include "Components/HarmoniaRechargeableItemComponent.h"
#include "Character/LyraHealthComponent.h"
#include "AbilitySystem/HarmoniaAbilitySystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

UHarmoniaGameplayAbility_UseRecoveryItem::UHarmoniaGameplayAbility_UseRecoveryItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Use inherited tag containers - configure these in Blueprint or derived classes:
	// - ActivationOwnedTags: Tags applied while using item (e.g., State.Item.Using)
	// - ActivationBlockedTags: Tags that prevent item use
	// - BlockAbilitiesWithTag: Abilities to block while using item (e.g., Ability.Melee, Ability.Skill)
}

void UHarmoniaGameplayAbility_UseRecoveryItem::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// EventData?�서 ?�이???�??가?�오�?(?�션)
	if (TriggerEventData && TriggerEventData->EventMagnitude > 0)
	{
		ItemType = static_cast<EHarmoniaRecoveryItemType>(TriggerEventData->EventMagnitude);
	}

	StartUsingRecoveryItem();
}

void UHarmoniaGameplayAbility_UseRecoveryItem::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// ?�?�머 ?�리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UsageTimerHandle);
		World->GetTimerManager().ClearTimer(MovementCheckTimerHandle);
	}

	// VFX ?�리
	if (VFXComponent && VFXComponent->IsActive())
	{
		VFXComponent->DeactivateImmediate();
	}

	// Unbind damage delegate
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		Avatar->OnTakeAnyDamage.RemoveDynamic(this, &UHarmoniaGameplayAbility_UseRecoveryItem::OnOwnerDamaged);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UHarmoniaGameplayAbility_UseRecoveryItem::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Rechargeable Item Component ?�인
	UHarmoniaRechargeableItemComponent* ItemComponent = GetRechargeableItemComponent();
	if (!ItemComponent)
	{
		return false;
	}

	// ?�이???�용 가???��? ?�인
	FText Reason;
	return ItemComponent->CanUseRecoveryItem(ItemType, Reason);
}

void UHarmoniaGameplayAbility_UseRecoveryItem::StartUsingRecoveryItem()
{
	UHarmoniaRechargeableItemComponent* ItemComponent = GetRechargeableItemComponent();
	if (!ItemComponent)
	{
		CancelUsingRecoveryItem(FText::FromString(TEXT("Item component not found")));
		return;
	}

	// Get config
	if (bUseConfigOverride)
	{
		CurrentConfig = ConfigOverride;
	}
	else
	{
		if (!ItemComponent->GetRecoveryItemConfig(ItemType, CurrentConfig))
		{
			CancelUsingRecoveryItem(FText::FromString(TEXT("Item config not found")));
			return;
		}
	}

	// Check usage start location (for movement detection)
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		UsageStartLocation = Character->GetActorLocation();
	}

	// VFX ?�생
	PlayUsageEffects();

	// ?�니메이???�생
	if (UsageAnimation)
	{
		ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
		if (Character && Character->GetMesh())
		{
			Character->PlayAnimMontage(UsageAnimation);
		}
	}

	// ?�전 ?�간???�으�??�?�머 ?�정
	if (CurrentConfig.UsageDuration > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				UsageTimerHandle,
				this,
				&UHarmoniaGameplayAbility_UseRecoveryItem::CompleteUsingRecoveryItem,
				CurrentConfig.UsageDuration,
				false
			);
		}

		// ?�동/?�격 감�? ?�정
		if (CurrentConfig.bCancelOnMovement)
		{
			// Set timer to check movement periodically
			if (UWorld* TimerWorld = GetWorld())
			{
				TimerWorld->GetTimerManager().SetTimer(
					MovementCheckTimerHandle,
					this,
					&UHarmoniaGameplayAbility_UseRecoveryItem::OnOwnerMoved,
					0.1f,
					true
				);
			}
		}

		if (CurrentConfig.bCancelOnDamage)
		{
			// Bind to damage event
			if (AActor* Avatar = GetAvatarActorFromActorInfo())
			{
				Avatar->OnTakeAnyDamage.AddDynamic(this, &UHarmoniaGameplayAbility_UseRecoveryItem::OnOwnerDamaged);
			}
		}
	}
	else
	{
		// 즉시 ?�용
		CompleteUsingRecoveryItem();
	}
}

void UHarmoniaGameplayAbility_UseRecoveryItem::CompleteUsingRecoveryItem()
{
	UHarmoniaRechargeableItemComponent* ItemComponent = GetRechargeableItemComponent();
	if (!ItemComponent)
	{
		CancelUsingRecoveryItem(FText::FromString(TEXT("Item component not found")));
		return;
	}

	// ?�이???�용 (충전 ?�수 ?�모)
	bool bUsed = false;
	if (ItemType == EHarmoniaRecoveryItemType::ResonanceShard)
	{
		bUsed = ItemComponent->UseResonanceShard(ShardFrequency);
	}
	else
	{
		bUsed = ItemComponent->UseRecoveryItem(ItemType);
	}

	if (!bUsed)
	{
		CancelUsingRecoveryItem(FText::FromString(TEXT("Failed to use item")));
		return;
	}

	// ?�복 ?�과 ?�용
	ApplyRecoveryEffects();

	// Ability 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHarmoniaGameplayAbility_UseRecoveryItem::CancelUsingRecoveryItem(const FText& Reason)
{
	UE_LOG(LogTemp, Warning, TEXT("Recovery item usage cancelled: %s"), *Reason.ToString());

	// ?�패 ?�운???�생
	if (CurrentConfig.FailureSound)
	{
		if (AActor* Avatar = GetAvatarActorFromActorInfo())
		{
			UGameplayStatics::PlaySoundAtLocation(this, CurrentConfig.FailureSound, Avatar->GetActorLocation());
		}
	}

	// Ability 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UHarmoniaGameplayAbility_UseRecoveryItem::ApplyRecoveryEffects()
{
	switch (CurrentConfig.EffectType)
	{
	case EHarmoniaRecoveryEffectType::Instant:
		{
			float HealthAmount = CurrentConfig.HealthRecoveryAmount;

			// ?�센???�복?�면 최�? 체력 계산
			if (CurrentConfig.HealthRecoveryPercent > 0.0f)
			{
				if (ULyraHealthComponent* HealthComp = GetAvatarActorFromActorInfo()->FindComponentByClass<ULyraHealthComponent>())
				{
					HealthAmount = HealthComp->GetMaxHealth() * CurrentConfig.HealthRecoveryPercent;
				}
			}

			ApplyInstantRecovery(HealthAmount);
		}
		break;

	case EHarmoniaRecoveryEffectType::OverTime:
		{
			float HealthPerSecond = CurrentConfig.HealthRecoveryAmount / CurrentConfig.RecoveryDuration;
			ApplyOverTimeRecovery(HealthPerSecond, CurrentConfig.RecoveryDuration);
		}
		break;

	case EHarmoniaRecoveryEffectType::TimeReversal:
		ApplyTimeReversalRecovery();
		break;

	case EHarmoniaRecoveryEffectType::AreaDeployable:
		DeployRecoveryArea();
		break;

	default:
		break;
	}

	// 추�? Gameplay Effects ?�용
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		for (TSubclassOf<UGameplayEffect> GEClass : CurrentConfig.GameplayEffects)
		{
			if (GEClass)
			{
				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
				EffectContext.AddSourceObject(this);

				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GEClass, 1.0f, EffectContext);
				if (SpecHandle.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
	}

	// 공명 ?�편 주파?�별 추�? ?�과
	if (ItemType == EHarmoniaRecoveryItemType::ResonanceShard)
	{
		UHarmoniaRechargeableItemComponent* ItemComponent = GetRechargeableItemComponent();
		if (ItemComponent)
		{
			FHarmoniaResonanceShardVariant ShardVariant;
			if (ItemComponent->GetResonanceShardVariant(ShardFrequency, ShardVariant))
			{
				if (ShardVariant.AdditionalEffect)
				{
					if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
					{
						FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
						EffectContext.AddSourceObject(this);

						FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ShardVariant.AdditionalEffect, 1.0f, EffectContext);
						if (SpecHandle.IsValid())
						{
							ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
						}
					}
				}
			}
		}
	}
}

void UHarmoniaGameplayAbility_UseRecoveryItem::ApplyInstantRecovery(float HealthAmount)
{
	if (ULyraHealthComponent* HealthComp = GetAvatarActorFromActorInfo()->FindComponentByClass<ULyraHealthComponent>())
	{
		// Apply healing through AbilitySystemComponent if available
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			float CurrentHealth = UHarmoniaAbilitySystemLibrary::GetHealth(ASC);
			float MaxHealth = UHarmoniaAbilitySystemLibrary::GetMaxHealth(ASC);
			float ActualHealing = FMath::Min(HealthAmount, MaxHealth - CurrentHealth);

			// Use library function to restore health
			UHarmoniaAbilitySystemLibrary::RestoreHealth(ASC, HealthAmount);
			UE_LOG(LogTemp, Log, TEXT("Instant recovery: Healed %f HP via AbilitySystem"), ActualHealing);
		}
		else
		{
			// Fallback for actors without ASC (shouldn't happen in this context)
			float CurrentHealth = HealthComp->GetHealth();
			float NewHealth = FMath::Min(CurrentHealth + HealthAmount, HealthComp->GetMaxHealth());
			float ActualHealed = NewHealth - CurrentHealth;
			UE_LOG(LogTemp, Warning, TEXT("Instant recovery: No ASC found, healing amount: %f HP"), ActualHealed);
		}
	}
}

void UHarmoniaGameplayAbility_UseRecoveryItem::ApplyOverTimeRecovery(float HealthPerSecond, float Duration)
{
	// HoT (Heal over Time) ?�과??Gameplay Effect�?구현?�는 것이 좋음
	UE_LOG(LogTemp, Log, TEXT("Over time recovery: %f HP/s for %f seconds"), HealthPerSecond, Duration);
}

void UHarmoniaGameplayAbility_UseRecoveryItem::ApplyTimeReversalRecovery()
{
	// ?�간 ??�� ?�복 (Frozen Time Snowflake ?�용)
	UE_LOG(LogTemp, Log, TEXT("Time reversal recovery applied"));
}

void UHarmoniaGameplayAbility_UseRecoveryItem::DeployRecoveryArea()
{
	// ?�치??범위 ?�복 (Life Luminescence ?�용)
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		FVector SpawnLocation = Avatar->GetActorLocation();
		UE_LOG(LogTemp, Log, TEXT("Recovery area deployed at location: %s"), *SpawnLocation.ToString());
	}
}

void UHarmoniaGameplayAbility_UseRecoveryItem::PlayUsageEffects()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	// VFX ?�생
	if (CurrentConfig.UsageVFX)
	{
		VFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CurrentConfig.UsageVFX,
			Avatar->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	// 공명 ?�편 주파?�별 VFX
	if (ItemType == EHarmoniaRecoveryItemType::ResonanceShard)
	{
		UHarmoniaRechargeableItemComponent* ItemComponent = GetRechargeableItemComponent();
		if (ItemComponent)
		{
			FHarmoniaResonanceShardVariant ShardVariant;
			if (ItemComponent->GetResonanceShardVariant(ShardFrequency, ShardVariant))
			{
				if (ShardVariant.UsageVFX)
				{
					UNiagaraFunctionLibrary::SpawnSystemAttached(
						ShardVariant.UsageVFX,
						Avatar->GetRootComponent(),
						NAME_None,
						FVector::ZeroVector,
						FRotator::ZeroRotator,
						EAttachLocation::SnapToTarget,
						true
					);
				}

				// SFX ?�생
				if (ShardVariant.UsageSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, ShardVariant.UsageSound, Avatar->GetActorLocation());
				}

				return; // ?�편??VFX/SFX�??�용?�으므�?기본 ?�운?�는 ?�생?��? ?�음
			}
		}
	}

	// SFX ?�생
	if (CurrentConfig.UsageSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CurrentConfig.UsageSound, Avatar->GetActorLocation());
	}
}

void UHarmoniaGameplayAbility_UseRecoveryItem::OnOwnerMoved()
{
	if (!CurrentConfig.bCancelOnMovement)
	{
		return;
	}

	// ?�동 거리 ?�인
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		float Distance = FVector::Dist(Character->GetActorLocation(), UsageStartLocation);
		if (Distance > MovementThreshold)
		{
			CancelUsingRecoveryItem(FText::FromString(TEXT("Moved during usage")));
		}
	}
}

void UHarmoniaGameplayAbility_UseRecoveryItem::OnOwnerDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (CurrentConfig.bCancelOnDamage && Damage > 0.0f)
	{
		CancelUsingRecoveryItem(FText::FromString(TEXT("Damaged during usage")));
	}
}

UHarmoniaRechargeableItemComponent* UHarmoniaGameplayAbility_UseRecoveryItem::GetRechargeableItemComponent() const
{
	if (RechargeableItemComponent)
	{
		return RechargeableItemComponent;
	}

	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		return Avatar->FindComponentByClass<UHarmoniaRechargeableItemComponent>();
	}

	return nullptr;
}
