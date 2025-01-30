// Copyright 2024 RedFlowering.

#include "Character/RFCharacter.h"
#include "RFCharacterMovementComponent.h"
#include "LyraCharacterWithAbilities.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Evaluation/MovieSceneTimeWarping.h"
#include "Settings/AlsCharacterSettings.h"
#include "Utility/AlsConstants.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Status_Aiming, "Status.Aiming");

ARFCharacter::ARFCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<URFCharacterMovementComponent>(ALyraCharacter::CharacterMovementComponentName))
{
	RFCharacterMovement = Cast<URFCharacterMovementComponent>(AlsCharacterMovement);

	WindNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WindNiagara"));
	WindNiagaraComponent->SetupAttachment(RootComponent);
	WindNiagaraComponent->SetRelativeLocationAndRotation(FVector(500.f, 0.f, 100.f), FRotator(0.f, -90.f, 0.f));
	
	LeftLegNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LeftLegNiagara"));
	LeftLegNiagaraComponent->SetupAttachment(GetMesh(), FName("LeftLeg_BoostNozzleSocket"));
	LeftLegNiagaraComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	
	RightLegNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RightLegNiagara"));
	RightLegNiagaraComponent->SetupAttachment(GetMesh(), FName("RightLeg_BoostNozzleSocket"));
	RightLegNiagaraComponent->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));

	WindNiagaraComponent->SetAutoActivate(false);
	LeftLegNiagaraComponent->SetAutoActivate(false);
	RightLegNiagaraComponent->SetAutoActivate(false);
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

void ARFCharacter::PlayBoostEffect(bool activate)
{
	if(!LeftLegNiagaraComponent || !RightLegNiagaraComponent)
		return;
	
	if(activate)
	{
		if(!LeftLegNiagaraComponent->IsActive())
		{
			LeftLegNiagaraComponent->Activate();
		}
		if(!RightLegNiagaraComponent->IsActive())
		{
			RightLegNiagaraComponent->Activate();
		}
		return;
	}

	if(LeftLegNiagaraComponent->IsActive())
	{
		LeftLegNiagaraComponent->Deactivate();
	}
	if(RightLegNiagaraComponent->IsActive())
	{
		RightLegNiagaraComponent->Deactivate();
	}
	
}
