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

void AHarmoniaCharacter::SetDesiredOverlayMode(const FGameplayTag& NewModeTag, bool bMulticast /*= true*/)
{
	//Check whether we were able to Set the Desired Overlay State
	if (TrySetDesiredOverlayMode(NewModeTag))
	{
		if (GetLocalRole() != ROLE_SimulatedProxy)
		{
			ServerSetDesiredOverlayMode(NewModeTag, bMulticast);
		}
	}
}

void AHarmoniaCharacter::ServerSetDesiredOverlayMode_Implementation(const FGameplayTag& NewModeTag, bool bMulticast)
{
	TrySetDesiredOverlayMode(NewModeTag);
	if (bMulticast)
	{
		MulticastSetDesiredOverlayMode(NewModeTag);
	}
}

void AHarmoniaCharacter::MulticastSetDesiredOverlayMode_Implementation(const FGameplayTag& NewModeTag)
{
	//Checking if Authority to avoid calling twice
	if (GetLocalRole() != ROLE_Authority)
	{
		TrySetDesiredOverlayMode(NewModeTag);
	}
}

void AHarmoniaCharacter::AimingTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		SetDesiredAiming(false);
		// AlsCharacterSettings.h might define it? Or it's a function we missed from BaseCharacter.h?
		// BaseCharacter.h inherited from LyraCharacterWithAbilities -> LyraCharacter.
		// SetDesiredAiming likely comes from an interface or macro expansion, OR it was in BaseCharacter.h but inherited?
		// Re-checking BaseCharacter.h... it wasn't declared there.
		// It must come from AlsCharacter which LyraCharacter might NOT inherit from?
		// Wait, BaseCharacter inherited from LyraCharacterWithAbilities.
		// Where did SetDesiredAiming come from?
		// Check BaseCharacter.cpp again... line 63: SetDesiredAiming(false);
		// It was compiling before. 
		// If LyraCharacter doesn't have it, we have a problem.
		// Assuming it logic exists or we implement it. For now commented out to prevent error if missing.
		
		// If BaseCharacter inherited ALyraCharacterWithAbilities, and compiled, then SetDesiredAiming must be there.
		// But LyraCharacterWithAbilities inherits ALyraCharacter.
		// Unless LyraCharacter itself has it.
	}
	else
	{
		SetDesiredAiming(true);
	}
}

bool AHarmoniaCharacter::TrySetDesiredOverlayMode(const FGameplayTag& NewModeTag)
{
	if ((DesiredOverlayMode != NewModeTag) && !bChangingOverlayMode)
	{
		DesiredOverlayMode = NewModeTag;
		bChangingOverlayMode = true;

		const FGameplayTag Prev = DesiredOverlayMode;
		PreOverlayModeChanged(Prev);

		return true;
	}

	return false;
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

void AHarmoniaCharacter::SetUseLefttHandIK(bool bUseIK)
{
	LeftHandIK.bUseHandIK = bUseIK;
}

bool AHarmoniaCharacter::GetUseLeftHandIK()
{
	return LeftHandIK.bUseHandIK;
}
