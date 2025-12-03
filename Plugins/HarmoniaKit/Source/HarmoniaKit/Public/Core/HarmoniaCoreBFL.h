// Copyright 2025 Snow Game Studio. All Rights Reserved.

/**
 * @file HarmoniaCoreBFL.h
 * @brief Harmonia Core Blueprint Function Library
 * 
 * Provides common utility functions used globally across the plugin.
 * This library is designed to reduce code duplication and provide safe access patterns.
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaLogCategories.h"
#include "HarmoniaCoreBFL.generated.h"

class UAbilitySystemComponent;
class UGameInstance;
class APlayerController;

/**
 * @class UHarmoniaCoreBFL
 * @brief Harmonia Core Blueprint Function Library
 * 
 * Provides common utility functions used globally:
 * - Server/Client authority checks
 * - Subsystem access helpers
 * - Safe component access
 * - AbilitySystem access
 * - World/GameInstance access
 * 
 * @see UHarmoniaComponentUtils - Component utilities
 * @see UHarmoniaReplicationUtils - Network utilities
 * @see UHarmoniaCombatLibrary - Combat utilities
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaCoreBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================================================================
	// Authority & Server Checks
	// ============================================================================

	/**
	 * Checks if the actor has server authority.
	 * Replaces GetOwner()->HasAuthority() pattern in components.
	 * 
	 * @param Actor Actor to check
	 * @return True if has server authority
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Authority")
	static bool HasServerAuthority(const AActor* Actor);

	/**
	 * Checks if the component's owner has server authority.
	 * Replaces if (!GetOwner() || !GetOwner()->HasAuthority()) pattern.
	 * 
	 * @param Component Component to check
	 * @return True if owner exists and has server authority
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Authority")
	static bool IsComponentServerAuthoritative(const UActorComponent* Component);

	/**
	 * Validates code should only run on server.
	 * Logs warning if not on server.
	 * 
	 * @param Component Component to check
	 * @param FunctionName Function name for logging (debug)
	 * @return True if running on server
	 */
	static bool CheckServerAuthority(const UActorComponent* Component, const FString& FunctionName = TEXT(""));

	/**
	 * Determines whether to call server RPC or execute directly.
	 * 
	 * @param Actor Actor to check
	 * @return True if should call server RPC
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Authority")
	static bool ShouldCallServerRPC(const AActor* Actor);

	// ============================================================================
	// World & GameInstance Access
	// ============================================================================

	/**
	 * Safely gets UWorld from WorldContextObject.
	 * 
	 * @param WorldContextObject Object that provides world context
	 * @return UWorld or nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|World", meta = (WorldContext = "WorldContextObject"))
	static UWorld* GetWorldSafe(const UObject* WorldContextObject);

	/**
	 * Gets GameInstance from WorldContextObject.
	 * 
	 * @param WorldContextObject Object that provides world context
	 * @return UGameInstance or nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|World", meta = (WorldContext = "WorldContextObject"))
	static UGameInstance* GetGameInstanceSafe(const UObject* WorldContextObject);

	/**
	 * Gets subsystem from GameInstance.
	 * Replaces GetWorld()->GetGameInstance()->GetSubsystem<T>() pattern.
	 * 
	 * @tparam TSubsystem Subsystem type
	 * @param WorldContextObject Object that provides world context
	 * @return Subsystem or nullptr
	 */
	template<typename TSubsystem>
	static TSubsystem* GetGameInstanceSubsystem(const UObject* WorldContextObject)
	{
		if (UGameInstance* GameInstance = GetGameInstanceSafe(WorldContextObject))
		{
			return GameInstance->GetSubsystem<TSubsystem>();
		}
		return nullptr;
	}

	/**
	 * Gets subsystem from World.
	 * 
	 * @tparam TSubsystem Subsystem type
	 * @param WorldContextObject Object that provides world context
	 * @return Subsystem or nullptr
	 */
	template<typename TSubsystem>
	static TSubsystem* GetWorldSubsystem(const UObject* WorldContextObject)
	{
		if (UWorld* World = GetWorldSafe(WorldContextObject))
		{
			return World->GetSubsystem<TSubsystem>();
		}
		return nullptr;
	}

	// ============================================================================
	// Component Access
	// ============================================================================

	/**
	 * Safely gets component from actor.
	 * Replaces Actor->FindComponentByClass<T>() pattern.
	 * 
	 * @tparam TComponent Component type
	 * @param Actor Actor to search
	 * @param bLogIfNotFound Whether to log if not found
	 * @return Component or nullptr
	 */
	template<typename TComponent>
	static TComponent* GetComponentSafe(AActor* Actor, bool bLogIfNotFound = false)
	{
		if (!Actor)
		{
			return nullptr;
		}

		TComponent* Component = Actor->FindComponentByClass<TComponent>();
		if (!Component && bLogIfNotFound)
		{
			UE_LOG(LogHarmoniaKit, Warning, TEXT("GetComponentSafe: %s not found on %s"),
				*TComponent::StaticClass()->GetName(), *Actor->GetName());
		}
		return Component;
	}

	/**
	 * Gets sibling component from owner.
	 * Replaces GetOwner()->FindComponentByClass<T>() pattern.
	 * 
	 * @tparam TComponent Component type
	 * @param SourceComponent Source component
	 * @param bLogIfNotFound Whether to log if not found
	 * @return Component or nullptr
	 */
	template<typename TComponent>
	static TComponent* GetSiblingComponent(UActorComponent* SourceComponent, bool bLogIfNotFound = false)
	{
		if (!SourceComponent)
		{
			return nullptr;
		}

		AActor* Owner = SourceComponent->GetOwner();
		return GetComponentSafe<TComponent>(Owner, bLogIfNotFound);
	}

	/**
	 * Gets or creates a component on the actor.
	 * 
	 * @tparam TComponent Component type
	 * @param Actor Target actor
	 * @return Existing or newly created component
	 */
	template<typename TComponent>
	static TComponent* GetOrCreateComponent(AActor* Actor)
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
	 * Blueprint-callable component search.
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Component", meta = (DeterminesOutputType = "ComponentClass"))
	static UActorComponent* GetComponentByClass(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass);

	// ============================================================================
	// Ability System Access
	// ============================================================================

	/**
	 * Gets AbilitySystemComponent from actor.
	 * Replaces UAbilitySystemGlobals::GetAbilitySystemComponentFromActor() pattern.
	 * 
	 * @param Actor Actor to search
	 * @return ASC or nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|AbilitySystem")
	static UAbilitySystemComponent* GetASC(AActor* Actor);

	/**
	 * Checks if actor has an ASC.
	 * 
	 * @param Actor Actor to check
	 * @return True if has ASC
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|AbilitySystem")
	static bool HasASC(const AActor* Actor);

	/**
	 * Checks if ASC has a gameplay tag.
	 * 
	 * @param Actor Actor to check
	 * @param Tag Tag to check for
	 * @return True if tag exists
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|AbilitySystem")
	static bool HasGameplayTag(AActor* Actor, FGameplayTag Tag);

	/**
	 * Adds a gameplay tag to ASC.
	 * 
	 * @param Actor Target actor
	 * @param Tag Tag to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Core|AbilitySystem")
	static void AddGameplayTag(AActor* Actor, FGameplayTag Tag);

	/**
	 * Removes a gameplay tag from ASC.
	 * 
	 * @param Actor Target actor
	 * @param Tag Tag to remove
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Core|AbilitySystem")
	static void RemoveGameplayTag(AActor* Actor, FGameplayTag Tag);

	// ============================================================================
	// Player Access
	// ============================================================================

	/**
	 * Gets local player controller.
	 * 
	 * @param WorldContextObject Object that provides world context
	 * @param PlayerIndex Player index (default: 0)
	 * @return Player controller or nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Player", meta = (WorldContext = "WorldContextObject"))
	static APlayerController* GetLocalPlayerController(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Gets local player's pawn.
	 * 
	 * @param WorldContextObject Object that provides world context
	 * @param PlayerIndex Player index (default: 0)
	 * @return Pawn or nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Player", meta = (WorldContext = "WorldContextObject"))
	static APawn* GetLocalPlayerPawn(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * Gets specific component from local player.
	 * 
	 * @tparam TComponent Component type
	 * @param WorldContextObject Object that provides world context
	 * @param PlayerIndex Player index (default: 0)
	 * @return Component or nullptr
	 */
	template<typename TComponent>
	static TComponent* GetLocalPlayerComponent(const UObject* WorldContextObject, int32 PlayerIndex = 0)
	{
		if (APawn* Pawn = GetLocalPlayerPawn(WorldContextObject, PlayerIndex))
		{
			return Pawn->FindComponentByClass<TComponent>();
		}
		return nullptr;
	}

	// ============================================================================
	// Validation & Null Checks
	// ============================================================================

	/**
	 * Checks if object is valid and not pending kill.
	 * 
	 * @param Object Object to check
	 * @return True if valid
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Validation")
	static bool IsValidObject(const UObject* Object);

	/**
	 * Checks if actor is valid and usable in game.
	 * 
	 * @param Actor Actor to check
	 * @return True if valid
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Validation")
	static bool IsValidActor(const AActor* Actor);

	/**
	 * Validates multiple objects at once.
	 * 
	 * @param Objects Objects to check
	 * @return True if all objects are valid
	 */
	static bool AreAllValid(std::initializer_list<const UObject*> Objects);

	// ============================================================================
	// Debug & Logging Helpers
	// ============================================================================

	/**
	 * Conditional log output (Development builds only).
	 */
	static void LogIfDevelopment(const FString& Message, ELogVerbosity::Type Verbosity = ELogVerbosity::Log);

	/**
	 * Logs error and returns false.
	 * Usage: return LogErrorAndReturnFalse("Something failed");
	 */
	static bool LogErrorAndReturnFalse(const FString& ErrorMessage);

	/**
	 * Logs warning and returns nullptr.
	 */
	template<typename T>
	static T* LogWarningAndReturnNull(const FString& WarningMessage)
	{
		UE_LOG(LogHarmoniaKit, Warning, TEXT("%s"), *WarningMessage);
		return nullptr;
	}
};

// ============================================================================
// Macros for Common Patterns
// ============================================================================

/**
 * Server authority check macro.
 * Returns immediately if not on server.
 * 
 * Usage:
 * void MyFunction()
 * {
 *     HARMONIA_REQUIRE_SERVER(this);  // this is UActorComponent
 *     // Server-only code...
 * }
 */
#define HARMONIA_REQUIRE_SERVER(Component) \
	if (!UHarmoniaCoreBFL::IsComponentServerAuthoritative(Component)) \
	{ \
		return; \
	}

/**
 * Server authority check macro (for functions with return value).
 * 
 * Usage:
 * bool MyFunction()
 * {
 *     HARMONIA_REQUIRE_SERVER_RETURN(this, false);
 *     // Server-only code...
 *     return true;
 * }
 */
#define HARMONIA_REQUIRE_SERVER_RETURN(Component, ReturnValue) \
	if (!UHarmoniaCoreBFL::IsComponentServerAuthoritative(Component)) \
	{ \
		return ReturnValue; \
	}

/**
 * Validity check macro.
 * 
 * Usage:
 * void MyFunction(AActor* Actor)
 * {
 *     HARMONIA_CHECK_VALID(Actor);
 *     // Actor usage code...
 * }
 */
#define HARMONIA_CHECK_VALID(Object) \
	if (!UHarmoniaCoreBFL::IsValidObject(Object)) \
	{ \
		return; \
	}

/**
 * Validity check macro (for functions with return value).
 */
#define HARMONIA_CHECK_VALID_RETURN(Object, ReturnValue) \
	if (!UHarmoniaCoreBFL::IsValidObject(Object)) \
	{ \
		return ReturnValue; \
	}

/**
 * Get component + validity check macro.
 * 
 * Usage:
 * void MyFunction()
 * {
 *     HARMONIA_GET_COMPONENT_OR_RETURN(UMyComponent, MyComp, GetOwner());
 *     MyComp->DoSomething();
 * }
 */
#define HARMONIA_GET_COMPONENT_OR_RETURN(ComponentType, VarName, Actor) \
	ComponentType* VarName = UHarmoniaCoreBFL::GetComponentSafe<ComponentType>(Actor); \
	if (!VarName) \
	{ \
		return; \
	}

/**
 * Get component + validity check macro (with return value).
 */
#define HARMONIA_GET_COMPONENT_OR_RETURN_VALUE(ComponentType, VarName, Actor, ReturnValue) \
	ComponentType* VarName = UHarmoniaCoreBFL::GetComponentSafe<ComponentType>(Actor); \
	if (!VarName) \
	{ \
		return ReturnValue; \
	}

/**
 * Get subsystem + validity check macro.
 * 
 * Usage:
 * void MyFunction()
 * {
 *     HARMONIA_GET_SUBSYSTEM_OR_RETURN(UMySaveSubsystem, SaveSys, this);
 *     SaveSys->SaveGame();
 * }
 */
#define HARMONIA_GET_SUBSYSTEM_OR_RETURN(SubsystemType, VarName, WorldContext) \
	SubsystemType* VarName = UHarmoniaCoreBFL::GetGameInstanceSubsystem<SubsystemType>(WorldContext); \
	if (!VarName) \
	{ \
		return; \
	}

/**
 * Get subsystem + validity check macro (with return value).
 */
#define HARMONIA_GET_SUBSYSTEM_OR_RETURN_VALUE(SubsystemType, VarName, WorldContext, ReturnValue) \
	SubsystemType* VarName = UHarmoniaCoreBFL::GetGameInstanceSubsystem<SubsystemType>(WorldContext); \
	if (!VarName) \
	{ \
		return ReturnValue; \
	}
