// Copyright 2025 Snow Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaLogCategories.h"
#include "HarmoniaCoreBFL.generated.h"

class UAbilitySystemComponent;
class UGameInstance;
class APlayerController;

/**
 * HarmoniaCoreBFL - Harmonia Core Blueprint Function Library
 * 
 * ?„ì—­?ìœ¼ë¡??¬ìš©?˜ëŠ” ê³µí†µ ? í‹¸ë¦¬í‹° ?¨ìˆ˜?¤ì„ ?œê³µ?©ë‹ˆ??
 * ???¼ì´ë¸ŒëŸ¬ë¦¬ëŠ” ì½”ë“œ ì¤‘ë³µ??ì¤„ì´ê³??¼ê????¨í„´???œê³µ?˜ê¸° ?„í•´ ?¤ê³„?˜ì—ˆ?µë‹ˆ??
 * 
 * ì£¼ìš” ê¸°ëŠ¥:
 * - ?œë²„/?´ë¼?´ì–¸??ê¶Œí•œ ì²´í¬
 * - ?œë¸Œ?œìŠ¤???‘ê·¼
 * - ì»´í¬?ŒíŠ¸ ?ˆì „ ?‘ê·¼
 * - AbilitySystem ?‘ê·¼
 * - World/GameInstance ?‘ê·¼
 * 
 * @see UHarmoniaComponentUtils - ì»´í¬?ŒíŠ¸ ê´€??? í‹¸ë¦¬í‹°
 * @see UHarmoniaReplicationUtils - ?¤íŠ¸?Œí¬ ê´€??? í‹¸ë¦¬í‹°
 * @see UHarmoniaCombatLibrary - ?„íˆ¬ ê´€??? í‹¸ë¦¬í‹°
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
	 * ?¡í„°ê°€ ?œë²„ ê¶Œí•œ??ê°€ì§€ê³??ˆëŠ”ì§€ ?•ì¸?©ë‹ˆ??
	 * ì»´í¬?ŒíŠ¸?ì„œ GetOwner()->HasAuthority() ?¨í„´???€ì²´í•©?ˆë‹¤.
	 * 
	 * @param Actor ?•ì¸???¡í„°
	 * @return ?œë²„ ê¶Œí•œ???ˆìœ¼ë©?true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Authority")
	static bool HasServerAuthority(const AActor* Actor);

	/**
	 * ì»´í¬?ŒíŠ¸???¤ë„ˆê°€ ?œë²„ ê¶Œí•œ??ê°€ì§€ê³??ˆëŠ”ì§€ ?•ì¸?©ë‹ˆ??
	 * if (!GetOwner() || !GetOwner()->HasAuthority()) ?¨í„´???€ì²´í•©?ˆë‹¤.
	 * 
	 * @param Component ?•ì¸??ì»´í¬?ŒíŠ¸
	 * @return ?¤ë„ˆê°€ ì¡´ì¬?˜ê³  ?œë²„ ê¶Œí•œ???ˆìœ¼ë©?true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Authority")
	static bool IsComponentServerAuthoritative(const UActorComponent* Component);

	/**
	 * ?œë²„?ì„œë§??¤í–‰?˜ì–´???˜ëŠ” ì½”ë“œ?¸ì? ?•ì¸?©ë‹ˆ??
	 * ?œë²„ê°€ ?„ë‹Œ ê²½ìš° ë¡œê·¸ë¥?ì¶œë ¥?©ë‹ˆ??
	 * 
	 * @param Component ?•ì¸??ì»´í¬?ŒíŠ¸
	 * @param FunctionName ë¡œê·¸???œì‹œ???¨ìˆ˜ ?´ë¦„ (?”ë²„ê¹…ìš©)
	 * @return ?œë²„?ì„œ ?¤í–‰ ì¤‘ì´ë©?true
	 */
	static bool CheckServerAuthority(const UActorComponent* Component, const FString& FunctionName = TEXT(""));

	/**
	 * ?´ë¼?´ì–¸?¸ì—???œë²„ë¡??”ì²­??ë³´ë‚¼ì§€, ì§ì ‘ ?¤í–‰? ì? ê²°ì •?©ë‹ˆ??
	 * 
	 * @param Actor ?•ì¸???¡í„°
	 * @return ?œë²„ RPCë¥??¸ì¶œ?´ì•¼ ?˜ë©´ true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Authority")
	static bool ShouldCallServerRPC(const AActor* Actor);

	// ============================================================================
	// World & GameInstance Access
	// ============================================================================

	/**
	 * WorldContextObject?ì„œ UWorldë¥??ˆì „?˜ê²Œ ê°€?¸ì˜µ?ˆë‹¤.
	 * 
	 * @param WorldContextObject World ì»¨í…?¤íŠ¸ë¥?ê°€ì§?ê°ì²´
	 * @return UWorld ?ëŠ” nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|World", meta = (WorldContext = "WorldContextObject"))
	static UWorld* GetWorldSafe(const UObject* WorldContextObject);

	/**
	 * WorldContextObject?ì„œ GameInstanceë¥?ê°€?¸ì˜µ?ˆë‹¤.
	 * 
	 * @param WorldContextObject World ì»¨í…?¤íŠ¸ë¥?ê°€ì§?ê°ì²´
	 * @return UGameInstance ?ëŠ” nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|World", meta = (WorldContext = "WorldContextObject"))
	static UGameInstance* GetGameInstanceSafe(const UObject* WorldContextObject);

	/**
	 * GameInstance?ì„œ ?œë¸Œ?œìŠ¤?œì„ ê°€?¸ì˜µ?ˆë‹¤.
	 * GetWorld()->GetGameInstance()->GetSubsystem<T>() ?¨í„´???€ì²´í•©?ˆë‹¤.
	 * 
	 * @tparam TSubsystem ?œë¸Œ?œìŠ¤???€??
	 * @param WorldContextObject World ì»¨í…?¤íŠ¸ë¥?ê°€ì§?ê°ì²´
	 * @return ?œë¸Œ?œìŠ¤???ëŠ” nullptr
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
	 * World?ì„œ ?œë¸Œ?œìŠ¤?œì„ ê°€?¸ì˜µ?ˆë‹¤.
	 * 
	 * @tparam TSubsystem ?œë¸Œ?œìŠ¤???€??
	 * @param WorldContextObject World ì»¨í…?¤íŠ¸ë¥?ê°€ì§?ê°ì²´
	 * @return ?œë¸Œ?œìŠ¤???ëŠ” nullptr
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
	 * ?¡í„°?ì„œ ì»´í¬?ŒíŠ¸ë¥??ˆì „?˜ê²Œ ê°€?¸ì˜µ?ˆë‹¤.
	 * Actor->FindComponentByClass<T>() ?¨í„´???€ì²´í•©?ˆë‹¤.
	 * 
	 * @tparam TComponent ì»´í¬?ŒíŠ¸ ?€??
	 * @param Actor ê²€?‰í•  ?¡í„°
	 * @param bLogIfNotFound ì°¾ì? ëª»í–ˆ????ë¡œê·¸ ì¶œë ¥ ?¬ë?
	 * @return ì»´í¬?ŒíŠ¸ ?ëŠ” nullptr
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
	 * ì»´í¬?ŒíŠ¸???¤ë„ˆ?ì„œ ?¤ë¥¸ ì»´í¬?ŒíŠ¸ë¥?ê°€?¸ì˜µ?ˆë‹¤.
	 * GetOwner()->FindComponentByClass<T>() ?¨í„´???€ì²´í•©?ˆë‹¤.
	 * 
	 * @tparam TComponent ì»´í¬?ŒíŠ¸ ?€??
	 * @param SourceComponent ?œì‘?ì´ ?˜ëŠ” ì»´í¬?ŒíŠ¸
	 * @param bLogIfNotFound ì°¾ì? ëª»í–ˆ????ë¡œê·¸ ì¶œë ¥ ?¬ë?
	 * @return ì»´í¬?ŒíŠ¸ ?ëŠ” nullptr
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
	 * ì»´í¬?ŒíŠ¸ë¥?ê°€?¸ì˜¤ê±°ë‚˜ ?†ìœ¼ë©??ì„±?©ë‹ˆ??
	 * 
	 * @tparam TComponent ì»´í¬?ŒíŠ¸ ?€??
	 * @param Actor ?€???¡í„°
	 * @return ê¸°ì¡´ ?ëŠ” ?ˆë¡œ ?ì„±??ì»´í¬?ŒíŠ¸
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
	 * ë¸”ë£¨?„ë¦°?¸ì—???¬ìš© ê°€?¥í•œ ì»´í¬?ŒíŠ¸ ê²€??
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Component", meta = (DeterminesOutputType = "ComponentClass"))
	static UActorComponent* GetComponentByClass(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass);

	// ============================================================================
	// Ability System Access
	// ============================================================================

	/**
	 * ?¡í„°?ì„œ AbilitySystemComponentë¥?ê°€?¸ì˜µ?ˆë‹¤.
	 * UAbilitySystemGlobals::GetAbilitySystemComponentFromActor() ?¨í„´???€ì²´í•©?ˆë‹¤.
	 * 
	 * @param Actor ê²€?‰í•  ?¡í„°
	 * @return ASC ?ëŠ” nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|AbilitySystem")
	static UAbilitySystemComponent* GetASC(AActor* Actor);

	/**
	 * ?¡í„°ê°€ ASCë¥?ê°€ì§€ê³??ˆëŠ”ì§€ ?•ì¸?©ë‹ˆ??
	 * 
	 * @param Actor ?•ì¸???¡í„°
	 * @return ASCê°€ ?ˆìœ¼ë©?true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|AbilitySystem")
	static bool HasASC(const AActor* Actor);

	/**
	 * ASC?ì„œ ?œê·¸ë¥??•ì¸?©ë‹ˆ??
	 * 
	 * @param Actor ?•ì¸???¡í„°
	 * @param Tag ?•ì¸???œê·¸
	 * @return ?œê·¸ê°€ ?ˆìœ¼ë©?true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|AbilitySystem")
	static bool HasGameplayTag(AActor* Actor, FGameplayTag Tag);

	/**
	 * ASC???œê·¸ë¥?ì¶”ê??©ë‹ˆ??
	 * 
	 * @param Actor ?€???¡í„°
	 * @param Tag ì¶”ê????œê·¸
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Core|AbilitySystem")
	static void AddGameplayTag(AActor* Actor, FGameplayTag Tag);

	/**
	 * ASC?ì„œ ?œê·¸ë¥??œê±°?©ë‹ˆ??
	 * 
	 * @param Actor ?€???¡í„°
	 * @param Tag ?œê±°???œê·¸
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Core|AbilitySystem")
	static void RemoveGameplayTag(AActor* Actor, FGameplayTag Tag);

	// ============================================================================
	// Player Access
	// ============================================================================

	/**
	 * ë¡œì»¬ ?Œë ˆ?´ì–´ ì»¨íŠ¸ë¡¤ëŸ¬ë¥?ê°€?¸ì˜µ?ˆë‹¤.
	 * 
	 * @param WorldContextObject World ì»¨í…?¤íŠ¸ë¥?ê°€ì§?ê°ì²´
	 * @param PlayerIndex ?Œë ˆ?´ì–´ ?¸ë±??(ê¸°ë³¸: 0)
	 * @return ?Œë ˆ?´ì–´ ì»¨íŠ¸ë¡¤ëŸ¬ ?ëŠ” nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Player", meta = (WorldContext = "WorldContextObject"))
	static APlayerController* GetLocalPlayerController(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * ë¡œì»¬ ?Œë ˆ?´ì–´??Pawn??ê°€?¸ì˜µ?ˆë‹¤.
	 * 
	 * @param WorldContextObject World ì»¨í…?¤íŠ¸ë¥?ê°€ì§?ê°ì²´
	 * @param PlayerIndex ?Œë ˆ?´ì–´ ?¸ë±??(ê¸°ë³¸: 0)
	 * @return Pawn ?ëŠ” nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Player", meta = (WorldContext = "WorldContextObject"))
	static APawn* GetLocalPlayerPawn(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * ë¡œì»¬ ?Œë ˆ?´ì–´???¹ì • ì»´í¬?ŒíŠ¸ë¥?ê°€?¸ì˜µ?ˆë‹¤.
	 * 
	 * @tparam TComponent ì»´í¬?ŒíŠ¸ ?€??
	 * @param WorldContextObject World ì»¨í…?¤íŠ¸ë¥?ê°€ì§?ê°ì²´
	 * @param PlayerIndex ?Œë ˆ?´ì–´ ?¸ë±??(ê¸°ë³¸: 0)
	 * @return ì»´í¬?ŒíŠ¸ ?ëŠ” nullptr
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
	 * ê°ì²´ê°€ ? íš¨?˜ê³  Pending Kill???„ë‹Œì§€ ?•ì¸?©ë‹ˆ??
	 * 
	 * @param Object ?•ì¸??ê°ì²´
	 * @return ? íš¨?˜ë©´ true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Validation")
	static bool IsValidObject(const UObject* Object);

	/**
	 * ?¡í„°ê°€ ? íš¨?˜ê³  ê²Œì„?ì„œ ?¬ìš© ê°€?¥í•œì§€ ?•ì¸?©ë‹ˆ??
	 * 
	 * @param Actor ?•ì¸???¡í„°
	 * @return ? íš¨?˜ë©´ true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Validation")
	static bool IsValidActor(const AActor* Actor);

	/**
	 * ?¬ëŸ¬ ê°ì²´??? íš¨?±ì„ ??ë²ˆì— ?•ì¸?©ë‹ˆ??
	 * 
	 * @param Objects ?•ì¸??ê°ì²´??
	 * @return ëª¨ë“  ê°ì²´ê°€ ? íš¨?˜ë©´ true
	 */
	static bool AreAllValid(std::initializer_list<const UObject*> Objects);

	// ============================================================================
	// Debug & Logging Helpers
	// ============================================================================

	/**
	 * ì¡°ê±´ë¶€ ë¡œê·¸ ì¶œë ¥ (Development ë¹Œë“œ?ì„œë§?
	 */
	static void LogIfDevelopment(const FString& Message, ELogVerbosity::Type Verbosity = ELogVerbosity::Log);

	/**
	 * ?ëŸ¬ ë¡œê·¸?€ ?¨ê»˜ falseë¥?ë°˜í™˜?©ë‹ˆ??
	 * return LogErrorAndReturnFalse("Something failed"); ?¨í„´???¬ìš©
	 */
	static bool LogErrorAndReturnFalse(const FString& ErrorMessage);

	/**
	 * ê²½ê³  ë¡œê·¸?€ ?¨ê»˜ nullptr??ë°˜í™˜?©ë‹ˆ??
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
 * ?œë²„ ê¶Œí•œ ì²´í¬ ë§¤í¬ë¡?
 * ?œë²„ê°€ ?„ë‹Œ ê²½ìš° ?¨ìˆ˜?ì„œ ì¦‰ì‹œ ë°˜í™˜?©ë‹ˆ??
 * 
 * ?¬ìš© ??
 * void MyFunction()
 * {
 *     HARMONIA_REQUIRE_SERVER(this);  // thisê°€ UActorComponent??ê²½ìš°
 *     // ?œë²„ ?„ìš© ì½”ë“œ...
 * }
 */
#define HARMONIA_REQUIRE_SERVER(Component) \
	if (!UHarmoniaCoreBFL::IsComponentServerAuthoritative(Component)) \
	{ \
		return; \
	}

/**
 * ?œë²„ ê¶Œí•œ ì²´í¬ ë§¤í¬ë¡?(ë°˜í™˜ê°’ì´ ?ˆëŠ” ?¨ìˆ˜??
 * 
 * ?¬ìš© ??
 * bool MyFunction()
 * {
 *     HARMONIA_REQUIRE_SERVER_RETURN(this, false);
 *     // ?œë²„ ?„ìš© ì½”ë“œ...
 *     return true;
 * }
 */
#define HARMONIA_REQUIRE_SERVER_RETURN(Component, ReturnValue) \
	if (!UHarmoniaCoreBFL::IsComponentServerAuthoritative(Component)) \
	{ \
		return ReturnValue; \
	}

/**
 * ? íš¨??ì²´í¬ ë§¤í¬ë¡?
 * 
 * ?¬ìš© ??
 * void MyFunction(AActor* Actor)
 * {
 *     HARMONIA_CHECK_VALID(Actor);
 *     // Actor ?¬ìš© ì½”ë“œ...
 * }
 */
#define HARMONIA_CHECK_VALID(Object) \
	if (!UHarmoniaCoreBFL::IsValidObject(Object)) \
	{ \
		return; \
	}

/**
 * ? íš¨??ì²´í¬ ë§¤í¬ë¡?(ë°˜í™˜ê°’ì´ ?ˆëŠ” ?¨ìˆ˜??
 */
#define HARMONIA_CHECK_VALID_RETURN(Object, ReturnValue) \
	if (!UHarmoniaCoreBFL::IsValidObject(Object)) \
	{ \
		return ReturnValue; \
	}

/**
 * ì»´í¬?ŒíŠ¸ ê°€?¸ì˜¤ê¸?+ ? íš¨??ì²´í¬ ë§¤í¬ë¡?
 * 
 * ?¬ìš© ??
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
 * ì»´í¬?ŒíŠ¸ ê°€?¸ì˜¤ê¸?+ ? íš¨??ì²´í¬ ë§¤í¬ë¡?(ë°˜í™˜ê°?ë²„ì „)
 */
#define HARMONIA_GET_COMPONENT_OR_RETURN_VALUE(ComponentType, VarName, Actor, ReturnValue) \
	ComponentType* VarName = UHarmoniaCoreBFL::GetComponentSafe<ComponentType>(Actor); \
	if (!VarName) \
	{ \
		return ReturnValue; \
	}

/**
 * ?œë¸Œ?œìŠ¤??ê°€?¸ì˜¤ê¸?+ ? íš¨??ì²´í¬ ë§¤í¬ë¡?
 * 
 * ?¬ìš© ??
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
 * ?œë¸Œ?œìŠ¤??ê°€?¸ì˜¤ê¸?+ ? íš¨??ì²´í¬ ë§¤í¬ë¡?(ë°˜í™˜ê°?ë²„ì „)
 */
#define HARMONIA_GET_SUBSYSTEM_OR_RETURN_VALUE(SubsystemType, VarName, WorldContext, ReturnValue) \
	SubsystemType* VarName = UHarmoniaCoreBFL::GetGameInstanceSubsystem<SubsystemType>(WorldContext); \
	if (!VarName) \
	{ \
		return ReturnValue; \
	}
