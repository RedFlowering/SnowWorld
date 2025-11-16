# Harmonia Combo Attack System

## Overview

The Harmonia Combo Attack System provides a flexible, DataTable-driven melee combat system that allows players to chain attacks together in sequence. When the player continues to press the attack button within a combo window, the next attack in the sequence will be triggered. If the player stops attacking, the combo resets.

## Features

- **DataTable-Driven**: All combo data is defined in DataTables for easy design iteration
- **Gameplay Tag Integration**: Uses Gameplay Tags for state management and event handling
- **Combo Window System**: Configurable time windows for combo continuation
- **Auto-Reset**: Combo automatically resets when input stops
- **Animation Support**: Integrated with AnimMontage system
- **Multiplier System**: Each combo step can have different damage and range multipliers

## Architecture

### Key Components

1. **HarmoniaGameplayAbility_ComboAttack**: The main Gameplay Ability class that handles combo logic
2. **FComboAttackData**: DataTable row structure defining individual combo attacks
3. **HarmoniaGameplayTags**: Gameplay Tags for combo system state management

### Gameplay Tags

The system uses the following tags:

#### Input Tags
- `InputTag.Attack.Melee`: Triggers melee attack

#### Ability Tags
- `Ability.Attack.Melee.Combo1`: First combo attack
- `Ability.Attack.Melee.Combo2`: Second combo attack
- `Ability.Attack.Melee.Combo3`: Third combo attack
- `Ability.Attack.Melee.Combo4`: Fourth combo attack

#### State Tags
- `State.Combat.Attacking`: Character is performing an attack
- `State.Combat.ComboWindow`: Combo window is currently active

#### Event Tags
- `GameplayEvent.Attack.ComboNext`: Trigger next combo in sequence
- `GameplayEvent.Attack.ComboReset`: Reset combo sequence

## Setup Guide

### 1. Create Combo Data DataTable

In the Unreal Editor:

1. Create a new DataTable asset
2. Set the row structure to `FComboAttackData`
3. Add rows for each combo attack:

**Example DataTable Configuration:**

| Row Name | Display Name | Attack Montage | Damage Multiplier | Range Multiplier | Combo Tag | Combo Window Duration | Can Be Canceled |
|----------|--------------|----------------|-------------------|------------------|-----------|----------------------|-----------------|
| Combo1   | Quick Slash  | AM_Attack_Combo1 | 1.0 | 1.0 | Ability.Attack.Melee.Combo1 | 0.8 | true |
| Combo2   | Heavy Swing  | AM_Attack_Combo2 | 1.5 | 1.2 | Ability.Attack.Melee.Combo2 | 0.9 | true |
| Combo3   | Spin Attack  | AM_Attack_Combo3 | 2.0 | 1.5 | Ability.Attack.Melee.Combo3 | 1.0 | true |
| Combo4   | Finisher     | AM_Attack_Combo4 | 3.0 | 2.0 | Ability.Attack.Melee.Combo4 | 0.5 | false |

### 2. Create Gameplay Ability Blueprint

1. Create a Blueprint based on `UHarmoniaGameplayAbility_ComboAttack`
2. Set the following properties:
   - **Combo Data Table**: Select your combo DataTable
   - **Combo Sequence**: Add row names in order (Combo1, Combo2, Combo3, Combo4)
   - **Activation Policy**: OnInputTriggered
   - **Activation Group**: Exclusive_Replaceable

3. Configure Ability Tags:
   - **Ability Tags**: `Ability.Attack.Melee`
   - **Cancel Abilities with Tag**: `Ability.Attack.Melee`
   - **Block Abilities with Tag**: Leave empty or add conflicting tags

### 3. Grant Ability to Character

#### Option A: Using LyraAbilitySet (Recommended)

Create or modify an Ability Set asset:

```cpp
// In your AbilitySet asset
FLyraAbilitySet_GameplayAbility ComboAbilityEntry;
ComboAbilityEntry.Ability = UHarmoniaGameplayAbility_ComboAttack::StaticClass();
ComboAbilityEntry.AbilityLevel = 1;
ComboAbilityEntry.InputTag = HarmoniaGameplayTags::InputTag_Attack_Melee;
```

#### Option B: Grant Directly in C++

```cpp
// In BaseCharacter or your character initialization
void ABaseCharacter::GrantComboAbility()
{
    if (ULyraAbilitySystemComponent* ASC = GetLyraAbilitySystemComponent())
    {
        FGameplayAbilitySpec AbilitySpec(
            UHarmoniaGameplayAbility_ComboAttack::StaticClass(),
            1, // Level
            INDEX_NONE,
            this
        );

        ASC->GiveAbility(AbilitySpec);
    }
}
```

#### Option C: Blueprint-Based (Easiest for Prototyping)

In your BaseCharacter Blueprint:

1. In the Event Graph, add `Event BeginPlay`
2. Get the Ability System Component
3. Call `Give Ability` with your Combo Attack ability class
4. Set the Input Tag to `InputTag.Attack.Melee`

### 4. Setup Input Binding

In your Enhanced Input configuration:

1. Create an Input Action for melee attack
2. In your Player Controller or Hero Component:

```cpp
void UMyHeroComponent::SetupInputBinding()
{
    if (ULyraAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        // Bind the input action to the Gameplay Tag
        ASC->BindAbilityActivationToInputComponent(
            InputComponent,
            FGameplayAbilityInputBinds(
                "ConfirmInput",
                "CancelInput",
                "ELyraAbilityInputID"
            )
        );

        // Trigger ability on input
        InputComponent->BindAction(
            MeleeAttackAction,
            ETriggerEvent::Triggered,
            this,
            &UMyHeroComponent::OnMeleeAttackInput
        );
    }
}

void UMyHeroComponent::OnMeleeAttackInput()
{
    if (ULyraAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        ASC->TryActivateAbilitiesByTag(
            FGameplayTagContainer(HarmoniaGameplayTags::InputTag_Attack_Melee)
        );
    }
}
```

Or use Lyra's native input system which supports tag-based activation automatically.

## How It Works

### Combo Flow

1. **Player presses attack** → First combo attack starts
2. **Montage plays** → Character performs attack animation
3. **Combo window opens** → `State.Combat.ComboWindow` tag is added
4. **Player presses attack again (within window)** → Next combo attack queues
5. **Current montage completes** → Next combo attack starts
6. **Repeat steps 2-5** for each combo in sequence
7. **Player stops attacking OR combo window expires** → Combo resets after timeout

### Combo Reset Conditions

The combo will reset when:
- Player doesn't input next attack within the combo window
- Ability is cancelled (e.g., by taking damage if configured)
- Maximum combo sequence is reached
- Character dies or is stunned (if configured with appropriate tags)

## Advanced Usage

### Custom Combo Logic

You can extend `UHarmoniaGameplayAbility_ComboAttack` to add custom behavior:

```cpp
UCLASS()
class UMyCustomComboAbility : public UHarmoniaGameplayAbility_ComboAttack
{
    GENERATED_BODY()

protected:
    virtual void PerformComboAttack() override
    {
        Super::PerformComboAttack();

        // Add custom logic here
        const FComboAttackData* ComboData = GetCurrentComboData();
        if (ComboData)
        {
            // Apply damage
            ApplyDamageToNearbyEnemies(ComboData->DamageMultiplier);

            // Spawn VFX
            SpawnComboEffect(ComboData->ComboTag);
        }
    }
};
```

### Blueprint Events

The ability exposes Blueprint events for customization:

- `OnAbilityAdded`: Called when ability is granted
- `OnAbilityRemoved`: Called when ability is removed
- `OnPawnAvatarSet`: Called when pawn avatar is set

### Integration with Other Systems

#### With Damage System

```cpp
void UMyCustomComboAbility::PerformComboAttack()
{
    Super::PerformComboAttack();

    const FComboAttackData* ComboData = GetCurrentComboData();
    if (ComboData)
    {
        // Create damage gameplay effect
        UGameplayEffect* DamageEffect = NewObject<UGameplayEffect>();
        // Configure effect with ComboData->DamageMultiplier
        ApplyGameplayEffectToTarget(...);
    }
}
```

#### With Animation Notifies

In your attack montages, add Animation Notifies to:
- Trigger hit detection at specific frames
- Play impact sounds/VFX
- Enable/disable combo window at precise moments

```cpp
// In your AnimNotify
void UAnimNotify_ComboHitWindow::Notify(...)
{
    if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
    {
        ASC->AddLooseGameplayTag(HarmoniaGameplayTags::State_Combat_ComboWindow);
    }
}
```

## Troubleshooting

### Combo doesn't advance
- Check that Combo Window Duration is long enough in DataTable
- Verify that `State.Combat.ComboWindow` tag is being added
- Ensure ComboSequence array has correct row names

### Ability won't activate
- Verify Input Tag is correctly bound
- Check that character has Ability System Component
- Ensure ability was granted to the character
- Check for blocking tags on the character

### Animation doesn't play
- Verify AttackMontage is set in DataTable
- Check that character has AnimInstance
- Ensure montage is compatible with character skeleton

## Performance Considerations

- DataTable lookups are cached per combo step
- Timer-based combo windows are lightweight
- Montage tasks are automatically cleaned up
- Consider pooling damage effects for frequent attacks

## Future Enhancements

Potential features for future versions:
- Directional combo variants (forward, back, side attacks)
- Branching combos (different paths based on input)
- Combo counter UI integration
- Combo achievement/scoring system
- Multiplayer combo synchronization improvements
