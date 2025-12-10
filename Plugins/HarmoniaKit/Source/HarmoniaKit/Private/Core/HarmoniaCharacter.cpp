// Copyright 2025 Snow Game Studio.

#include "Core/HarmoniaCharacter.h"
#include "Core/HarmoniaCharacterMovementComponent.h"
#include "Core/HarmoniaLockOnTargetingComponent.h"
#include "Core/HarmoniaHealthComponent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Settings/AlsCharacterSettings.h"
#include "Utility/AlsConstants.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Status_Aiming, "Status.Aiming");

AHarmoniaCharacter::AHarmoniaCharacter(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer
		.SetDefaultSubobjectClass<UHarmoniaCharacterMovementComponent>(ALyraCharacter::CharacterMovementComponentName)
		.SetDefaultSubobjectClass<UHarmoniaHealthComponent>(TEXT("HealthComponent")))
{
	HarmoniaCharacterMovement = Cast<UHarmoniaCharacterMovementComponent>(GetCharacterMovement());
	LockOnComponent = CreateDefaultSubobject<UHarmoniaLockOnTargetingComponent>(TEXT("LockOnComponent"));
	HarmoniaHealthComponent = Cast<UHarmoniaHealthComponent>(GetHealthComponent());
}

void AHarmoniaCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetAbilitySystemComponent())
	{
		GetAbilitySystemComponent()->RegisterGameplayTagEvent(TAG_Character_Status_Aiming, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::AimingTagChanged);
	}
}

void AHarmoniaCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, DesiredOverlayMode, COND_SkipOwner);
}

void AHarmoniaCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		static FGameplayTag InAirTag = FGameplayTag::RequestGameplayTag(FName("State.InAir"));
		EMovementMode CurrentMode = GetCharacterMovement()->MovementMode;
		
		if (GetCharacterMovement()->IsFalling())
		{
			if (!ASC->HasMatchingGameplayTag(InAirTag))
			{
				ASC->AddLooseGameplayTag(InAirTag);
			}
		}
		else if (CurrentMode == MOVE_Walking || CurrentMode == MOVE_NavWalking)
		{
			if (ASC->HasMatchingGameplayTag(InAirTag))
			{
				ASC->RemoveLooseGameplayTag(InAirTag);
			}
		}
	}
}

void AHarmoniaCharacter::SetDesiredOverlayMode(const FGameplayTag& NewModeTag)
{
	if (DesiredOverlayMode != NewModeTag)
	{
		if (GetLocalRole() == ROLE_Authority)
		{
			const FGameplayTag Prev = DesiredOverlayMode;
			DesiredOverlayMode = NewModeTag;
			OnRep_DesiredOverlayMode(Prev);
		}
		else
		{
			ServerSetDesiredOverlayMode(NewModeTag);
		}
	}
}

void AHarmoniaCharacter::ServerSetDesiredOverlayMode_Implementation(const FGameplayTag& NewModeTag)
{
	SetDesiredOverlayMode(NewModeTag);
}

void AHarmoniaCharacter::OnRep_DesiredOverlayMode(const FGameplayTag& PreviousMode)
{
	if (DesiredOverlayMode == PreviousMode)
	{
		return;
	}

	bChangingOverlayMode = true;
	PreviousOverlayMode = PreviousMode; 

	PreOverlayModeChanged(PreviousMode);
}

void AHarmoniaCharacter::AimingTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		SetDesiredAiming(false);
	}
	else
	{
		SetDesiredAiming(true);
	}
}

void AHarmoniaCharacter::PreOverlayModeChanged(const FGameplayTag& PreviousMode)
{
	OnUnEquipAnimation(true);
	SetOverlayMode(PreviousMode);
	OnUnEquipAnimation(false);
}

void AHarmoniaCharacter::PostOverlayModeChanged()
{
	OnEquipAnimation(true);
	SetOverlayMode(DesiredOverlayMode);
	OnEquipAnimation(false);
}

void AHarmoniaCharacter::OnEquipAnimation(bool bStart)
{
	if (!bStart)
	{
		bChangingOverlayMode = false;
	}
}

void AHarmoniaCharacter::OnUnEquipAnimation(bool bStart)
{
	if (!bStart)
	{
		PostOverlayModeChanged();
	}
}

void AHarmoniaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AHarmoniaCharacter::SetRightHandIK(FTransform IKTransform)
{
	RightHandIK.TargetTransform = IKTransform;
}

FTransform AHarmoniaCharacter::GetRightHandIK()
{
	return RightHandIK.TargetTransform;
}

void AHarmoniaCharacter::SetUseRightHandIK(bool bUseIK)
{
	RightHandIK.bUseHandIK = bUseIK;
}

bool AHarmoniaCharacter::GetUseRightHandIK()
{
	return RightHandIK.bUseHandIK;
}

void AHarmoniaCharacter::SetLeftHandIK(FTransform IKTransform)
{
	LeftHandIK.TargetTransform = IKTransform;
}

FTransform AHarmoniaCharacter::GetLeftHandIK()
{
	return LeftHandIK.TargetTransform;
}

void AHarmoniaCharacter::SetUseLeftHandIK(bool bUseIK)
{
	LeftHandIK.bUseHandIK = bUseIK;
}

bool AHarmoniaCharacter::GetUseLeftHandIK()
{
	return LeftHandIK.bUseHandIK;
}

// ============================================================
// Mantling
// ============================================================

bool AHarmoniaCharacter::StartMantling(const FAlsMantlingTraceSettings& TraceSettings)
{
	if (!bGAMantlingRequest)
	{
		return false;
	}

	return Super::StartMantling(TraceSettings);
}

bool AHarmoniaCharacter::TryMantleFromGA()
{
	bGAMantlingRequest = true;

	bool bSuccess = StartMantlingGrounded();
	if (!bSuccess)
	{
		bSuccess = StartMantlingInAir();
	}

	bGAMantlingRequest = false;

	return bSuccess;
}

void AHarmoniaCharacter::OnMantlingStarted_Implementation(const FAlsMantlingParameters& Parameters)
{
	Super::OnMantlingStarted_Implementation(Parameters);
}

void AHarmoniaCharacter::OnMantlingEnded_Implementation()
{
	Super::OnMantlingEnded_Implementation();
	OnMantlingEndedDelegate.Broadcast();
}
