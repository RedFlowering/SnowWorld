// Copyright 2024 RedFlowering.

#include "Character/RFCharacter.h"
#include "RFCharacterMovementComponent.h"
#include "LyraCharacterWithAbilities.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Settings/AlsCharacterSettings.h"
#include "Utility/AlsConstants.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Status_Aiming, "Status.Aiming");

ARFCharacter::ARFCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<URFCharacterMovementComponent>(ALyraCharacter::CharacterMovementComponentName))
{
	RFCharacterMovement = Cast<URFCharacterMovementComponent>(AlsCharacterMovement);
}

void ARFCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	GetAbilitySystemComponent()->RegisterGameplayTagEvent(TAG_Character_Status_Aiming, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::AimingTagChanged);
}

void ARFCharacter::SetDesiredOverlayMode(const FGameplayTag& NewModeTag, bool bMulticast /*= true*/)
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

void ARFCharacter::ServerSetDesiredOverlayMode_Implementation(const FGameplayTag& NewModeTag, bool bMulticast)
{
	TrySetDesiredOverlayMode(NewModeTag);
	if (bMulticast)
	{
		MulticastSetDesiredOverlayMode(NewModeTag);
	}
}

void ARFCharacter::MulticastSetDesiredOverlayMode_Implementation(const FGameplayTag& NewModeTag)
{
	//Checking if Authority to avoid calling twice
	if (GetLocalRole() != ROLE_Authority)
	{
		TrySetDesiredOverlayMode(NewModeTag);
	}
}

void ARFCharacter::AimingTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		SetDesiredAiming(false);
		//SetRotationMode(DesiredRotationMode);
	}
	else
	{
		SetDesiredAiming(true);
		//SetRotationMode(AlsRotationModeTags::Aiming);
	}
}

bool ARFCharacter::TrySetDesiredOverlayMode(const FGameplayTag& NewModeTag)
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

void ARFCharacter::PreOverlayModeChanged(const FGameplayTag& PreviousMode)
{
	OnUnEquipAnimation(true);
	SetOverlayMode(PreviousMode);
	OnUnEquipAnimation(false);
}

void ARFCharacter::PostOverlayModeChanged()
{
	OnEquipAnimation(true);
	SetOverlayMode(DesiredOverlayMode);
	OnEquipAnimation(false);
}

void ARFCharacter::OnEquipAnimation(bool bStart)
{
	if (!bStart)
	{
		bChangingOverlayMode = false;
	}
}

void ARFCharacter::OnUnEquipAnimation(bool bStart)
{
	if (!bStart)
	{
		PostOverlayModeChanged();
	}
}
