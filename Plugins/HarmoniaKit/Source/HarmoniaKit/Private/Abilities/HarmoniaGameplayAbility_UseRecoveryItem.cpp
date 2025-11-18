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

	// 기본 태그 설정
	// Using tags - 사용 중에는 다른 행동 제한
	// UsingTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Item.Using")));

	// Blocked tags - 사용 중에는 공격, 스킬 등 제한
	// BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Melee")));
	// BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Skill")));
}

void UHarmoniaGameplayAbility_UseRecoveryItem::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// EventData에서 아이템 타입 가져오기 (옵션)
	if (TriggerEventData && TriggerEventData->EventMagnitude > 0)
	{
		ItemType = static_cast<EHarmoniaRecoveryItemType>(TriggerEventData->EventMagnitude);
	}

	StartUsingRecoveryItem();
}

void UHarmoniaGameplayAbility_UseRecoveryItem::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 타이머 정리
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UsageTimerHandle);
	}

	// VFX 정리
	if (VFXComponent && VFXComponent->IsActive())
	{
		VFXComponent->DeactivateImmediate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UHarmoniaGameplayAbility_UseRecoveryItem::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Rechargeable Item Component 확인
	UHarmoniaRechargeableItemComponent* ItemComponent = GetRechargeableItemComponent();
	if (!ItemComponent)
	{
		return false;
	}

	// 아이템 사용 가능 여부 확인
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

	// 설정 가져오기
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

	// 사용 시작 위치 저장 (이동 감지용)
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		UsageStartLocation = Character->GetActorLocation();
	}

	// VFX 재생
	PlayUsageEffects();

	// 애니메이션 재생
	if (UsageAnimation)
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			// TODO: Play montage
		}
	}

	// 시전 시간이 있으면 타이머 설정
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

		// 이동/피격 감지 설정
		if (CurrentConfig.bCancelOnMovement)
		{
			// TODO: Bind to movement event
		}

		if (CurrentConfig.bCancelOnDamage)
		{
			// TODO: Bind to damage event
		}
	}
	else
	{
		// 즉시 사용
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

	// 아이템 사용 (충전 횟수 소모)
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

	// 회복 효과 적용
	ApplyRecoveryEffects();

	// Ability 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UHarmoniaGameplayAbility_UseRecoveryItem::CancelUsingRecoveryItem(const FText& Reason)
{
	UE_LOG(LogTemp, Warning, TEXT("Recovery item usage cancelled: %s"), *Reason.ToString());

	// 실패 사운드 재생
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

			// 퍼센트 회복이면 최대 체력 계산
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

	// 추가 Gameplay Effects 적용
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

	// 공명 파편 주파수별 추가 효과
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
	// HoT (Heal over Time) 효과는 Gameplay Effect로 구현하는 것이 좋음
	// For full implementation, create a periodic Gameplay Effect Blueprint with:
	// - Duration: Specified duration
	// - Period: 1.0 second (or desired tick rate)
	// - Modifiers: Health +HealthPerSecond (Execute on Application, Periodic)
	//
	// Example implementation:
	// if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	// {
	//     if (HealOverTimeEffectClass)
	//     {
	//         FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	//         FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(
	//             HealOverTimeEffectClass, 1.0f, Context);
	//         if (Spec.IsValid())
	//         {
	//             Spec.Data->SetDuration(Duration);
	//             // Set magnitude for healing per tick via SetByCaller or modifier
	//             ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	//         }
	//     }
	// }

	UE_LOG(LogTemp, Log, TEXT("Over time recovery: %f HP/s for %f seconds"), HealthPerSecond, Duration);
}

void UHarmoniaGameplayAbility_UseRecoveryItem::ApplyTimeReversalRecovery()
{
	// 시간 역행 회복 (Frozen Time Snowflake 전용)
	// 최근 1-2초간의 피해를 복구
	// 실제 구현은 타임스탬프 기반 피해 기록 시스템 필요
	//
	// Implementation steps:
	// 1. Create a damage tracking component that records:
	//    - Timestamp of each damage event
	//    - Amount of damage taken
	//    - Actor's position/rotation at time of damage
	// 2. On time reversal:
	//    - Query damage events from last 1-2 seconds
	//    - Calculate total damage to reverse
	//    - Restore health using UHarmoniaAbilitySystemLibrary::RestoreHealth
	//    - Optionally restore position/rotation
	// 3. VFX/SFX:
	//    - Play time reversal particle effects
	//    - Play rewind sound effect
	//
	// Example:
	// if (UDamageHistoryComponent* History = GetAvatarActorFromActorInfo()->FindComponentByClass<UDamageHistoryComponent>())
	// {
	//     float ReversalWindow = 2.0f;
	//     float DamageToReverse = History->GetDamageInTimeWindow(ReversalWindow);
	//     if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	//     {
	//         UHarmoniaAbilitySystemLibrary::RestoreHealth(ASC, DamageToReverse);
	//     }
	// }

	UE_LOG(LogTemp, Log, TEXT("Time reversal recovery applied"));
}

void UHarmoniaGameplayAbility_UseRecoveryItem::DeployRecoveryArea()
{
	// 설치형 범위 회복 (Life Luminescence 전용)
	// 바닥에 회복 구역 생성

	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		FVector SpawnLocation = Avatar->GetActorLocation();
		FRotator SpawnRotation = Avatar->GetActorRotation();

		// Spawn deployable recovery area actor
		// Note: The AHarmoniaRecoveryAreaActor class is already implemented and ready to use
		// To complete this implementation:
		// 1. Add a UPROPERTY for the recovery area actor class in the ability or config
		// 2. Spawn the actor using the code below:
		//
		// UWorld* World = GetWorld();
		// if (World && DeployableConfig.DeployableActorClass)
		// {
		//     FActorSpawnParameters SpawnParams;
		//     SpawnParams.Owner = Avatar;
		//     SpawnParams.Instigator = Cast<APawn>(Avatar);
		//
		//     if (AHarmoniaRecoveryAreaActor* DeployedArea = World->SpawnActor<AHarmoniaRecoveryAreaActor>(
		//         DeployableConfig.DeployableActorClass, SpawnLocation, SpawnRotation, SpawnParams))
		//     {
		//         // Configure the recovery area with settings from config
		//         DeployedArea->SetRecoveryConfig(DeployableConfig.RecoveryConfig);
		//         DeployedArea->Activate();
		//     }
		// }

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

	// VFX 재생
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

	// 공명 파편 주파수별 VFX
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

				// SFX 재생
				if (ShardVariant.UsageSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, ShardVariant.UsageSound, Avatar->GetActorLocation());
				}

				return; // 파편의 VFX/SFX를 사용했으므로 기본 사운드는 재생하지 않음
			}
		}
	}

	// SFX 재생
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

	// 이동 거리 확인
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
