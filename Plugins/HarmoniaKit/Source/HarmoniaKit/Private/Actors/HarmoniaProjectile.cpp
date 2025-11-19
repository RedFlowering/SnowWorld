// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/HarmoniaAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"

AHarmoniaProjectile::AHarmoniaProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	// Create collision component
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(10.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = CollisionComponent;

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create movement component
	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
	MovementComponent->InitialSpeed = 3000.0f;
	MovementComponent->MaxSpeed = 3000.0f;
	MovementComponent->bRotationFollowsVelocity = true;
	MovementComponent->bShouldBounce = false;
	MovementComponent->ProjectileGravityScale = 0.0f;

	// Create trail component
	TrailComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailComponent"));
	TrailComponent->SetupAttachment(RootComponent);
	TrailComponent->bAutoActivate = true;

	// Create audio component
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->bAutoActivate = false;

	// Bind collision events
	CollisionComponent->OnComponentHit.AddDynamic(this, &AHarmoniaProjectile::OnProjectileHit_Internal);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AHarmoniaProjectile::OnProjectileOverlap);
}

void AHarmoniaProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Set up lifetime timer
	if (ProjectileData.Lifetime > 0.0f)
	{
		GetWorldTimerManager().SetTimer(LifetimeTimerHandle, this, &AHarmoniaProjectile::DestroyProjectile, ProjectileData.Lifetime, false);
	}
}

void AHarmoniaProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update homing
	if (HomingTarget.IsValid() && ProjectileData.MovementType == EHarmoniaProjectileMovement::Homing)
	{
		MovementComponent->bIsHomingProjectile = true;
		MovementComponent->HomingTargetComponent = HomingTarget->GetRootComponent();
		MovementComponent->HomingAccelerationMagnitude = ProjectileData.HomingAcceleration;
	}
}

void AHarmoniaProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHarmoniaProjectile, ProjectileData);
	DOREPLIFETIME(AHarmoniaProjectile, DamageMultiplier);
}

// ============================================================================
// Initialization
// ============================================================================

void AHarmoniaProjectile::InitializeProjectile(const FHarmoniaProjectileData& InProjectileData, AActor* InOwner, float InDamageMultiplier)
{
	ProjectileData = InProjectileData;
	ProjectileOwner = InOwner;
	DamageMultiplier = InDamageMultiplier;

	// Set up penetration
	RemainingPenetrations = ProjectileData.PenetrationCount;
	RemainingBounces = ProjectileData.BounceCount;

	// Configure movement
	switch (ProjectileData.MovementType)
	{
	case EHarmoniaProjectileMovement::Ballistic:
		MovementComponent->ProjectileGravityScale = ProjectileData.GravityScale;
		break;

	case EHarmoniaProjectileMovement::Straight:
		MovementComponent->ProjectileGravityScale = 0.0f;
		break;

	case EHarmoniaProjectileMovement::Homing:
		MovementComponent->ProjectileGravityScale = 0.0f;
		MovementComponent->bIsHomingProjectile = true;
		break;

	case EHarmoniaProjectileMovement::Parabolic:
		MovementComponent->ProjectileGravityScale = ProjectileData.GravityScale * 0.5f;
		break;

	case EHarmoniaProjectileMovement::Boomerang:
		// TODO: Implement boomerang behavior
		break;

	case EHarmoniaProjectileMovement::Hitscan:
		// Hitscan should be handled differently, not as a projectile
		break;
	}

	// Set mesh
	if (ProjectileData.ProjectileMesh)
	{
		MeshComponent->SetStaticMesh(ProjectileData.ProjectileMesh);
	}

	// Set trail effect
	if (ProjectileData.TrailEffect)
	{
		TrailComponent->SetTemplate(ProjectileData.TrailEffect);
	}

	// Set speed
	if (ProjectileData.InitialSpeed > 0.0f)
	{
		MovementComponent->InitialSpeed = ProjectileData.InitialSpeed;
		MovementComponent->MaxSpeed = ProjectileData.MaxSpeed > 0.0f ? ProjectileData.MaxSpeed : ProjectileData.InitialSpeed;
	}

	// Ignore owner collision
	if (ProjectileOwner)
	{
		CollisionComponent->IgnoreActorWhenMoving(ProjectileOwner, true);
	}
}

void AHarmoniaProjectile::LaunchProjectile(const FVector& Direction, float Speed)
{
	if (Speed > 0.0f)
	{
		MovementComponent->InitialSpeed = Speed;
		MovementComponent->MaxSpeed = Speed;
	}

	MovementComponent->Velocity = Direction.GetSafeNormal() * MovementComponent->InitialSpeed;
	SetActorRotation(Direction.Rotation());
}

void AHarmoniaProjectile::SetHomingTarget(AActor* Target)
{
	HomingTarget = Target;
}

// ============================================================================
// Collision Handling
// ============================================================================

void AHarmoniaProjectile::OnProjectileHit_Internal(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!OtherActor || OtherActor == ProjectileOwner)
	{
		return;
	}

	// Check if we already hit this actor
	if (HitActors.Contains(OtherActor))
	{
		return;
	}

	// Broadcast hit event
	OnProjectileHit.Broadcast(OtherActor, Hit, ProjectileData);

	// Apply damage
	ApplyDamageToTarget(OtherActor, Hit);

	// Add to hit actors list
	HitActors.Add(OtherActor);

	// Spawn impact effects
	SpawnImpactEffects(Hit);

	// Handle penetration
	if (HandlePenetration(OtherActor))
	{
		return; // Continue through target
	}

	// Handle bouncing
	if (RemainingBounces > 0)
	{
		HandleBounce(Hit);
		RemainingBounces--;
		return;
	}

	// Handle sticking
	if (ProjectileData.bStickToSurfaces)
	{
		StickToSurface(Hit);
		return;
	}

	// Handle explosion
	if (ProjectileData.bExplodeOnImpact)
	{
		Explode(Hit.ImpactPoint);
		return;
	}

	// Default: destroy projectile
	DestroyProjectile();
}

void AHarmoniaProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Handle overlap-based collision (for trigger-type projectiles)
	// This can be used for proximity detonation, etc.
}

void AHarmoniaProjectile::ApplyDamageToTarget(AActor* Target, const FHitResult& HitResult)
{
	if (!Target || !HasAuthority())
	{
		return;
	}

	// Get ability system component from owner
	UAbilitySystemComponent* OwnerASC = GetOwnerAbilitySystemComponent();
	if (!OwnerASC)
	{
		return;
	}

	// Get target ability system component
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
	if (!TargetASC)
	{
		return;
	}

	// Calculate final damage
	float FinalDamage = ProjectileData.DamageConfig.BaseDamage * ProjectileData.DamageConfig.DamageMultiplier * DamageMultiplier;

	// Apply penetration damage falloff
	int32 PenetrationsUsed = ProjectileData.PenetrationCount - RemainingPenetrations;
	if (PenetrationsUsed > 0)
	{
		FinalDamage *= FMath::Pow(ProjectileData.PenetrationDamageFalloff, PenetrationsUsed);
	}

	// Create gameplay effect context
	FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	EffectContext.AddInstigator(ProjectileOwner, ProjectileOwner);
	EffectContext.AddHitResult(HitResult);

	// Apply damage via gameplay effect
	if (ProjectileData.DamageConfig.DamageEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(ProjectileData.DamageConfig.DamageEffectClass, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			// Set damage magnitude
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), FinalDamage);

			// Apply effect to target
			OwnerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}
	}

	// Apply additional effects
	for (const TSubclassOf<UGameplayEffect>& AdditionalEffect : ProjectileData.DamageConfig.AdditionalEffects)
	{
		if (AdditionalEffect)
		{
			FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(AdditionalEffect, 1.0f, EffectContext);
			if (SpecHandle.IsValid())
			{
				OwnerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}

	// Trigger gameplay cue
	if (ProjectileData.ImpactGameplayCueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = HitResult.ImpactPoint;
		CueParams.Normal = HitResult.ImpactNormal;
		CueParams.PhysicalMaterial = HitResult.PhysMaterial;
		CueParams.Instigator = ProjectileOwner;
		CueParams.EffectCauser = this;

		TargetASC->ExecuteGameplayCue(ProjectileData.ImpactGameplayCueTag, CueParams);
	}
}

bool AHarmoniaProjectile::HandlePenetration(AActor* HitActor)
{
	if (RemainingPenetrations <= 0)
	{
		return false;
	}

	RemainingPenetrations--;
	return true; // Continue through target
}

void AHarmoniaProjectile::HandleBounce(const FHitResult& HitResult)
{
	// Calculate bounce direction
	FVector IncomingDirection = MovementComponent->Velocity.GetSafeNormal();
	FVector BounceDirection = IncomingDirection - 2.0f * (IncomingDirection | HitResult.ImpactNormal) * HitResult.ImpactNormal;

	// Apply velocity retention
	float BounceSpeed = MovementComponent->Velocity.Size() * ProjectileData.BounceVelocityRetention;
	MovementComponent->Velocity = BounceDirection * BounceSpeed;

	// Update rotation
	SetActorRotation(BounceDirection.Rotation());

	// Spawn impact effects
	SpawnImpactEffects(HitResult);
}

void AHarmoniaProjectile::Explode(const FVector& ExplosionLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	// Broadcast explode event
	OnProjectileExplode.Broadcast(ExplosionLocation);

	// Apply explosion damage
	if (ProjectileData.ExplosionRadius > 0.0f)
	{
		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		if (ProjectileOwner)
		{
			QueryParams.AddIgnoredActor(ProjectileOwner);
		}

		GetWorld()->OverlapMultiByChannel(
			OverlapResults,
			ExplosionLocation,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(ProjectileData.ExplosionRadius),
			QueryParams
		);

		for (const FOverlapResult& Overlap : OverlapResults)
		{
			if (AActor* HitActor = Overlap.GetActor())
			{
				// Calculate distance falloff
				float Distance = FVector::Dist(ExplosionLocation, HitActor->GetActorLocation());
				float DamageFalloff = 1.0f - FMath::Pow(Distance / ProjectileData.ExplosionRadius, ProjectileData.DamageConfig.ExplosionFalloff);
				DamageFalloff = FMath::Clamp(DamageFalloff, 0.0f, 1.0f);

				// Apply damage
				FHitResult HitResult;
				HitResult.ImpactPoint = HitActor->GetActorLocation();
				HitResult.Location = HitActor->GetActorLocation();
				HitResult.Actor = HitActor;

				// Temporarily modify damage for falloff
				float OriginalDamage = ProjectileData.DamageConfig.BaseDamage;
				ProjectileData.DamageConfig.BaseDamage *= DamageFalloff;
				ApplyDamageToTarget(HitActor, HitResult);
				ProjectileData.DamageConfig.BaseDamage = OriginalDamage;
			}
		}
	}

	// Spawn explosion effects
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, ExplosionLocation);
	}

	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, ExplosionLocation);
	}

	// Destroy projectile
	DestroyProjectile();
}

void AHarmoniaProjectile::StickToSurface(const FHitResult& HitResult)
{
	// Stop movement
	MovementComponent->StopMovingImmediately();
	MovementComponent->SetActive(false);

	// Attach to hit actor if possible
	if (AActor* HitActor = HitResult.GetActor())
	{
		AttachToActor(HitActor, FAttachmentTransformRules::KeepWorldTransform);
	}

	// Set stuck flag
	bIsStuck = true;

	// Disable collision
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Play impact effects
	SpawnImpactEffects(HitResult);

	// Optional: Set timer to destroy after some time
	GetWorldTimerManager().SetTimer(LifetimeTimerHandle, this, &AHarmoniaProjectile::DestroyProjectile, 10.0f, false);
}

void AHarmoniaProjectile::DestroyProjectile()
{
	Destroy();
}

void AHarmoniaProjectile::SpawnImpactEffects(const FHitResult& HitResult)
{
	// Spawn impact particle effect
	UParticleSystem* EffectToSpawn = ProjectileData.ImpactEffect ? ProjectileData.ImpactEffect : ImpactEffect;
	if (EffectToSpawn)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EffectToSpawn, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
	}

	// Play impact sound
	USoundBase* SoundToPlay = ProjectileData.ImpactSound ? ProjectileData.ImpactSound : ImpactSound;
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToPlay, HitResult.ImpactPoint);
	}
}

UAbilitySystemComponent* AHarmoniaProjectile::GetOwnerAbilitySystemComponent() const
{
	if (!ProjectileOwner)
	{
		return nullptr;
	}

	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ProjectileOwner);
}
