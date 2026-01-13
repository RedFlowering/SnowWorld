// Copyright 2025 Snow Game Studio.

#include "Animation/AnimNotifyState_ParryableWindow.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "HarmoniaLogCategories.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "HarmoniaGameplayTags.h"

UAnimNotifyState_ParryableWindow::UAnimNotifyState_ParryableWindow()
{
	// BranchingPoint로 설정하여 정확한 타이밍 보장
	bIsNativeBranchingPoint = true;
}

void UAnimNotifyState_ParryableWindow::BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyBegin(BranchingPointPayload);

	if (bIsUnparryable)
	{
		UE_LOG(LogHarmoniaCombat, Verbose, TEXT("[Parry] NotifyBegin skipped - Attack is unparryable"));
		return;
	}

	USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent;
	if (!MeshComp)
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("[Parry] NotifyBegin failed - No SkelMeshComponent"));
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		UE_LOG(LogHarmoniaCombat, Warning, TEXT("[Parry] NotifyBegin failed - No Owner actor"));
		return;
	}

	// 현재 몽타주 타임 기록 (히트 시점 검증용)
	RegisteredStartTime = 0.0f;
	if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
	{
		if (UAnimMontage* CurrentMontage = AnimInst->GetCurrentActiveMontage())
		{
			RegisteredStartTime = AnimInst->Montage_GetPosition(CurrentMontage);
		}
	}

	CachedOwner = Owner;

	// 공격자에게 Parryable 태그 부여 (ASC 사용)
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner))
	{
		ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Combat_Parryable);
	}

	// 공격자의 MeleeCombatComponent에 ParryConfig 저장
	if (UHarmoniaMeleeCombatComponent* MeleeComp = Owner->FindComponentByClass<UHarmoniaMeleeCombatComponent>())
	{
		MeleeComp->SetCurrentParryConfig(ParryConfig);
		
		// Reset hit tracking for new attack phase (allows each combo hit to be parried)
		MeleeComp->ResetHitTracking();
	}
		
	UE_LOG(LogHarmoniaCombat, Log, TEXT("[Parry] Window opened - Attacker: %s, Parryable tag added, HitTracking reset"), 
		*GetNameSafe(Owner));
}

void UAnimNotifyState_ParryableWindow::BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	Super::BranchingPointNotifyEnd(BranchingPointPayload);

	if (bIsUnparryable)
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent;
	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	// 공격자의 Parryable 태그 제거
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner))
	{
		ASC->RemoveLooseGameplayTag(HarmoniaGameplayTags::State_Combat_Parryable);
		
		UE_LOG(LogHarmoniaCombat, Log, TEXT("[Parry] Window closed - Attacker: %s, Parryable tag removed"), 
			*GetNameSafe(Owner));
	}

	CachedOwner = nullptr;
}

FString UAnimNotifyState_ParryableWindow::GetNotifyName_Implementation() const
{
	return bIsUnparryable ? TEXT("Parryable Window (DISABLED)") : TEXT("Parryable Window");
}
