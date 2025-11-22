# Stamina System Implementation Guide

## Overview

HarmoniaAttributeSet now includes a comprehensive **Soul-like RPG stat system** with stamina mechanics. This guide will help you set up and use the stamina system in your project.

## New Attributes

### Core Attributes
- **Health** / **MaxHealth** - Character hit points
- **Stamina** / **MaxStamina** - Action resource (dodge, attack, block)
- **StaminaRegenRate** - Stamina recovered per second (default: 10.0)

### Primary Stats (Soul-like RPG System)
All primary stats range from 1-99 and start at 10:

- **Vitality** - Increases MaxHealth and physical defense
- **Endurance** - Increases MaxStamina, StaminaRegenRate, and MaxEquipLoad
- **Strength** - Increases physical damage
- **Dexterity** - Increases attack speed and critical chance
- **Intelligence** - Increases magic damage and magic defense
- **Faith** - Increases miracles/incantations power and healing effectiveness
- **Luck** - Increases item discovery and affects status buildup

### Combat Stats
- **AttackPower** - Base damage multiplier
- **Defense** - Damage reduction
- **CriticalChance** - Critical hit probability (0-1, default: 0.1)
- **CriticalDamage** - Critical damage multiplier (default: 2.0x)
- **Poise** / **MaxPoise** - Stagger resistance (default: 50.0)

### Movement & Equipment
- **MovementSpeed** - Movement speed multiplier
- **AttackSpeed** - Attack speed multiplier
- **EquipLoad** / **MaxEquipLoad** - Current/max equipment weight

## Setting Up Stamina System

### 1. Create Stamina Regeneration Effect

Create a **GameplayEffect Blueprint** in Content Browser:

**Path**: `Content/HarmoniaKit/Effects/GE_StaminaRegen.uasset`

**Configuration**:
- Duration Policy: **Infinite**
- Period: **0.1** seconds (updates 10 times per second)
- Modifiers:
  - Attribute: `HarmoniaAttributeSet.Stamina`
  - Operation: **Add**
  - Modifier Magnitude: **Attribute Based**
    - Backing Attribute: `HarmoniaAttributeSet.StaminaRegenRate`
    - Coefficient: **0.1** (because period is 0.1s)
    - Pre Multiply Additive Value: **0.0**
    - Post Multiply Additive Value: **0.0**

This will regenerate stamina based on StaminaRegenRate attribute.

### 2. Link Endurance to Stamina Regen Rate

Create a **GameplayEffect Blueprint**:

**Path**: `Content/HarmoniaKit/Effects/GE_EnduranceModifiers.uasset`

**Configuration**:
- Duration Policy: **Instant**
- Modifiers:
  1. **MaxStamina**:
     - Attribute: `HarmoniaAttributeSet.MaxStamina`
     - Operation: **Add**
     - Modifier Magnitude: **Attribute Based**
       - Backing Attribute: `HarmoniaAttributeSet.Endurance`
       - Coefficient: **2.0** (each point of Endurance adds 2 max stamina)

  2. **StaminaRegenRate**:
     - Attribute: `HarmoniaAttributeSet.StaminaRegenRate`
     - Operation: **Add**
     - Modifier Magnitude: **Attribute Based**
       - Backing Attribute: `HarmoniaAttributeSet.Endurance`
       - Coefficient: **0.5** (each point of Endurance adds 0.5 stamina/sec)

  3. **MaxEquipLoad**:
     - Attribute: `HarmoniaAttributeSet.MaxEquipLoad`
     - Operation: **Add**
     - Modifier Magnitude: **Attribute Based**
       - Backing Attribute: `HarmoniaAttributeSet.Endurance`
       - Coefficient: **3.0** (each point of Endurance adds 3 max equip load)

Apply this effect once when character spawns or when Endurance changes.

### 3. Create Stamina Cost Effects

#### Dodge Roll Stamina Cost

**Path**: `Content/HarmoniaKit/Effects/GE_StaminaCost_Dodge.uasset`

**Configuration**:
- Duration Policy: **Instant**
- Modifiers:
  - Attribute: `HarmoniaAttributeSet.Stamina`
  - Operation: **Add**
  - Modifier Magnitude: **Scalable Float**
    - Value: **-20.0** (costs 20 stamina to dodge)

#### Attack Stamina Cost

**Path**: `Content/HarmoniaKit/Effects/GE_StaminaCost_Attack.uasset`

**Configuration**:
- Duration Policy: **Instant**
- Modifiers:
  - Attribute: `HarmoniaAttributeSet.Stamina`
  - Operation: **Add**
  - Modifier Magnitude: **Scalable Float**
    - Value: **-15.0** (costs 15 stamina per attack)

Can be modified based on weapon type:
- Light weapons: -10 stamina
- Medium weapons: -15 stamina
- Heavy weapons: -25 stamina

#### Block Stamina Cost

**Path**: `Content/HarmoniaKit/Effects/GE_StaminaCost_Block.uasset`

**Configuration**:
- Duration Policy: **Has Duration**
- Duration Magnitude: **Infinite** (while blocking)
- Period: **0.1** seconds
- Modifiers:
  - Attribute: `HarmoniaAttributeSet.Stamina`
  - Operation: **Add**
  - Modifier Magnitude: **Scalable Float**
    - Value: **-0.5** (costs 5 stamina per second while blocking)

### 4. Apply Effects to Abilities

In your **Gameplay Ability** classes (C++ or Blueprint):

#### Dodge Ability
```cpp
// In ActivateAbility():
if (GetStamina() >= 20.0f)
{
    // Apply stamina cost
    FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
        DodgeStaminaCostClass, GetAbilityLevel());
    ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo,
        CurrentActivationInfo, SpecHandle);

    // Perform dodge roll
    PerformDodgeRoll();
}
else
{
    // Not enough stamina - cancel ability
    CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}
```

#### Attack Ability
```cpp
// In ActivateAbility():
if (GetStamina() >= 15.0f)
{
    // Apply stamina cost
    FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
        AttackStaminaCostClass, GetAbilityLevel());
    ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo,
        CurrentActivationInfo, SpecHandle);

    // Perform attack
    PerformAttack();
}
```

#### Block Ability
```cpp
// In ActivateAbility():
// Apply continuous stamina drain while blocking
BlockStaminaDrainHandle = ApplyGameplayEffectSpecToOwner(
    MakeOutgoingGameplayEffectSpec(BlockStaminaCostClass, GetAbilityLevel()));

// In EndAbility():
// Remove stamina drain effect
BP_RemoveGameplayEffectFromOwnerWithHandle(BlockStaminaDrainHandle);
```

### 5. UI Integration

#### Stamina Bar Widget (Blueprint)

Create a Progress Bar widget bound to:
```cpp
float GetStaminaPercent()
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        const UHarmoniaAttributeSet* AttributeSet =
            ASC->GetSet<UHarmoniaAttributeSet>();
        if (AttributeSet)
        {
            return AttributeSet->GetStamina() / AttributeSet->GetMaxStamina();
        }
    }
    return 0.0f;
}
```

#### Listen to Stamina Changes

```cpp
// In C++:
void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        const UHarmoniaAttributeSet* AttributeSet =
            ASC->GetSet<UHarmoniaAttributeSet>();
        if (AttributeSet)
        {
            AttributeSet->OnStaminaChanged.AddUObject(
                this, &AMyCharacter::OnStaminaChanged);
            AttributeSet->OnOutOfStamina.AddUObject(
                this, &AMyCharacter::OnOutOfStamina);
        }
    }
}

void AMyCharacter::OnStaminaChanged(AActor* Instigator, AActor* Causer,
    const FGameplayEffectSpec* EffectSpec, float Magnitude,
    float OldValue, float NewValue)
{
    // Update UI
    OnStaminaChangedDelegate.Broadcast(NewValue, GetMaxStamina());
}

void AMyCharacter::OnOutOfStamina(AActor* Instigator, AActor* Causer,
    const FGameplayEffectSpec* EffectSpec, float Magnitude,
    float OldValue, float NewValue)
{
    // Play exhausted animation or effect
    PlayExhaustedEffect();
}
```

## Equipment Load System

The equipment load system affects movement and stamina regeneration:

### Calculate Equipment Load Percentage

```cpp
float GetEquipLoadPercentage()
{
    return EquipLoad / MaxEquipLoad;
}
```

### Apply Movement Speed Penalty

Create **GE_EquipLoadPenalty.uasset**:

**Configuration**:
- Duration Policy: **Infinite**
- Modifiers:
  - Attribute: `HarmoniaAttributeSet.MovementSpeed`
  - Operation: **Multiply**
  - Modifier Magnitude: **Custom Calculation**

Implement custom calculation:
- 0-30% load: 1.0x speed (fast roll)
- 30-70% load: 0.9x speed (medium roll)
- 70-100% load: 0.7x speed (fat roll)
- 100%+ load: 0.5x speed (overburdened)

### Apply Stamina Regen Penalty

Modify the StaminaRegenRate based on EquipLoad:
- 0-30% load: 1.0x regen
- 30-70% load: 0.8x regen
- 70-100% load: 0.5x regen
- 100%+ load: 0.2x regen

## Poise System

Poise represents resistance to stagger/interruption:

### Create Poise Damage Effect

**Path**: `Content/HarmoniaKit/Effects/GE_PoiseDamage.uasset`

**Configuration**:
- Duration Policy: **Instant**
- Modifiers:
  - Attribute: `HarmoniaAttributeSet.Poise`
  - Operation: **Add**
  - Modifier Magnitude: **Set by Caller**
    - Tag: `Data.PoiseDamage`

### Handle Poise Break

```cpp
void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        const UHarmoniaAttributeSet* AttributeSet =
            ASC->GetSet<UHarmoniaAttributeSet>();
        if (AttributeSet)
        {
            AttributeSet->OnPoiseBroken.AddUObject(
                this, &AMyCharacter::OnPoiseBroken);
        }
    }
}

void AMyCharacter::OnPoiseBroken(AActor* Instigator, AActor* Causer,
    const FGameplayEffectSpec* EffectSpec, float Magnitude,
    float OldValue, float NewValue)
{
    // Character is staggered
    PlayStaggerAnimation();

    // Apply stagger gameplay effect (stun for 2 seconds)
    ApplyGameplayEffectToSelf(StaggerEffectClass);

    // Reset poise after delay
    GetWorldTimerManager().SetTimer(
        PoiseResetTimer,
        this,
        &AMyCharacter::ResetPoise,
        2.0f,
        false
    );
}

void AMyCharacter::ResetPoise()
{
    // Restore poise to max
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
            PoiseRestoreEffectClass, 1.0f);
        ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}
```

## Stat Scaling Formulas

Typical soul-like scaling formulas for derived stats:

### MaxHealth (from Vitality)
```
MaxHealth = BaseMaxHealth + (Vitality * 10)
```

### MaxStamina (from Endurance)
```
MaxStamina = BaseMaxStamina + (Endurance * 2)
```

### AttackPower (from Strength/Dexterity)
```
PhysicalAttackPower = BaseAttackPower + (Strength * 2) + (Dexterity * 0.5)
MagicAttackPower = BaseAttackPower + (Intelligence * 2) + (Faith * 0.5)
```

### Defense (from Vitality)
```
Defense = BaseDefense + (Vitality * 0.5)
```

### CriticalChance (from Luck/Dexterity)
```
CriticalChance = BaseCriticalChance + ((Luck + Dexterity) * 0.01)
```

### Poise (from Endurance + Equipment)
```
MaxPoise = BasePoise + (Endurance * 2) + EquipmentPoise
```

## Testing

To test the stamina system:

1. Create a test character with HarmoniaAttributeSet
2. Apply the stamina regen effect on spawn
3. Create test abilities for dodge, attack, and block
4. Test stamina consumption and regeneration
5. Verify Endurance affects stamina regen rate
6. Test equipment load penalties
7. Test poise system with stagger mechanics

## Example Character Setup

```cpp
void AMyCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (AbilitySystemComponent)
    {
        // Initialize attribute set
        AbilitySystemComponent->InitStats(
            UHarmoniaAttributeSet::StaticClass(),
            nullptr
        );

        // Apply stamina regeneration effect
        FGameplayEffectContextHandle EffectContext =
            AbilitySystemComponent->MakeEffectContext();
        EffectContext.AddSourceObject(this);

        FGameplayEffectSpecHandle SpecHandle =
            AbilitySystemComponent->MakeOutgoingSpec(
                StaminaRegenEffectClass, 1, EffectContext);

        if (SpecHandle.IsValid())
        {
            AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(
                *SpecHandle.Data.Get());
        }

        // Apply stat modifiers
        ApplyStatModifiers();

        // Give abilities
        GiveAbilities();
    }
}
```

## Conclusion

This stamina system provides a solid foundation for soul-like combat mechanics. The system is:
- **Flexible**: Easy to adjust costs and regen rates
- **Scalable**: Stats can be increased with level-ups
- **Network-ready**: All attributes replicate properly
- **Extensible**: Easy to add new stats or mechanics

For questions or issues, refer to the HarmoniaKit documentation or check the attribute set implementation in:
- `Plugins/HarmoniaKit/Source/HarmoniaKit/Public/AbilitySystem/HarmoniaAttributeSet.h`
- `Plugins/HarmoniaKit/Source/HarmoniaKit/Private/AbilitySystem/HarmoniaAttributeSet.cpp`
