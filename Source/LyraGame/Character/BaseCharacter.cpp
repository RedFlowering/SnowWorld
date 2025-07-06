// Copyright 2024 Snow Game Studio.

#include "Character/BaseCharacter.h"
#include "BaseCharacterMovementComponent.h"
#include "LyraCharacterWithAbilities.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Settings/AlsCharacterSettings.h"
#include "Utility/AlsConstants.h"
#include "SenseReceiverComponent.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Status_Aiming, "Status.Aiming");

ABaseCharacter::ABaseCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UBaseCharacterMovementComponent>(ALyraCharacter::CharacterMovementComponentName))
{
	BaseCharacterMovement = Cast<UBaseCharacterMovementComponent>(GetCharacterMovement());

	WallDetector = CreateDefaultSubobject<USenseReceiverComponent>(TEXT("WallDetector"));
	WallDetector->SetupAttachment(GetRootComponent());
}

void ABaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	GetAbilitySystemComponent()->RegisterGameplayTagEvent(TAG_Character_Status_Aiming, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::AimingTagChanged);
}

void ABaseCharacter::SetDesiredOverlayMode(const FGameplayTag& NewModeTag, bool bMulticast /*= true*/)
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

void ABaseCharacter::ServerSetDesiredOverlayMode_Implementation(const FGameplayTag& NewModeTag, bool bMulticast)
{
	TrySetDesiredOverlayMode(NewModeTag);
	if (bMulticast)
	{
		MulticastSetDesiredOverlayMode(NewModeTag);
	}
}

void ABaseCharacter::MulticastSetDesiredOverlayMode_Implementation(const FGameplayTag& NewModeTag)
{
	//Checking if Authority to avoid calling twice
	if (GetLocalRole() != ROLE_Authority)
	{
		TrySetDesiredOverlayMode(NewModeTag);
	}
}

void ABaseCharacter::AimingTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
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

bool ABaseCharacter::TrySetDesiredOverlayMode(const FGameplayTag& NewModeTag)
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

void ABaseCharacter::PreOverlayModeChanged(const FGameplayTag& PreviousMode)
{
	OnUnEquipAnimation(true);
	SetOverlayMode(PreviousMode);
	OnUnEquipAnimation(false);
}

void ABaseCharacter::PostOverlayModeChanged()
{
	OnEquipAnimation(true);
	SetOverlayMode(DesiredOverlayMode);
	OnEquipAnimation(false);
}

void ABaseCharacter::OnEquipAnimation(bool bStart)
{
	if (!bStart)
	{
		bChangingOverlayMode = false;
	}
}

void ABaseCharacter::OnUnEquipAnimation(bool bStart)
{
	if (!bStart)
	{
		PostOverlayModeChanged();
	}
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABaseCharacter::SetRightHandIK(FTransform IKTransform)
{
	RightHandIK.TargetTransform = IKTransform;
}

FTransform ABaseCharacter::GetRightHandIK()
{
	return RightHandIK.TargetTransform;
}

void ABaseCharacter::SetUseRightHandIK(bool bUseIK)
{
	RightHandIK.bUseHandIK = bUseIK;
}

bool ABaseCharacter::GetUseRightHandIK()
{
	return RightHandIK.bUseHandIK;
}

void ABaseCharacter::SetLeftHandIK(FTransform IKTransform)
{
	LeftHandIK.TargetTransform = IKTransform;
}

FTransform ABaseCharacter::GetLeftHandIK()
{
	return LeftHandIK.TargetTransform;
}

void ABaseCharacter::SetUseLefttHandIK(bool bUseIK)
{
	LeftHandIK.bUseHandIK = bUseIK;
}

bool ABaseCharacter::GetUseLeftHandIK()
{
	return LeftHandIK.bUseHandIK;
}
