// Copyright 2025 Snow Game Studio.

#include "Actors/HarmoniaSenseInteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/HarmoniaSenseInteractableComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

AHarmoniaSenseInteractableActor::AHarmoniaSenseInteractableActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// Create mesh component
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);

	// Create sense interactable component
	InteractableComponent = CreateDefaultSubobject<UHarmoniaSenseInteractableComponent>(TEXT("InteractableComponent"));

	// Create widget component
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(RootComponent);
	WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetDrawSize(FVector2D(200.0f, 50.0f));
	WidgetComponent->SetVisibility(false);
}

void AHarmoniaSenseInteractableActor::BeginPlay()
{
	Super::BeginPlay();

	SetupInteractionBindings();

	// Set initial visual state
	SetVisualState(EInteractableVisualState::Idle);
}

void AHarmoniaSenseInteractableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ============================================================================
// Visual State Functions
// ============================================================================

void AHarmoniaSenseInteractableActor::SetVisualState(EInteractableVisualState State)
{
	if (!bEnableVisualFeedback || !MeshComponent)
	{
		return;
	}

	UMaterialInterface* MaterialToUse = nullptr;

	switch (State)
	{
	case EInteractableVisualState::Idle:
		MaterialToUse = IdleMaterial;
		break;

	case EInteractableVisualState::Highlighted:
		MaterialToUse = HighlightMaterial;
		break;

	case EInteractableVisualState::Active:
		MaterialToUse = ActiveMaterial;
		break;

	case EInteractableVisualState::Disabled:
		MaterialToUse = DisabledMaterial;
		break;
	}

	if (MaterialToUse)
	{
		MeshComponent->SetMaterial(0, MaterialToUse);
	}
}

void AHarmoniaSenseInteractableActor::SetWidgetVisible(bool bVisible)
{
	if (!bEnableWidgetControl || !WidgetComponent)
	{
		return;
	}

	WidgetComponent->SetVisibility(bVisible);
}

// ============================================================================
// Interaction Events
// ============================================================================

void AHarmoniaSenseInteractableActor::OnSensedByInteractor_Implementation(AActor* Interactor, FName SensorTag)
{
	SensingActorCount++;

	// Visual feedback
	SetVisualState(EInteractableVisualState::Highlighted);
	SetWidgetVisible(true);

	// Play sound
	if (SensedSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SensedSound, GetActorLocation());
	}
}

void AHarmoniaSenseInteractableActor::OnLostFromInteractor_Implementation(AActor* Interactor, FName SensorTag)
{
	SensingActorCount--;

	if (SensingActorCount <= 0)
	{
		SensingActorCount = 0;

		// Visual feedback
		SetVisualState(EInteractableVisualState::Idle);
		SetWidgetVisible(false);
	}
}

void AHarmoniaSenseInteractableActor::OnInteractionTriggered_Implementation(AActor* Interactor, FName SensorTag)
{
	// Visual feedback
	SetVisualState(EInteractableVisualState::Active);

	// Play sound
	if (InteractionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, InteractionSound, GetActorLocation());
	}

	// Handle post-interaction behavior
	if (bDestroyAfterInteraction)
	{
		// Destroy actor after a short delay (to allow effects to play)
		SetLifeSpan(0.5f);
	}
	else if (bDisableAfterInteraction)
	{
		DisableInteractable();

		// Schedule respawn if configured
		if (RespawnTime > 0.0f)
		{
			GetWorldTimerManager().SetTimer(
				RespawnTimerHandle,
				this,
				&AHarmoniaSenseInteractableActor::RespawnInteractable,
				RespawnTime,
				false);
		}
	}
}

// ============================================================================
// Protected Functions
// ============================================================================

void AHarmoniaSenseInteractableActor::SetupInteractionBindings()
{
	if (!InteractableComponent)
	{
		return;
	}

	// Bind to interaction component events
	InteractableComponent->OnSensed.AddDynamic(this, &AHarmoniaSenseInteractableActor::Internal_OnSensed);
	InteractableComponent->OnLostSense.AddDynamic(this, &AHarmoniaSenseInteractableActor::Internal_OnLostSense);
	InteractableComponent->OnSenseInteractionTriggered.AddDynamic(this, &AHarmoniaSenseInteractableActor::Internal_OnInteraction);
}

void AHarmoniaSenseInteractableActor::RespawnInteractable()
{
	EnableInteractable();
	SetVisualState(EInteractableVisualState::Idle);
}

void AHarmoniaSenseInteractableActor::DisableInteractable()
{
	if (InteractableComponent)
	{
		InteractableComponent->bIsActive = false;
		InteractableComponent->SetAllInteractionsEnabled(false);
	}

	SetVisualState(EInteractableVisualState::Disabled);
	SetWidgetVisible(false);
}

void AHarmoniaSenseInteractableActor::EnableInteractable()
{
	if (InteractableComponent)
	{
		InteractableComponent->bIsActive = true;
		InteractableComponent->SetAllInteractionsEnabled(true);
		InteractableComponent->ResetAllInteractionData();
	}

	SetVisualState(EInteractableVisualState::Idle);
}

// ============================================================================
// Internal Callbacks
// ============================================================================

void AHarmoniaSenseInteractableActor::Internal_OnSensed(
	AActor* Interactor,
	UHarmoniaSenseInteractableComponent* Component,
	FName SensorTag)
{
	OnSensedByInteractor(Interactor, SensorTag);
}

void AHarmoniaSenseInteractableActor::Internal_OnLostSense(
	AActor* Interactor,
	UHarmoniaSenseInteractableComponent* Component,
	FName SensorTag)
{
	OnLostFromInteractor(Interactor, SensorTag);
}

void AHarmoniaSenseInteractableActor::Internal_OnInteraction(
	AActor* Interactor,
	UHarmoniaSenseInteractableComponent* Component,
	FName SensorTag)
{
	OnInteractionTriggered(Interactor, SensorTag);
}
