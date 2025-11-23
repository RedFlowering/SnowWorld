// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaComponentUtils.generated.h"

/**
 * Utility functions for common component finding and caching patterns
 * Reduces boilerplate code across HarmoniaKit components
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaComponentUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Find and cache a component on an actor
	 * @param Actor - The actor to search
	 * @param CachedComponent - Reference to cached component pointer (will be set if found)
	 * @return The found component or nullptr
	 */
	template<typename TComponent>
	static TComponent* FindAndCacheComponent(AActor* Actor, TObjectPtr<TComponent>& CachedComponent)
	{
		if (!Actor)
		{
			return nullptr;
		}

		// Return cached if available
		if (CachedComponent)
		{
			return CachedComponent;
		}

		// Find and cache
		CachedComponent = Actor->FindComponentByClass<TComponent>();
		return CachedComponent;
	}

	/**
	 * Find component with validation and logging
	 * @param Actor - The actor to search
	 * @param bLogError - Whether to log an error if not found
	 * @return The found component or nullptr
	 */
	template<typename TComponent>
	static TComponent* FindComponentWithLogging(AActor* Actor, bool bLogError = true)
	{
		if (!Actor)
		{
			if (bLogError)
			{
				UE_LOG(LogHarmoniaKit, Error, TEXT("FindComponent: Null actor provided"));
			}
			return nullptr;
		}

		TComponent* Component = Actor->FindComponentByClass<TComponent>();
		if (!Component && bLogError)
		{
			UE_LOG(LogHarmoniaKit, Error, TEXT("FindComponent: %s not found on actor %s"),
				*TComponent::StaticClass()->GetName(), *Actor->GetName());
		}

		return Component;
	}

	/**
	 * Get or add a component to an actor
	 * @param Actor - The actor
	 * @return The component (existing or newly created)
	 */
	template<typename TComponent>
	static TComponent* GetOrAddComponent(AActor* Actor)
	{
		if (!Actor)
		{
			return nullptr;
		}

		TComponent* Component = Actor->FindComponentByClass<TComponent>();
		if (!Component)
		{
			Component = NewObject<TComponent>(Actor, TComponent::StaticClass());
			if (Component)
			{
				Actor->AddInstanceComponent(Component);
				Component->RegisterComponent();
			}
		}

		return Component;
	}

	/**
	 * Find all components of a type on an actor
	 * @param Actor - The actor to search
	 * @param OutComponents - Array to fill with found components
	 * @return Number of components found
	 */
	template<typename TComponent>
	static int32 FindAllComponents(AActor* Actor, TArray<TComponent*>& OutComponents)
	{
		OutComponents.Empty();

		if (!Actor)
		{
			return 0;
		}

		TArray<UActorComponent*> Components = Actor->GetComponentsByClass(TComponent::StaticClass());
		for (UActorComponent* Component : Components)
		{
			if (TComponent* TypedComponent = Cast<TComponent>(Component))
			{
				OutComponents.Add(TypedComponent);
			}
		}

		return OutComponents.Num();
	}

	/**
	 * Find component on owner's owner (useful for components that need to access player controller components)
	 * @param OwnerActor - The starting actor
	 * @return The found component or nullptr
	 */
	template<typename TComponent>
	static TComponent* FindComponentOnOwner(AActor* OwnerActor)
	{
		if (!OwnerActor)
		{
			return nullptr;
		}

		AActor* Owner = OwnerActor->GetOwner();
		if (!Owner)
		{
			return nullptr;
		}

		return Owner->FindComponentByClass<TComponent>();
	}

	/**
	 * Find component on pawn from controller
	 * @param Controller - The controller
	 * @return The found component on the pawn or nullptr
	 */
	template<typename TComponent>
	static TComponent* FindComponentOnPawn(AController* Controller)
	{
		if (!Controller)
		{
			return nullptr;
		}

		APawn* Pawn = Controller->GetPawn();
		if (!Pawn)
		{
			return nullptr;
		}

		return Pawn->FindComponentByClass<TComponent>();
	}

	/**
	 * Blueprint-accessible version of component finding
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Utils", meta = (DeterminesOutputType = "ComponentClass"))
	static UActorComponent* FindComponentByClass(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass);

	/**
	 * Blueprint-accessible version of component finding with validation
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Utils", meta = (DeterminesOutputType = "ComponentClass"))
	static UActorComponent* FindComponentByClassWithValidation(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass, bool& bFound);
};
