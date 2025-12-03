// Copyright 2025 Snow Game Studio.

#include "Components/HarmoniaRangedCombatComponent.h"
#include "HarmoniaLogCategories.h"
#include "Components/HarmoniaLockOnComponent.h"
#include "Actors/HarmoniaProjectile.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SplineComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "HarmoniaLoadManager.h"

UHarmoniaRangedCombatComponent::UHarmoniaRangedCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
}

void UHarmoniaRangedCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// HarmoniaLoadManager를 통해 DataTable 로드
	if (UHarmoniaLoadManager* LoadManager = UHarmoniaLoadManager::Get())
	{
		if (!WeaponDataTable)
		{
			WeaponDataTable = LoadManager->GetDataTableByKey(TEXT("RangedWeapons"));
		}
		if (!SpellDataTable)
		{
			SpellDataTable = LoadManager->GetDataTableByKey(TEXT("Spells"));
		}
	}

	// Initialize ammo based on weapon data
	FHarmoniaRangedWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		CurrentAmmo = WeaponData.MagazineSize;
		TotalAmmo = WeaponData.MaxAmmo;
	}
}

void UHarmoniaRangedCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update draw time
	if (bIsDrawing && bWeaponDrawn)
	{
		CurrentDrawTime += DeltaTime;

		FHarmoniaRangedWeaponData WeaponData;
		if (GetCurrentWeaponData(WeaponData))
		{
			// Clamp to max draw time
			CurrentDrawTime = FMath::Min(CurrentDrawTime, WeaponData.MaxDrawTime);
		}
	}

	// Update reload
	if (bIsReloading)
	{
		ReloadTimeRemaining -= DeltaTime;
		if (ReloadTimeRemaining <= 0.0f)
		{
			PerformReload();
		}
	}

	// Update trajectory visualization
	if (bShowTrajectoryPreview && bIsAiming && bWeaponDrawn && TrajectorySpline)
	{
		TArray<FVector> PathPositions;
		CalculateTrajectoryPath(PathPositions);

		TrajectorySpline->ClearSplinePoints();
		for (int32 i = 0; i < PathPositions.Num(); ++i)
		{
			TrajectorySpline->AddSplinePoint(PathPositions[i], ESplineCoordinateSpace::World, false);
		}
		TrajectorySpline->UpdateSpline();
	}
}

void UHarmoniaRangedCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHarmoniaRangedCombatComponent, CurrentWeaponType);
	DOREPLIFETIME(UHarmoniaRangedCombatComponent, bWeaponDrawn);
	DOREPLIFETIME(UHarmoniaRangedCombatComponent, bIsAiming);
	DOREPLIFETIME(UHarmoniaRangedCombatComponent, bIsDrawing);
	DOREPLIFETIME(UHarmoniaRangedCombatComponent, CurrentDrawTime);
	DOREPLIFETIME(UHarmoniaRangedCombatComponent, bIsReloading);
	DOREPLIFETIME(UHarmoniaRangedCombatComponent, CurrentAmmo);
	DOREPLIFETIME(UHarmoniaRangedCombatComponent, TotalAmmo);
	DOREPLIFETIME(UHarmoniaRangedCombatComponent, CurrentSpellRowName);
}

// ============================================================================
// Weapon Management
// ============================================================================

void UHarmoniaRangedCombatComponent::SetCurrentWeaponType(EHarmoniaRangedWeaponType NewWeaponType)
{
	if (CurrentWeaponType == NewWeaponType)
	{
		return;
	}

	CurrentWeaponType = NewWeaponType;

	// Reset state
	bIsDrawing = false;
	CurrentDrawTime = 0.0f;
	bIsReloading = false;

	// Initialize ammo
	FHarmoniaRangedWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		CurrentAmmo = WeaponData.MagazineSize;
		TotalAmmo = WeaponData.MaxAmmo;
		OnAmmoChanged.Broadcast(CurrentAmmo, WeaponData.MagazineSize);
	}

	OnRep_CurrentWeaponType();
}

bool UHarmoniaRangedCombatComponent::GetCurrentWeaponData(FHarmoniaRangedWeaponData& OutWeaponData) const
{
	if (!WeaponDataTable || CurrentWeaponType == EHarmoniaRangedWeaponType::None)
	{
		return false;
	}

	// Find weapon data in table
	const FString ContextString = TEXT("GetCurrentWeaponData");
	TArray<FHarmoniaRangedWeaponData*> AllWeaponData;
	WeaponDataTable->GetAllRows<FHarmoniaRangedWeaponData>(ContextString, AllWeaponData);

	for (FHarmoniaRangedWeaponData* WeaponData : AllWeaponData)
	{
		if (WeaponData && WeaponData->WeaponType == CurrentWeaponType)
		{
			OutWeaponData = *WeaponData;
			return true;
		}
	}

	return false;
}

void UHarmoniaRangedCombatComponent::DrawWeapon()
{
	if (bWeaponDrawn)
	{
		return;
	}

	bWeaponDrawn = true;
	OnWeaponDrawn.Broadcast();
}

void UHarmoniaRangedCombatComponent::HolsterWeapon()
{
	if (!bWeaponDrawn)
	{
		return;
	}

	bWeaponDrawn = false;
	bIsAiming = false;
	bIsDrawing = false;
	CurrentDrawTime = 0.0f;

	OnWeaponHolstered.Broadcast();
}

// ============================================================================
// Aiming
// ============================================================================

void UHarmoniaRangedCombatComponent::StartAiming()
{
	if (bIsAiming || !bWeaponDrawn)
	{
		return;
	}

	bIsAiming = true;

	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerStartAiming();
	}

	OnAimingStateChanged.Broadcast(true);

	// Apply movement speed penalty
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character && Character->GetCharacterMovement())
	{
		FHarmoniaRangedWeaponData WeaponData;
		if (GetCurrentWeaponData(WeaponData))
		{
			Character->GetCharacterMovement()->MaxWalkSpeed *= WeaponData.AimingMovementSpeedMultiplier;
		}
	}
}

void UHarmoniaRangedCombatComponent::StopAiming()
{
	if (!bIsAiming)
	{
		return;
	}

	bIsAiming = false;

	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerStopAiming();
	}

	OnAimingStateChanged.Broadcast(false);

	// Restore movement speed
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character && Character->GetCharacterMovement())
	{
		FHarmoniaRangedWeaponData WeaponData;
		if (GetCurrentWeaponData(WeaponData))
		{
			Character->GetCharacterMovement()->MaxWalkSpeed /= WeaponData.AimingMovementSpeedMultiplier;
		}
	}
}

void UHarmoniaRangedCombatComponent::ToggleAiming()
{
	if (bIsAiming)
	{
		StopAiming();
	}
	else
	{
		StartAiming();
	}
}

FVector UHarmoniaRangedCombatComponent::GetAimDirection() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ForwardVector;
	}

	// Check if we have a locked target
	UHarmoniaLockOnComponent* LockOn = GetLockOnComponent();
	if (LockOn && LockOn->GetCurrentTarget())
	{
		AActor* Target = LockOn->GetCurrentTarget();
		if (Target)
		{
			FVector AimLoc = GetAimLocation();
			FVector TargetLoc = Target->GetActorLocation();
			return (TargetLoc - AimLoc).GetSafeNormal();
		}
	}

	// Use camera direction if available
	ACharacter* Character = Cast<ACharacter>(Owner);
	if (Character)
	{
		APlayerController* PC = Cast<APlayerController>(Character->GetController());
		if (PC)
		{
			FVector CameraLoc;
			FRotator CameraRot;
			PC->GetPlayerViewPoint(CameraLoc, CameraRot);
			return CameraRot.Vector();
		}
	}

	// Fallback to actor forward
	return Owner->GetActorForwardVector();
}

FVector UHarmoniaRangedCombatComponent::GetAimLocation() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return FVector::ZeroVector;
	}

	FHarmoniaRangedWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		// Try to get muzzle socket location
		USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
		if (Mesh && !WeaponData.MuzzleSocketName.IsNone())
		{
			if (Mesh->DoesSocketExist(WeaponData.MuzzleSocketName))
			{
				return Mesh->GetSocketLocation(WeaponData.MuzzleSocketName);
			}
		}
	}

	// Fallback to actor location + forward offset
	return Owner->GetActorLocation() + Owner->GetActorForwardVector() * 50.0f;
}

FVector UHarmoniaRangedCombatComponent::GetAimTargetLocation() const
{
	FVector AimLoc = GetAimLocation();
	FVector AimDir = GetAimDirection();

	FHarmoniaRangedWeaponData WeaponData;
	float MaxRange = 10000.0f; // Default range
	if (GetCurrentWeaponData(WeaponData) && WeaponData.MaxRange > 0.0f)
	{
		MaxRange = WeaponData.MaxRange;
	}

	// Perform line trace
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(HitResult, AimLoc, AimLoc + AimDir * MaxRange, ECC_Visibility, QueryParams))
	{
		return HitResult.ImpactPoint;
	}

	return AimLoc + AimDir * MaxRange;
}

void UHarmoniaRangedCombatComponent::CalculateTrajectoryPath(TArray<FVector>& OutPathPositions, float TimeStep, float MaxSimTime)
{
	OutPathPositions.Empty();

	FHarmoniaRangedWeaponData WeaponData;
	if (!GetCurrentWeaponData(WeaponData))
	{
		return;
	}

	FVector StartLocation = GetAimLocation();
	FVector LaunchVelocity = GetAimDirection() * 3000.0f; // Default speed

	// For ballistic projectiles, calculate trajectory with gravity
	bool bUseBallistic = false;

	// Simulate trajectory
	FVector CurrentPos = StartLocation;
	FVector CurrentVel = LaunchVelocity;
	float CurrentTime = 0.0f;

	const FVector Gravity = FVector(0, 0, -980.0f); // Default gravity

	while (CurrentTime < MaxSimTime && OutPathPositions.Num() < 100)
	{
		OutPathPositions.Add(CurrentPos);

		// Check for collision
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwner());

		FVector NextPos = CurrentPos + CurrentVel * TimeStep;
		if (bUseBallistic)
		{
			NextPos += 0.5f * Gravity * TimeStep * TimeStep;
		}

		if (GetWorld()->LineTraceSingleByChannel(HitResult, CurrentPos, NextPos, ECC_Visibility, QueryParams))
		{
			OutPathPositions.Add(HitResult.ImpactPoint);
			break;
		}

		CurrentPos = NextPos;
		if (bUseBallistic)
		{
			CurrentVel += Gravity * TimeStep;
		}
		CurrentTime += TimeStep;
	}
}

// ============================================================================
// Firing
// ============================================================================

bool UHarmoniaRangedCombatComponent::RequestFire()
{
	if (!CanFire())
	{
		return false;
	}

	FHarmoniaRangedWeaponData WeaponData;
	if (!GetCurrentWeaponData(WeaponData))
	{
		return false;
	}

	// For bows, start drawing
	if (WeaponData.WeaponType == EHarmoniaRangedWeaponType::Bow ||
		WeaponData.WeaponType == EHarmoniaRangedWeaponType::Longbow ||
		WeaponData.WeaponType == EHarmoniaRangedWeaponType::Shortbow ||
		WeaponData.WeaponType == EHarmoniaRangedWeaponType::CompositeBow)
	{
		bIsDrawing = true;
		CurrentDrawTime = 0.0f;

		if (GetOwnerRole() < ROLE_Authority)
		{
			ServerStartDrawing();
		}

		return true;
	}

	// For other weapons, fire immediately
	FireProjectile();
	return true;
}

bool UHarmoniaRangedCombatComponent::RequestRelease()
{
	if (!bIsDrawing)
	{
		return false;
	}

	bIsDrawing = false;

	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerReleaseDrawing();
	}

	// Fire the projectile
	FireProjectile();
	return true;
}

bool UHarmoniaRangedCombatComponent::CanFire() const
{
	if (!bWeaponDrawn || bIsReloading)
	{
		return false;
	}

	// Check ammo
	if (!HasAmmo())
	{
		return false;
	}

	// Check fire rate
	FHarmoniaRangedWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData) && WeaponData.FireRate > 0.0f)
	{
		float TimeSinceLastFire = GetWorld()->GetTimeSeconds() - LastFireTime;
		float MinFireInterval = 1.0f / WeaponData.FireRate;
		if (TimeSinceLastFire < MinFireInterval)
		{
			return false;
		}
	}

	return true;
}

float UHarmoniaRangedCombatComponent::GetDrawProgress() const
{
	FHarmoniaRangedWeaponData WeaponData;
	if (!GetCurrentWeaponData(WeaponData) || WeaponData.DrawTime <= 0.0f)
	{
		return 1.0f;
	}

	return FMath::Clamp(CurrentDrawTime / WeaponData.DrawTime, 0.0f, 1.0f);
}

float UHarmoniaRangedCombatComponent::GetDrawDamageMultiplier() const
{
	FHarmoniaRangedWeaponData WeaponData;
	if (!GetCurrentWeaponData(WeaponData))
	{
		return 1.0f;
	}

	float DrawProgress = GetDrawProgress();
	return FMath::Lerp(1.0f, WeaponData.MaxDrawDamageMultiplier, DrawProgress);
}

void UHarmoniaRangedCombatComponent::RequestReload()
{
	if (bIsReloading || !bWeaponDrawn)
	{
		return;
	}

	FHarmoniaRangedWeaponData WeaponData;
	if (!GetCurrentWeaponData(WeaponData) || WeaponData.ReloadTime <= 0.0f)
	{
		return;
	}

	// Check if we have ammo to reload
	if (TotalAmmo <= 0 && WeaponData.MaxAmmo > 0)
	{
		return;
	}

	bIsReloading = true;
	ReloadTimeRemaining = WeaponData.ReloadTime;
}

void UHarmoniaRangedCombatComponent::FireProjectile()
{
	if (!ConsumeResources())
	{
		return;
	}

	FHarmoniaRangedWeaponData WeaponData;
	if (!GetCurrentWeaponData(WeaponData))
	{
		return;
	}

	FVector SpawnLocation = GetAimLocation();
	FVector Direction = GetAimDirection();
	float DamageMultiplier = GetDrawDamageMultiplier();

	// Apply spread
	if (WeaponData.SpreadAngle > 0.0f)
	{
		float SpreadRadians = FMath::DegreesToRadians(WeaponData.SpreadAngle);
		FVector Spread = FMath::VRandCone(Direction, SpreadRadians);
		Direction = Spread.GetSafeNormal();
	}

	LastFireTime = GetWorld()->GetTimeSeconds();

	// Spawn multiple projectiles if needed (shotgun)
	for (int32 i = 0; i < WeaponData.ProjectilesPerShot; ++i)
	{
		FVector SpreadDirection = Direction;
		if (i > 0 && WeaponData.SpreadAngle > 0.0f)
		{
			float SpreadRadians = FMath::DegreesToRadians(WeaponData.SpreadAngle);
			SpreadDirection = FMath::VRandCone(Direction, SpreadRadians).GetSafeNormal();
		}

		// For now, create simple projectile data
		FHarmoniaProjectileData ProjectileData;
		ProjectileData.ProjectileType = WeaponData.DefaultProjectileType;
		ProjectileData.InitialSpeed = 3000.0f;
		ProjectileData.DamageConfig.BaseDamage = 25.0f * DamageMultiplier * WeaponData.BaseDamageMultiplier;

		AActor* Projectile = SpawnProjectile(ProjectileData, SpawnLocation, SpreadDirection, DamageMultiplier);

		if (Projectile)
		{
			OnProjectileFired.Broadcast(Projectile, ProjectileData);
		}
	}

	// Apply recoil
	ApplyRecoil();

	// Reset draw time
	CurrentDrawTime = 0.0f;
}

void UHarmoniaRangedCombatComponent::PerformReload()
{
	bIsReloading = false;

	FHarmoniaRangedWeaponData WeaponData;
	if (!GetCurrentWeaponData(WeaponData))
	{
		return;
	}

	// Calculate how much ammo to reload
	int32 AmmoNeeded = WeaponData.MagazineSize - CurrentAmmo;
	int32 AmmoToReload = FMath::Min(AmmoNeeded, TotalAmmo);

	CurrentAmmo += AmmoToReload;
	TotalAmmo -= AmmoToReload;

	OnAmmoChanged.Broadcast(CurrentAmmo, WeaponData.MagazineSize);
}

void UHarmoniaRangedCombatComponent::ApplyRecoil()
{
	// TODO: Implement camera recoil
}

bool UHarmoniaRangedCombatComponent::ConsumeResources()
{
	FHarmoniaRangedWeaponData WeaponData;
	if (!GetCurrentWeaponData(WeaponData))
	{
		return false;
	}

	// Consume ammo
	if (WeaponData.MagazineSize > 0)
	{
		if (CurrentAmmo <= 0)
		{
			return false;
		}
		CurrentAmmo--;
		OnAmmoChanged.Broadcast(CurrentAmmo, WeaponData.MagazineSize);
	}

	// Consume stamina
	if (WeaponData.StaminaCostPerShot > 0.0f)
	{
		if (!ConsumeStamina(WeaponData.StaminaCostPerShot))
		{
			return false;
		}
	}

	// Consume mana (for magic weapons)
	if (WeaponData.ManaCostPerShot > 0.0f)
	{
		if (!ConsumeMana(WeaponData.ManaCostPerShot))
		{
			return false;
		}
	}

	return true;
}

// ============================================================================
// Ammo Management
// ============================================================================

int32 UHarmoniaRangedCombatComponent::GetMagazineSize() const
{
	FHarmoniaRangedWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		return WeaponData.MagazineSize;
	}
	return 0;
}

void UHarmoniaRangedCombatComponent::AddAmmo(int32 Amount)
{
	FHarmoniaRangedWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		if (WeaponData.MaxAmmo > 0)
		{
			TotalAmmo = FMath::Min(TotalAmmo + Amount, WeaponData.MaxAmmo);
		}
		else
		{
			TotalAmmo += Amount;
		}
		OnAmmoChanged.Broadcast(CurrentAmmo, WeaponData.MagazineSize);
	}
}

void UHarmoniaRangedCombatComponent::SetAmmo(int32 InCurrentAmmo, int32 InTotalAmmo)
{
	CurrentAmmo = InCurrentAmmo;
	TotalAmmo = InTotalAmmo;

	FHarmoniaRangedWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		OnAmmoChanged.Broadcast(CurrentAmmo, WeaponData.MagazineSize);
	}
}

bool UHarmoniaRangedCombatComponent::HasAmmo() const
{
	FHarmoniaRangedWeaponData WeaponData;
	if (!GetCurrentWeaponData(WeaponData))
	{
		return false;
	}

	// Infinite ammo
	if (WeaponData.MagazineSize == 0)
	{
		return true;
	}

	return CurrentAmmo > 0;
}

// ============================================================================
// Spell Management
// ============================================================================

void UHarmoniaRangedCombatComponent::SetCurrentSpell(FName SpellRowName)
{
	CurrentSpellRowName = SpellRowName;
}

bool UHarmoniaRangedCombatComponent::GetCurrentSpellData(FHarmoniaSpellData& OutSpellData) const
{
	if (!SpellDataTable || CurrentSpellRowName.IsNone())
	{
		return false;
	}

	const FString ContextString = TEXT("GetCurrentSpellData");
	FHarmoniaSpellData* SpellData = SpellDataTable->FindRow<FHarmoniaSpellData>(CurrentSpellRowName, ContextString);
	if (SpellData)
	{
		OutSpellData = *SpellData;
		return true;
	}

	return false;
}

bool UHarmoniaRangedCombatComponent::CanCastSpell() const
{
	FHarmoniaSpellData SpellData;
	if (!GetCurrentSpellData(SpellData))
	{
		return false;
	}

	// Check cooldown
	if (GetSpellCooldownRemaining() > 0.0f)
	{
		return false;
	}

	// Check mana
	if (!HasEnoughMana(SpellData.ManaCost))
	{
		return false;
	}

	return true;
}

float UHarmoniaRangedCombatComponent::GetSpellCooldownRemaining() const
{
	FHarmoniaSpellData SpellData;
	if (!GetCurrentSpellData(SpellData))
	{
		return 0.0f;
	}

	float TimeSinceLastCast = GetWorld()->GetTimeSeconds() - LastSpellCastTime;
	float CooldownRemaining = SpellData.Cooldown - TimeSinceLastCast;
	return FMath::Max(CooldownRemaining, 0.0f);
}

// ============================================================================
// Projectile Spawning
// ============================================================================

AActor* UHarmoniaRangedCombatComponent::SpawnProjectile(const FHarmoniaProjectileData& ProjectileData, const FVector& SpawnLocation, const FVector& Direction, float DamageMultiplier)
{
	if (!GetWorld())
	{
		return nullptr;
	}

	// For now, spawn a simple projectile actor
	// TODO: Implement full projectile system

	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerFireProjectile(SpawnLocation, Direction, DamageMultiplier);
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn projectile
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetRotation(Direction.ToOrientationQuat());

	// Use default projectile class if available
	if (DefaultProjectileClass)
	{
		AActor* Projectile = GetWorld()->SpawnActor<AActor>(DefaultProjectileClass, SpawnTransform, SpawnParams);
		return Projectile;
	}

	return nullptr;
}

// ============================================================================
// Helper Functions
// ============================================================================



UHarmoniaLockOnComponent* UHarmoniaRangedCombatComponent::GetLockOnComponent() const
{
	if (CachedLockOnComponent)
	{
		return CachedLockOnComponent;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	UHarmoniaLockOnComponent* LockOn = Owner->FindComponentByClass<UHarmoniaLockOnComponent>();
	// Cache it using const_cast since this is a lazy initialization pattern
	const_cast<UHarmoniaRangedCombatComponent*>(this)->CachedLockOnComponent = LockOn;
	return LockOn;
}

// ============================================================================
// Server RPCs
// ============================================================================

void UHarmoniaRangedCombatComponent::ServerFireProjectile_Implementation(const FVector& SpawnLocation, const FVector& Direction, float DamageMultiplier)
{
	FireProjectile();
}

bool UHarmoniaRangedCombatComponent::ServerFireProjectile_Validate(const FVector& SpawnLocation, const FVector& Direction, float DamageMultiplier)
{
	// [ANTI-CHEAT] Validate projectile fire request
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	// Validate spawn location is near the player
	const float MaxSpawnDistance = 500.0f;
	float Distance = FVector::Dist(Owner->GetActorLocation(), SpawnLocation);
	if (Distance > MaxSpawnDistance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerFireProjectile: Spawn location too far from player (%.1f > %.1f)"),
			Distance, MaxSpawnDistance);
		return false;
	}

	// Validate damage multiplier is within reasonable bounds
	const float MaxDamageMultiplier = 5.0f;
	if (DamageMultiplier < 0.0f || DamageMultiplier > MaxDamageMultiplier)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerFireProjectile: Invalid damage multiplier (%.2f)"),
			DamageMultiplier);
		return false;
	}

	// Validate direction is normalized
	if (!Direction.IsNormalized())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerFireProjectile: Direction not normalized"));
		return false;
	}

	// Validate player has ammo (if applicable - weapon uses ammo if MagazineSize > 0)
	FHarmoniaRangedWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		if (WeaponData.MagazineSize > 0 && CurrentAmmo <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ANTI-CHEAT] ServerFireProjectile: No ammo"));
			return false;
		}
	}

	return true;
}

void UHarmoniaRangedCombatComponent::ServerStartAiming_Implementation()
{
	bIsAiming = true;
}

bool UHarmoniaRangedCombatComponent::ServerStartAiming_Validate()
{
	// Basic validation - allow start aiming requests
	return true;
}

void UHarmoniaRangedCombatComponent::ServerStopAiming_Implementation()
{
	bIsAiming = false;
}

bool UHarmoniaRangedCombatComponent::ServerStopAiming_Validate()
{
	// Basic validation - allow stop aiming requests
	return true;
}

void UHarmoniaRangedCombatComponent::ServerStartDrawing_Implementation()
{
	bIsDrawing = true;
	CurrentDrawTime = 0.0f;
}

bool UHarmoniaRangedCombatComponent::ServerStartDrawing_Validate()
{
	// Validate player can draw (has bow/etc equipped)
	return true;
}

void UHarmoniaRangedCombatComponent::ServerReleaseDrawing_Implementation()
{
	bIsDrawing = false;
}

bool UHarmoniaRangedCombatComponent::ServerReleaseDrawing_Validate()
{
	// Validate player was drawing
	return true;
}

// ============================================================================
// Replication Callbacks
// ============================================================================

void UHarmoniaRangedCombatComponent::OnRep_CurrentWeaponType()
{
	// Update visuals, animations, etc.
}

void UHarmoniaRangedCombatComponent::OnRep_IsAiming()
{
	OnAimingStateChanged.Broadcast(bIsAiming);
}

void UHarmoniaRangedCombatComponent::OnRep_CurrentAmmo()
{
	FHarmoniaRangedWeaponData WeaponData;
	if (GetCurrentWeaponData(WeaponData))
	{
		OnAmmoChanged.Broadcast(CurrentAmmo, WeaponData.MagazineSize);
	}
}
