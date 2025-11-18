// Copyright 2025 Snow Game Studio.

#include "Examples/HarmoniaMeleeCharacterExample.h"
#include "Components/HarmoniaMeleeCombatComponent.h"
#include "Components/HarmoniaSenseAttackComponent.h"
#include "Components/HarmoniaEquipmentComponent.h"
#include "Components/HarmoniaLockOnComponent.h"
#include "Abilities/HarmoniaGameplayAbility_MeleeAttack.h"
#include "Abilities/HarmoniaGameplayAbility_Block.h"
#include "Abilities/HarmoniaGameplayAbility_Parry.h"
#include "Abilities/HarmoniaGameplayAbility_Dodge.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

AHarmoniaMeleeCharacterExample::AHarmoniaMeleeCharacterExample(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create melee combat component
	MeleeCombatComponent = CreateDefaultSubobject<UHarmoniaMeleeCombatComponent>(TEXT("MeleeCombatComponent"));

	// Create lock-on component
	LockOnComponent = CreateDefaultSubobject<UHarmoniaLockOnComponent>(TEXT("LockOnComponent"));

	// Create attack components
	MainHandAttackComponent = CreateDefaultSubobject<UHarmoniaSenseAttackComponent>(TEXT("MainHandAttackComponent"));
	MainHandAttackComponent->SetupAttachment(GetMesh(), FName("weapon_r")); // Attach to right hand socket

	OffHandAttackComponent = CreateDefaultSubobject<UHarmoniaSenseAttackComponent>(TEXT("OffHandAttackComponent"));
	OffHandAttackComponent->SetupAttachment(GetMesh(), FName("weapon_l")); // Attach to left hand socket
	OffHandAttackComponent->SetAutoActivate(false); // Disabled by default

	// Configure main hand attack component
	MainHandAttackComponent->AttackData.TraceConfig.TraceShape = EHarmoniaAttackTraceShape::Box;
	MainHandAttackComponent->AttackData.TraceConfig.TraceExtent = FVector(100.0f, 50.0f, 50.0f);
	MainHandAttackComponent->AttackData.TraceConfig.TraceOffset = FVector(50.0f, 0.0f, 0.0f);
	MainHandAttackComponent->AttackData.TraceConfig.bHitOncePerTarget = true;
	MainHandAttackComponent->AttackData.TraceConfig.MaxTargets = 5;
}

void AHarmoniaMeleeCharacterExample::BeginPlay()
{
	Super::BeginPlay();

	// Grant melee abilities
	GrantMeleeAbilities();

	// Add input mapping context
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (MeleeCombatMappingContext)
			{
				Subsystem->AddMappingContext(MeleeCombatMappingContext, 1); // Priority 1
			}
		}
	}

	// Set default weapon
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetCurrentWeaponType(EHarmoniaMeleeWeaponType::Sword);
	}
}

void AHarmoniaMeleeCharacterExample::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind light attack
		if (LightAttackAction)
		{
			EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Triggered, this, &AHarmoniaMeleeCharacterExample::OnLightAttack);
		}

		// Bind heavy attack
		if (HeavyAttackAction)
		{
			EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this, &AHarmoniaMeleeCharacterExample::OnHeavyAttack);
		}

		// Bind block
		if (BlockAction)
		{
			EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Started, this, &AHarmoniaMeleeCharacterExample::OnBlockStarted);
			EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Completed, this, &AHarmoniaMeleeCharacterExample::OnBlockCompleted);
		}

		// Bind parry
		if (ParryAction)
		{
			EnhancedInputComponent->BindAction(ParryAction, ETriggerEvent::Triggered, this, &AHarmoniaMeleeCharacterExample::OnParry);
		}

		// Bind dodge
		if (DodgeAction)
		{
			EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &AHarmoniaMeleeCharacterExample::OnDodge);
		}

		// Bind lock on
		if (LockOnAction)
		{
			EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Triggered, this, &AHarmoniaMeleeCharacterExample::OnLockOnToggle);
		}
	}
}

// ============================================================================
// Input Callbacks
// ============================================================================

void AHarmoniaMeleeCharacterExample::OnLightAttack(const FInputActionValue& Value)
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(this))
	{
		if (LightAttackAbilityClass)
		{
			ASC->TryActivateAbilityByClass(LightAttackAbilityClass);
		}
	}
}

void AHarmoniaMeleeCharacterExample::OnHeavyAttack(const FInputActionValue& Value)
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(this))
	{
		if (HeavyAttackAbilityClass)
		{
			ASC->TryActivateAbilityByClass(HeavyAttackAbilityClass);
		}
	}
}

void AHarmoniaMeleeCharacterExample::OnBlockStarted(const FInputActionValue& Value)
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(this))
	{
		if (BlockAbilityClass)
		{
			FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(BlockAbilityClass);
			if (Spec)
			{
				BlockAbilityHandle = Spec->Handle;
				ASC->TryActivateAbility(Spec->Handle);
			}
		}
	}
}

void AHarmoniaMeleeCharacterExample::OnBlockCompleted(const FInputActionValue& Value)
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(this))
	{
		if (BlockAbilityHandle.IsValid())
		{
			ASC->CancelAbilityHandle(BlockAbilityHandle);
			BlockAbilityHandle = FGameplayAbilitySpecHandle();
		}
	}
}

void AHarmoniaMeleeCharacterExample::OnParry(const FInputActionValue& Value)
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(this))
	{
		if (ParryAbilityClass)
		{
			ASC->TryActivateAbilityByClass(ParryAbilityClass);
		}
	}
}

void AHarmoniaMeleeCharacterExample::OnDodge(const FInputActionValue& Value)
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(this))
	{
		if (DodgeAbilityClass)
		{
			ASC->TryActivateAbilityByClass(DodgeAbilityClass);
		}
	}
}

void AHarmoniaMeleeCharacterExample::OnLockOnToggle(const FInputActionValue& Value)
{
	if (LockOnComponent)
	{
		LockOnComponent->ToggleLockOn();
	}
}

// ============================================================================
// Example Functions
// ============================================================================

void AHarmoniaMeleeCharacterExample::SwitchWeapon(EHarmoniaMeleeWeaponType NewWeaponType)
{
	if (MeleeCombatComponent)
	{
		MeleeCombatComponent->SetCurrentWeaponType(NewWeaponType);

		// Update equipment visuals (if using equipment component)
		// UHarmoniaEquipmentComponent* EquipmentComp = FindComponentByClass<UHarmoniaEquipmentComponent>();
		// if (EquipmentComp)
		// {
		//     // Update equipped weapon mesh
		// }
	}
}

bool AHarmoniaMeleeCharacterExample::CanAttack() const
{
	if (MeleeCombatComponent)
	{
		return MeleeCombatComponent->CanAttack();
	}
	return false;
}

bool AHarmoniaMeleeCharacterExample::CanBlock() const
{
	if (MeleeCombatComponent)
	{
		return MeleeCombatComponent->CanBlock();
	}
	return false;
}

bool AHarmoniaMeleeCharacterExample::CanDodge() const
{
	if (MeleeCombatComponent)
	{
		return MeleeCombatComponent->CanDodge();
	}
	return false;
}

float AHarmoniaMeleeCharacterExample::GetStaminaPercent() const
{
	if (MeleeCombatComponent)
	{
		const float Current = MeleeCombatComponent->GetCurrentStamina();
		const float Max = MeleeCombatComponent->GetMaxStamina();
		return Max > 0.0f ? (Current / Max) : 0.0f;
	}
	return 0.0f;
}

int32 AHarmoniaMeleeCharacterExample::GetCurrentComboIndex() const
{
	if (MeleeCombatComponent)
	{
		return MeleeCombatComponent->GetCurrentComboIndex();
	}
	return 0;
}

EHarmoniaMeleeWeaponType AHarmoniaMeleeCharacterExample::GetCurrentWeaponType() const
{
	if (MeleeCombatComponent)
	{
		return MeleeCombatComponent->GetCurrentWeaponType();
	}
	return EHarmoniaMeleeWeaponType::None;
}

// ============================================================================
// Private Functions
// ============================================================================

void AHarmoniaMeleeCharacterExample::GrantMeleeAbilities()
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(this))
	{
		// Grant light attack
		if (LightAttackAbilityClass)
		{
			ASC->GiveAbility(FGameplayAbilitySpec(LightAttackAbilityClass, 1, INDEX_NONE, this));
		}

		// Grant heavy attack
		if (HeavyAttackAbilityClass)
		{
			ASC->GiveAbility(FGameplayAbilitySpec(HeavyAttackAbilityClass, 1, INDEX_NONE, this));
		}

		// Grant block
		if (BlockAbilityClass)
		{
			ASC->GiveAbility(FGameplayAbilitySpec(BlockAbilityClass, 1, INDEX_NONE, this));
		}

		// Grant parry
		if (ParryAbilityClass)
		{
			ASC->GiveAbility(FGameplayAbilitySpec(ParryAbilityClass, 1, INDEX_NONE, this));
		}

		// Grant dodge
		if (DodgeAbilityClass)
		{
			ASC->GiveAbility(FGameplayAbilitySpec(DodgeAbilityClass, 1, INDEX_NONE, this));
		}
	}
}
