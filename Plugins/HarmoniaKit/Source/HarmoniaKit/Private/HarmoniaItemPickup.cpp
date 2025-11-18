// Copyright 2025 Snow Game Studio.

#include "HarmoniaItemPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "SenseStimulusComponent.h"
#include "Components/HarmoniaInteractionComponent.h"
#include "Components/HarmoniaInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AHarmoniaItemPickup::AHarmoniaItemPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Network replication
	bReplicates = true;
	SetReplicateMovement(true);

	// Create root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	MeshComponent->SetGenerateOverlapEvents(true);

	// Create sense stimulus component for detection
	SenseStimulusComponent = CreateDefaultSubobject<USenseStimulusComponent>(TEXT("SenseStimulusComponent"));

	// Create interactable component
	InteractableComponent = CreateDefaultSubobject<UHarmoniaInteractionComponent>(TEXT("InteractableComponent"));

	// Create widget component for displaying item info
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetDrawAtDesiredSize(true);
	WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
}

void AHarmoniaItemPickup::BeginPlay()
{
	Super::BeginPlay();

	// Apply visual effects
	ApplyVisualEffects();

	// Set up lifetime timer if configured
	if (HasAuthority() && LifeTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			LifetimeTimerHandle,
			this,
			&AHarmoniaItemPickup::OnLifetimeExpired,
			LifeTime,
			false
		);
	}
}

void AHarmoniaItemPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSinceSpawn += DeltaTime;

	// Apply floating and rotation effects
	if (MeshComponent)
	{
		// Floating effect
		float FloatOffset = FMath::Sin(TimeSinceSpawn * 2.0f) * 10.0f;
		FVector NewLocation = MeshComponent->GetRelativeLocation();
		NewLocation.Z = FloatOffset;
		MeshComponent->SetRelativeLocation(NewLocation);

		// Rotation effect
		FRotator NewRotation = MeshComponent->GetRelativeRotation();
		NewRotation.Yaw += DeltaTime * 60.0f;
		MeshComponent->SetRelativeRotation(NewRotation);
	}

	// Check for auto-pickup
	if (HasAuthority() && bAutoPickup && !bCollected)
	{
		CheckAutoPickup();
	}
}

void AHarmoniaItemPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHarmoniaItemPickup, LootItem);
	DOREPLIFETIME(AHarmoniaItemPickup, Quantity);
	DOREPLIFETIME(AHarmoniaItemPickup, GoldAmount);
}

// ============================================================================
// IHarmoniaInteractableInterface Implementation
// ============================================================================

void AHarmoniaItemPickup::OnInteract_Implementation(const FHarmoniaInteractionContext& Context, FHarmoniaInteractionResult& OutResult)
{
	OutResult.bSuccess = false;

	// Check if already collected
	if (bCollected)
	{
		return;
	}

	// Check if actor has inventory component
	AActor* InteractingActor = Context.Interactor;
	if (!InteractingActor)
	{
		return;
	}

	UHarmoniaInventoryComponent* InventoryComponent = InteractingActor->FindComponentByClass<UHarmoniaInventoryComponent>();
	if (!InventoryComponent)
	{
		return;
	}

	// Try pickup
	if (HasAuthority())
	{
		OutResult.bSuccess = TryPickup(InteractingActor);
	}
}

// ============================================================================
// Public Functions
// ============================================================================

void AHarmoniaItemPickup::InitializePickup(const FHarmoniaLootTableRow& InLootItem, int32 InQuantity)
{
	LootItem = InLootItem;
	Quantity = FMath::Max(1, InQuantity);
	GoldAmount = 0;

	// Set mesh if ItemMesh was provided
	if (ItemMesh && MeshComponent)
	{
		MeshComponent->SetStaticMesh(ItemMesh);
	}

	// Configure sense stimulus based on rarity
	if (SenseStimulusComponent)
	{
		// Higher rarity items have larger detection range
		float DetectionRadius = 200.0f;
		switch (LootItem.Rarity)
		{
		case EHarmoniaLootRarity::Uncommon:
			DetectionRadius = 300.0f;
			break;
		case EHarmoniaLootRarity::Rare:
			DetectionRadius = 400.0f;
			break;
		case EHarmoniaLootRarity::Epic:
			DetectionRadius = 500.0f;
			break;
		case EHarmoniaLootRarity::Legendary:
			DetectionRadius = 600.0f;
			break;
		default:
			break;
		}

		// Set up stimulus
		// TODO: Configure sense stimulus properly once SenseSystem API is confirmed
		// SenseStimulusComponent->RegisterSenseStimulus(FName("Sight"), DetectionRadius);
	}

	UE_LOG(LogTemp, Log, TEXT("Item pickup initialized: %s x%d"), *LootItem.ItemID.ToString(), Quantity);
}

void AHarmoniaItemPickup::InitializeGoldPickup(int32 InGoldAmount)
{
	GoldAmount = FMath::Max(0, InGoldAmount);
	Quantity = 0;
	LootItem = FHarmoniaLootTableRow();

	// Set mesh for gold
	if (ItemMesh && MeshComponent)
	{
		MeshComponent->SetStaticMesh(ItemMesh);
	}

	UE_LOG(LogTemp, Log, TEXT("Gold pickup initialized: %d gold"), GoldAmount);
}

bool AHarmoniaItemPickup::TryPickup(AActor* Collector)
{
	if (!HasAuthority() || bCollected || !Collector)
	{
		return false;
	}

	// Get inventory component
	UHarmoniaInventoryComponent* InventoryComponent = Collector->FindComponentByClass<UHarmoniaInventoryComponent>();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Collector %s has no inventory component"), *Collector->GetName());
		return false;
	}

	bool bSuccess = false;

	// Handle gold pickup
	if (GoldAmount > 0)
	{
		// TODO: Add gold to player's currency
		// For now, just log
		UE_LOG(LogTemp, Log, TEXT("%s picked up %d gold"), *Collector->GetName(), GoldAmount);
		bSuccess = true;
	}
	// Handle item pickup
	else if (LootItem.ItemID != NAME_None)
	{
		// TODO: Implement proper inventory add method once API is confirmed
		// Try to add to inventory
		// bool bAdded = InventoryComponent->AddItemByID(LootItem.ItemID, Quantity);
		bool bAdded = true; // Temporary - always succeed
		if (bAdded)
		{
			UE_LOG(LogTemp, Log, TEXT("%s picked up %s x%d"), *Collector->GetName(), *LootItem.ItemID.ToString(), Quantity);
			bSuccess = true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s inventory full, cannot pick up %s"), *Collector->GetName(), *LootItem.ItemID.ToString());
			bSuccess = false;
		}
	}

	if (bSuccess)
	{
		bCollected = true;

		// Clear lifetime timer
		if (LifetimeTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(LifetimeTimerHandle);
		}

		// Destroy the pickup
		Destroy();
	}

	return bSuccess;
}

FText AHarmoniaItemPickup::GetDisplayName() const
{
	if (GoldAmount > 0)
	{
		return FText::FromString("Gold");
	}
	else if (LootItem.ItemID != NAME_None)
	{
		// Format the ItemID into a readable name
		// Convert "ItemID_Example" to "Item ID Example"
		FString ItemIDString = LootItem.ItemID.ToString();
		ItemIDString = ItemIDString.Replace(TEXT("_"), TEXT(" "));

		// Capitalize first letter
		if (ItemIDString.Len() > 0)
		{
			ItemIDString[0] = FChar::ToUpper(ItemIDString[0]);
		}

		return FText::FromString(ItemIDString);
	}

	return FText::FromString("Unknown Item");
}

// ============================================================================
// Protected Functions
// ============================================================================

void AHarmoniaItemPickup::CheckAutoPickup()
{
	if (!bAutoPickup || bCollected)
	{
		return;
	}

	// Find nearby pawns
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, APawn::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
		if (Distance <= AutoPickupRange)
		{
			if (TryPickup(Actor))
			{
				break;
			}
		}
	}
}

void AHarmoniaItemPickup::OnLifetimeExpired()
{
	UE_LOG(LogTemp, Log, TEXT("Item pickup %s expired after %.1f seconds"), *GetName(), LifeTime);
	Destroy();
}

void AHarmoniaItemPickup::ApplyVisualEffects()
{
	if (!MeshComponent)
	{
		return;
	}

	// Apply material effects based on rarity
	// This can be customized in Blueprint or with material parameters
	// For now, just ensure the mesh is visible
	MeshComponent->SetVisibility(true);
	MeshComponent->SetHiddenInGame(false);
}
