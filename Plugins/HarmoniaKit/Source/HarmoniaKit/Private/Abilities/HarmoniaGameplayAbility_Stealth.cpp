// Copyright Epic Games, Inc. All Rights Reserved.

#include "Abilities/HarmoniaGameplayAbility_Stealth.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "Monsters/HarmoniaMonsterBase.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UHarmoniaGameplayAbility_Stealth::UHarmoniaGameplayAbility_Stealth()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	StealthDuration = 0.0f;
	StealthCooldown = 10.0f;
	StealthOpacity = 0.2f;
	AmbushDamageMultiplier = 2.0f;
	StealthMovementSpeedMultiplier = 1.3f;
	DetectionRangeMultiplier = 0.3f;
	bIsStealthed = false;
}

void UHarmoniaGameplayAbility_Stealth::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	EnterStealth();

	// If duration is set, end after duration
	if (StealthDuration > 0.0f)
	{
		FTimerHandle DurationTimer;
		GetWorld()->GetTimerManager().SetTimer(DurationTimer, [this, Handle, ActorInfo, ActivationInfo]()
		{
			BreakStealth();
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		}, StealthDuration, false);
	}
}

void UHarmoniaGameplayAbility_Stealth::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bIsStealthed)
	{
		BreakStealth();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHarmoniaGameplayAbility_Stealth::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	// Break stealth on input release (manual break)
	if (bIsStealthed)
	{
		BreakStealth();
	}

	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
}

void UHarmoniaGameplayAbility_Stealth::EnterStealth()
{
	if (bIsStealthed)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	// Apply stealth effect
	if (StealthEffect)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StealthEffect, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			StealthEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	// Apply ambush bonus (for next attack)
	ApplyAmbushBonus();

	// Apply visual effects
	ApplyStealthVisuals();

	// Bind to damage event to break stealth when hit
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (Avatar)
	{
		Avatar->OnTakeAnyDamage.AddDynamic(this, &UHarmoniaGameplayAbility_Stealth::OnOwnerDamaged);
	}

	bIsStealthed = true;

	UE_LOG(LogTemp, Log, TEXT("Monster entered stealth mode"));
}

void UHarmoniaGameplayAbility_Stealth::BreakStealth()
{
	if (!bIsStealthed)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		// Remove stealth effect
		if (StealthEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(StealthEffectHandle);
			StealthEffectHandle.Invalidate();
		}
	}

	// Remove visual effects
	RemoveStealthVisuals();

	// Unbind damage event
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (Avatar)
	{
		Avatar->OnTakeAnyDamage.RemoveDynamic(this, &UHarmoniaGameplayAbility_Stealth::OnOwnerDamaged);
	}

	bIsStealthed = false;

	UE_LOG(LogTemp, Log, TEXT("Monster stealth broken"));
}

void UHarmoniaGameplayAbility_Stealth::ApplyStealthVisuals()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(Avatar);
	if (!Character)
	{
		return;
	}

	// Get all mesh components
	TArray<UMeshComponent*> MeshComponents;
	Character->GetComponents<UMeshComponent>(MeshComponents);

	OriginalOpacities.Empty();

	for (UMeshComponent* MeshComp : MeshComponents)
	{
		if (!MeshComp)
		{
			continue;
		}

		// Store original opacity
		OriginalOpacities.Add(MeshComp, 1.0f);

		// Get all materials
		int32 NumMaterials = MeshComp->GetNumMaterials();
		for (int32 i = 0; i < NumMaterials; ++i)
		{
			UMaterialInterface* Material = MeshComp->GetMaterial(i);
			if (Material)
			{
				// Create dynamic material instance if needed
				UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(Material);
				if (!DynMat)
				{
					DynMat = MeshComp->CreateAndSetMaterialInstanceDynamic(i);
				}

				if (DynMat)
				{
					// Set opacity parameter (assumes material has "Opacity" parameter)
					DynMat->SetScalarParameterValue(FName("Opacity"), StealthOpacity);
				}
			}
		}
	}

	// Could also set actor to be partially hidden or use a different rendering mode
	// Character->SetActorHiddenInGame(false); // Still visible but translucent
}

void UHarmoniaGameplayAbility_Stealth::RemoveStealthVisuals()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(Avatar);
	if (!Character)
	{
		return;
	}

	// Restore all mesh components to original opacity
	for (const auto& Pair : OriginalOpacities)
	{
		UMeshComponent* MeshComp = Pair.Key;
		if (MeshComp)
		{
			int32 NumMaterials = MeshComp->GetNumMaterials();
			for (int32 i = 0; i < NumMaterials; ++i)
			{
				UMaterialInterface* Material = MeshComp->GetMaterial(i);
				UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(Material);
				if (DynMat)
				{
					DynMat->SetScalarParameterValue(FName("Opacity"), 1.0f);
				}
			}
		}
	}

	OriginalOpacities.Empty();
}

void UHarmoniaGameplayAbility_Stealth::ApplyAmbushBonus()
{
	if (!AmbushEffect)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AmbushEffect, AmbushDamageMultiplier, EffectContext);
	if (SpecHandle.IsValid())
	{
		// Apply ambush effect that will last for one attack
		AmbushEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}

	UE_LOG(LogTemp, Log, TEXT("Ambush bonus applied (%.1fx damage on next attack)"), AmbushDamageMultiplier);
}

void UHarmoniaGameplayAbility_Stealth::OnOwnerDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	// Taking damage breaks stealth
	if (bIsStealthed && Damage > 0.0f)
	{
		BreakStealth();

		// End the ability
		if (IsActive())
		{
			CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
		}
	}
}
