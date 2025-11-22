// Copyright 2025 Snow Game Studio.

#include "Libraries/HarmoniaCombatLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "HarmoniaGameplayTags.h"
#include "Definitions/HarmoniaCombatSystemDefinitions.h"

// ============================================================================
// Direction Calculations
// ============================================================================

bool UHarmoniaCombatLibrary::IsAttackFromBehind(AActor* Target, FVector AttackOrigin, float AngleTolerance)
{
	if (!Target)
	{
		return false;
	}

	// Get target's forward vector
	FVector TargetForward = Target->GetActorForwardVector();
	TargetForward.Z = 0.0f; // Ignore vertical component
	TargetForward.Normalize();

	// Get direction from target to attacker
	FVector ToAttacker = AttackOrigin - Target->GetActorLocation();
	ToAttacker.Z = 0.0f; // Ignore vertical component
	ToAttacker.Normalize();

	// Calculate angle between target's forward and direction to attacker
	const float DotProduct = FVector::DotProduct(TargetForward, ToAttacker);
	const float AngleRadians = FMath::Acos(DotProduct);
	const float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

	// Check if within backstab angle tolerance
	// 180 degrees means directly behind, lower tolerance = stricter requirement
	const float RequiredAngle = 180.0f - AngleTolerance;

	return AngleDegrees >= RequiredAngle;
}

EHarmoniaHitDirection UHarmoniaCombatLibrary::GetHitDirection(AActor* Target, FVector HitLocation)
{
	if (!Target)
	{
		return EHarmoniaHitDirection::Front;
	}

	// Get direction from target to hit location
	FVector ToHit = HitLocation - Target->GetActorLocation();
	ToHit.Z = 0.0f;
	ToHit.Normalize();

	FVector Forward = Target->GetActorForwardVector();
	FVector Right = Target->GetActorRightVector();

	// Calculate dot products
	float ForwardDot = FVector::DotProduct(Forward, ToHit);
	float RightDot = FVector::DotProduct(Right, ToHit);

	// Determine direction based on dot products
	if (FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
	{
		return (ForwardDot > 0.0f) ? EHarmoniaHitDirection::Front : EHarmoniaHitDirection::Back;
	}
	else
	{
		return (RightDot > 0.0f) ? EHarmoniaHitDirection::Right : EHarmoniaHitDirection::Left;
	}
}

float UHarmoniaCombatLibrary::GetAngleBetweenVectors(FVector VectorA, FVector VectorB, bool bIgnoreZ)
{
	if (bIgnoreZ)
	{
		VectorA.Z = 0.0f;
		VectorB.Z = 0.0f;
	}

	VectorA.Normalize();
	VectorB.Normalize();

	float DotProduct = FVector::DotProduct(VectorA, VectorB);
	DotProduct = FMath::Clamp(DotProduct, -1.0f, 1.0f);

	return FMath::RadiansToDegrees(FMath::Acos(DotProduct));
}

// ============================================================================
// Combat State Queries
// ============================================================================

bool UHarmoniaCombatLibrary::CanAttackTarget(AActor* Attacker, AActor* Target)
{
	if (!Attacker || !Target)
	{
		return false;
	}

	// Same actor
	if (Attacker == Target)
	{
		return false;
	}

	// Target is invulnerable
	if (IsInvulnerable(Target))
	{
		return false;
	}

	// Target is dead (has dead tag)
	UAbilitySystemComponent* TargetASC = GetAbilitySystemFromActor(Target);
	if (TargetASC)
	{
		FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Dead"));
		if (TargetASC->HasMatchingGameplayTag(DeadTag))
		{
			return false;
		}
	}

	return true;
}

bool UHarmoniaCombatLibrary::IsInvulnerable(AActor* Actor)
{
	if (!Actor)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemFromActor(Actor);
	if (ASC)
	{
		FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Invulnerable"));
		return ASC->HasMatchingGameplayTag(InvulnerableTag);
	}

	return false;
}

bool UHarmoniaCombatLibrary::IsAttacking(AActor* Actor)
{
	if (!Actor)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemFromActor(Actor);
	if (ASC)
	{
		FGameplayTag AttackingTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Attacking"));
		return ASC->HasMatchingGameplayTag(AttackingTag);
	}

	return false;
}

// ============================================================================
// Damage Calculations
// ============================================================================

float UHarmoniaCombatLibrary::CalculateDamage(float BaseDamage, float DamageMultiplier, bool bIsCritical, float CriticalMultiplier)
{
	float FinalDamage = BaseDamage * DamageMultiplier;

	if (bIsCritical)
	{
		FinalDamage *= CriticalMultiplier;
	}

	return FinalDamage;
}

float UHarmoniaCombatLibrary::ApplyBlockReduction(float Damage, bool bIsBlocked, float BlockReduction)
{
	if (bIsBlocked)
	{
		return Damage * (1.0f - FMath::Clamp(BlockReduction, 0.0f, 1.0f));
	}
	return Damage;
}

bool UHarmoniaCombatLibrary::RollForCritical(float CriticalChance, bool bGuaranteedCrit)
{
	if (bGuaranteedCrit)
	{
		return true;
	}

	return FMath::FRand() < FMath::Clamp(CriticalChance, 0.0f, 1.0f);
}

// ============================================================================
// Hit Detection Helpers
// ============================================================================

bool UHarmoniaCombatLibrary::BoxSweepForHits(
	UObject* World,
	FVector Origin,
	FVector Direction,
	float Distance,
	FVector BoxExtent,
	FRotator Rotation,
	const TArray<AActor*>& IgnoredActors,
	TArray<FHitResult>& OutHits,
	bool bDebug)
{
	if (!World)
	{
		return false;
	}

	UWorld* GameWorld = World->GetWorld();
	if (!GameWorld)
	{
		return false;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(IgnoredActors);
	QueryParams.bTraceComplex = false;

	FVector End = Origin + Direction * Distance;

	bool bHit = GameWorld->SweepMultiByChannel(
		OutHits,
		Origin,
		End,
		Rotation.Quaternion(),
		ECC_Pawn,
		FCollisionShape::MakeBox(BoxExtent),
		QueryParams
	);

	if (bDebug)
	{
		DrawDebugBox(GameWorld, Origin, BoxExtent, Rotation.Quaternion(), bHit ? FColor::Green : FColor::Red, false, 2.0f);
		DrawDebugBox(GameWorld, End, BoxExtent, Rotation.Quaternion(), bHit ? FColor::Green : FColor::Red, false, 2.0f);
		DrawDebugLine(GameWorld, Origin, End, bHit ? FColor::Green : FColor::Red, false, 2.0f);
	}

	return bHit;
}

bool UHarmoniaCombatLibrary::SphereSweepForHits(
	UObject* World,
	FVector Origin,
	FVector Direction,
	float Distance,
	float Radius,
	const TArray<AActor*>& IgnoredActors,
	TArray<FHitResult>& OutHits,
	bool bDebug)
{
	if (!World)
	{
		return false;
	}

	UWorld* GameWorld = World->GetWorld();
	if (!GameWorld)
	{
		return false;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(IgnoredActors);
	QueryParams.bTraceComplex = false;

	FVector End = Origin + Direction * Distance;

	bool bHit = GameWorld->SweepMultiByChannel(
		OutHits,
		Origin,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	if (bDebug)
	{
		DrawDebugSphere(GameWorld, Origin, Radius, 12, bHit ? FColor::Green : FColor::Red, false, 2.0f);
		DrawDebugSphere(GameWorld, End, Radius, 12, bHit ? FColor::Green : FColor::Red, false, 2.0f);
		DrawDebugLine(GameWorld, Origin, End, bHit ? FColor::Green : FColor::Red, false, 2.0f);
	}

	return bHit;
}

// ============================================================================
// Gameplay Tag Helpers
// ============================================================================

void UHarmoniaCombatLibrary::AddGameplayTag(AActor* Actor, FGameplayTag Tag)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemFromActor(Actor))
	{
		ASC->AddLooseGameplayTag(Tag);
	}
}

void UHarmoniaCombatLibrary::RemoveGameplayTag(AActor* Actor, FGameplayTag Tag)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemFromActor(Actor))
	{
		ASC->RemoveLooseGameplayTag(Tag);
	}
}

bool UHarmoniaCombatLibrary::HasGameplayTag(AActor* Actor, FGameplayTag Tag)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemFromActor(Actor))
	{
		return ASC->HasMatchingGameplayTag(Tag);
	}
	return false;
}

UAbilitySystemComponent* UHarmoniaCombatLibrary::GetAbilitySystemFromActor(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
}

// ============================================================================
// Animation Helpers
// ============================================================================

float UHarmoniaCombatLibrary::PlayMontageOnActor(AActor* Actor, UAnimMontage* Montage, float PlayRate, FName SectionName)
{
	if (!Actor || !Montage)
	{
		return 0.0f;
	}

	ACharacter* Character = Cast<ACharacter>(Actor);
	if (!Character)
	{
		return 0.0f;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh)
	{
		return 0.0f;
	}

	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	if (!AnimInstance)
	{
		return 0.0f;
	}

	float Duration = AnimInstance->Montage_Play(Montage, PlayRate);

	if (SectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}

	return Duration;
}

void UHarmoniaCombatLibrary::StopMontageOnActor(AActor* Actor, UAnimMontage* Montage, float BlendOutTime)
{
	if (!Actor)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(Actor);
	if (!Character)
	{
		return;
	}

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh)
	{
		return;
	}

	UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	if (Montage)
	{
		AnimInstance->Montage_Stop(BlendOutTime, Montage);
	}
	else
	{
		AnimInstance->Montage_Stop(BlendOutTime);
	}
}
