// Copyright 2025 Snow Game Studio.

#include "Abilities/HarmoniaGameplayAbility_Mantle.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Core/HarmoniaCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HarmoniaGameplayAbility_Mantle)

UHarmoniaGameplayAbility_Mantle::UHarmoniaGameplayAbility_Mantle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UHarmoniaGameplayAbility_Mantle::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AHarmoniaCharacter* HarmoniaCharacter = GetHarmoniaCharacter();
	if (!HarmoniaCharacter)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	HarmoniaCharacter->OnMantlingEndedDelegate.AddDynamic(this, &UHarmoniaGameplayAbility_Mantle::OnMantlingEnded);
	StartMantleCheckTask();
}

void UHarmoniaGameplayAbility_Mantle::StartMantleCheckTask()
{
	if (bIsMantling)
	{
		return;
	}

	UAbilityTask_WaitDelay* WaitTask = UAbilityTask_WaitDelay::WaitDelay(this, MantleCheckInterval);
	if (WaitTask)
	{
		WaitTask->OnFinish.AddDynamic(this, &UHarmoniaGameplayAbility_Mantle::OnMantleCheckTimer);
		WaitTask->ReadyForActivation();
	}
}

void UHarmoniaGameplayAbility_Mantle::OnMantleCheckTimer()
{
	if (bIsMantling)
	{
		return;
	}

	AHarmoniaCharacter* HarmoniaCharacter = GetHarmoniaCharacter();
	if (HarmoniaCharacter && HarmoniaCharacter->TryMantleFromGA())
	{
		bIsMantling = true;
		
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			// Add State.Mantling tag FIRST (before cancelling abilities)
			// This prevents cancelled abilities from immediately reactivating due to held input
			ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Mantling")));
			
			// Cancel Sprint GA first (before Jump cancellation removes State.InAir tag)
			if (SprintAbilityClass)
			{
				FGameplayAbilitySpec* SprintSpec = ASC->FindAbilitySpecFromClass(SprintAbilityClass);
				if (SprintSpec && SprintSpec->IsActive())
				{
					TArray<UGameplayAbility*> Instances = SprintSpec->GetAbilityInstances();
					for (UGameplayAbility* Instance : Instances)
					{
						if (Instance && Instance->IsActive())
						{
							Instance->CancelAbility(SprintSpec->Handle, Instance->GetCurrentActorInfo(), Instance->GetCurrentActivationInfo(), true);
						}
					}
				}
			}
			
			// Cancel Block GA
			if (BlockAbilityClass)
			{
				FGameplayAbilitySpec* BlockSpec = ASC->FindAbilitySpecFromClass(BlockAbilityClass);
				UE_LOG(LogTemp, Warning, TEXT("[Mantle] Trying to cancel Block - BlockSpec: %s, IsActive: %d"), 
					BlockSpec ? TEXT("Found") : TEXT("Not Found"), 
					BlockSpec ? BlockSpec->IsActive() : false);
				if (BlockSpec && BlockSpec->IsActive())
				{
					TArray<UGameplayAbility*> Instances = BlockSpec->GetAbilityInstances();
					for (UGameplayAbility* Instance : Instances)
					{
						if (Instance && Instance->IsActive())
						{
							UE_LOG(LogTemp, Warning, TEXT("[Mantle] Cancelling Block ability instance"));
							Instance->CancelAbility(BlockSpec->Handle, Instance->GetCurrentActorInfo(), Instance->GetCurrentActivationInfo(), true);
						}
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[Mantle] BlockAbilityClass is NOT SET!"));
			}
			
			// Cancel Jump GA by class
			if (JumpAbilityClass)
			{
				FGameplayAbilitySpec* JumpSpec = ASC->FindAbilitySpecFromClass(JumpAbilityClass);
				if (JumpSpec && JumpSpec->IsActive())
				{
					TArray<UGameplayAbility*> Instances = JumpSpec->GetAbilityInstances();
					for (UGameplayAbility* Instance : Instances)
					{
						if (Instance && Instance->IsActive())
						{
							Instance->CancelAbility(JumpSpec->Handle, Instance->GetCurrentActorInfo(), Instance->GetCurrentActivationInfo(), true);
						}
					}
				}
			}
			
			// Apply mantle cost effect
			if (CostGameplayEffectClass)
			{
				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
				EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());
				
				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CostGameplayEffectClass, GetAbilityLevel(), EffectContext);
				if (SpecHandle.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
	}
	else
	{
		StartMantleCheckTask();
	}
}

void UHarmoniaGameplayAbility_Mantle::OnMantlingEnded()
{
	bIsMantling = false;
	
	// Remove State.Mantling tag
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("State.Mantling")));
	}
	
	StartMantleCheckTask();
}

void UHarmoniaGameplayAbility_Mantle::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AHarmoniaCharacter* HarmoniaCharacter = Cast<AHarmoniaCharacter>(ActorInfo->AvatarActor.Get()))
	{
		HarmoniaCharacter->OnMantlingEndedDelegate.RemoveDynamic(this, &UHarmoniaGameplayAbility_Mantle::OnMantlingEnded);
	}
	
	bIsMantling = false;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

AHarmoniaCharacter* UHarmoniaGameplayAbility_Mantle::GetHarmoniaCharacter() const
{
	return Cast<AHarmoniaCharacter>(GetAvatarActorFromActorInfo());
}
