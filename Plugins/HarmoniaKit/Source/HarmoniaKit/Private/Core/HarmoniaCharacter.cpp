// Copyright 2025 Snow Game Studio.

#include "Core/HarmoniaCharacter.h"
#include "Core/HarmoniaCharacterMovementComponent.h"
#include "Core/HarmoniaLockOnTargetingComponent.h"
#include "Core/HarmoniaHealthComponent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Settings/AlsCharacterSettings.h"
#include "Utility/AlsConstants.h"
#include "SenseReceiverComponent.h"
#include "Components/CapsuleComponent.h" // Needed for SetDefaultSubobjectClass usage usually implies access to base
#include "Net/UnrealNetwork.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Status_Aiming, "Status.Aiming");

AHarmoniaCharacter::AHarmoniaCharacter(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer
		.SetDefaultSubobjectClass<UHarmoniaCharacterMovementComponent>(ALyraCharacter::CharacterMovementComponentName)
		.SetDefaultSubobjectClass<UHarmoniaHealthComponent>(TEXT("HealthComponent")))
{
	HarmoniaCharacterMovement = Cast<UHarmoniaCharacterMovementComponent>(GetCharacterMovement());

	// Create lock-on targeting component
	LockOnComponent = CreateDefaultSubobject<UHarmoniaLockOnTargetingComponent>(TEXT("LockOnComponent"));

	// Cast parent's HealthComponent to HarmoniaHealthComponent (now accessible as protected member)
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

void AHarmoniaCharacter::SetDesiredOverlayMode(const FGameplayTag& NewModeTag)
{
	if (DesiredOverlayMode != NewModeTag)
	{
		// Autonomous proxy: Set locally for prediction (if using COND_SkipOwner), then notify server
		// If using COND_SkipOwner, we must set it locally.
		// However, overlay states usually don't need strict prediction like movement.
		// Let's stick to Server Authoritative model for simplicity and correctness first.
		
		// If Local, and Authority, set directly.
		if (GetLocalRole() == ROLE_Authority)
		{
			const FGameplayTag Prev = DesiredOverlayMode;
			DesiredOverlayMode = NewModeTag;
			OnRep_DesiredOverlayMode(Prev); // Explicit call for Listen Server
		}
		else
		{
			// Client: Send request to server
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
		// No change?
		return;
	}

	// Logic adaptation:
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
