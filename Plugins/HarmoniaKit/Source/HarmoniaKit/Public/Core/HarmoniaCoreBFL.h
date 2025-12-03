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
 * ?�역?�으�??�용?�는 공통 ?�틸리티 ?�수?�을 ?�공?�니??
 * ???�이브러리는 코드 중복??줄이�??��????�턴???�공?�기 ?�해 ?�계?�었?�니??
 * 
 * 주요 기능:
 * - ?�버/?�라?�언??권한 체크
 * - ?�브?�스???�근
 * - 컴포?�트 ?�전 ?�근
 * - AbilitySystem ?�근
 * - World/GameInstance ?�근
 * 
 * @see UHarmoniaComponentUtils - 컴포?�트 관???�틸리티
 * @see UHarmoniaReplicationUtils - ?�트?�크 관???�틸리티
 * @see UHarmoniaCombatLibrary - ?�투 관???�틸리티
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
	 * ?�터가 ?�버 권한??가지�??�는지 ?�인?�니??
	 * 컴포?�트?�서 GetOwner()->HasAuthority() ?�턴???�체합?�다.
	 * 
	 * @param Actor ?�인???�터
	 * @return ?�버 권한???�으�?true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Authority")
	static bool HasServerAuthority(const AActor* Actor);

	/**
	 * 컴포?�트???�너가 ?�버 권한??가지�??�는지 ?�인?�니??
	 * if (!GetOwner() || !GetOwner()->HasAuthority()) ?�턴???�체합?�다.
	 * 
	 * @param Component ?�인??컴포?�트
	 * @return ?�너가 존재?�고 ?�버 권한???�으�?true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Authority")
	static bool IsComponentServerAuthoritative(const UActorComponent* Component);

	/**
	 * ?�버?�서�??�행?�어???�는 코드?��? ?�인?�니??
	 * ?�버가 ?�닌 경우 로그�?출력?�니??
	 * 
	 * @param Component ?�인??컴포?�트
	 * @param FunctionName 로그???�시???�수 ?�름 (?�버깅용)
	 * @return ?�버?�서 ?�행 중이�?true
	 */
	static bool CheckServerAuthority(const UActorComponent* Component, const FString& FunctionName = TEXT(""));

	/**
	 * ?�라?�언?�에???�버�??�청??보낼지, 직접 ?�행?��? 결정?�니??
	 * 
	 * @param Actor ?�인???�터
	 * @return ?�버 RPC�??�출?�야 ?�면 true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Authority")
	static bool ShouldCallServerRPC(const AActor* Actor);

	// ============================================================================
	// World & GameInstance Access
	// ============================================================================

	/**
	 * WorldContextObject?�서 UWorld�??�전?�게 가?�옵?�다.
	 * 
	 * @param WorldContextObject World 컨텍?�트�?가�?객체
	 * @return UWorld ?�는 nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|World", meta = (WorldContext = "WorldContextObject"))
	static UWorld* GetWorldSafe(const UObject* WorldContextObject);

	/**
	 * WorldContextObject?�서 GameInstance�?가?�옵?�다.
	 * 
	 * @param WorldContextObject World 컨텍?�트�?가�?객체
	 * @return UGameInstance ?�는 nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|World", meta = (WorldContext = "WorldContextObject"))
	static UGameInstance* GetGameInstanceSafe(const UObject* WorldContextObject);

	/**
	 * GameInstance?�서 ?�브?�스?�을 가?�옵?�다.
	 * GetWorld()->GetGameInstance()->GetSubsystem<T>() ?�턴???�체합?�다.
	 * 
	 * @tparam TSubsystem ?�브?�스???�??
	 * @param WorldContextObject World 컨텍?�트�?가�?객체
	 * @return ?�브?�스???�는 nullptr
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
	 * World?�서 ?�브?�스?�을 가?�옵?�다.
	 * 
	 * @tparam TSubsystem ?�브?�스???�??
	 * @param WorldContextObject World 컨텍?�트�?가�?객체
	 * @return ?�브?�스???�는 nullptr
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
	 * ?�터?�서 컴포?�트�??�전?�게 가?�옵?�다.
	 * Actor->FindComponentByClass<T>() ?�턴???�체합?�다.
	 * 
	 * @tparam TComponent 컴포?�트 ?�??
	 * @param Actor 검?�할 ?�터
	 * @param bLogIfNotFound 찾�? 못했????로그 출력 ?��?
	 * @return 컴포?�트 ?�는 nullptr
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
	 * 컴포?�트???�너?�서 ?�른 컴포?�트�?가?�옵?�다.
	 * GetOwner()->FindComponentByClass<T>() ?�턴???�체합?�다.
	 * 
	 * @tparam TComponent 컴포?�트 ?�??
	 * @param SourceComponent ?�작?�이 ?�는 컴포?�트
	 * @param bLogIfNotFound 찾�? 못했????로그 출력 ?��?
	 * @return 컴포?�트 ?�는 nullptr
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
	 * 컴포?�트�?가?�오거나 ?�으�??�성?�니??
	 * 
	 * @tparam TComponent 컴포?�트 ?�??
	 * @param Actor ?�???�터
	 * @return 기존 ?�는 ?�로 ?�성??컴포?�트
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
	 * 블루?�린?�에???�용 가?�한 컴포?�트 검??
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Component", meta = (DeterminesOutputType = "ComponentClass"))
	static UActorComponent* GetComponentByClass(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass);

	// ============================================================================
	// Ability System Access
	// ============================================================================

	/**
	 * ?�터?�서 AbilitySystemComponent�?가?�옵?�다.
	 * UAbilitySystemGlobals::GetAbilitySystemComponentFromActor() ?�턴???�체합?�다.
	 * 
	 * @param Actor 검?�할 ?�터
	 * @return ASC ?�는 nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|AbilitySystem")
	static UAbilitySystemComponent* GetASC(AActor* Actor);

	/**
	 * ?�터가 ASC�?가지�??�는지 ?�인?�니??
	 * 
	 * @param Actor ?�인???�터
	 * @return ASC가 ?�으�?true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|AbilitySystem")
	static bool HasASC(const AActor* Actor);

	/**
	 * ASC?�서 ?�그�??�인?�니??
	 * 
	 * @param Actor ?�인???�터
	 * @param Tag ?�인???�그
	 * @return ?�그가 ?�으�?true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|AbilitySystem")
	static bool HasGameplayTag(AActor* Actor, FGameplayTag Tag);

	/**
	 * ASC???�그�?추�??�니??
	 * 
	 * @param Actor ?�???�터
	 * @param Tag 추�????�그
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Core|AbilitySystem")
	static void AddGameplayTag(AActor* Actor, FGameplayTag Tag);

	/**
	 * ASC?�서 ?�그�??�거?�니??
	 * 
	 * @param Actor ?�???�터
	 * @param Tag ?�거???�그
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Core|AbilitySystem")
	static void RemoveGameplayTag(AActor* Actor, FGameplayTag Tag);

	// ============================================================================
	// Player Access
	// ============================================================================

	/**
	 * 로컬 ?�레?�어 컨트롤러�?가?�옵?�다.
	 * 
	 * @param WorldContextObject World 컨텍?�트�?가�?객체
	 * @param PlayerIndex ?�레?�어 ?�덱??(기본: 0)
	 * @return ?�레?�어 컨트롤러 ?�는 nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Player", meta = (WorldContext = "WorldContextObject"))
	static APlayerController* GetLocalPlayerController(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * 로컬 ?�레?�어??Pawn??가?�옵?�다.
	 * 
	 * @param WorldContextObject World 컨텍?�트�?가�?객체
	 * @param PlayerIndex ?�레?�어 ?�덱??(기본: 0)
	 * @return Pawn ?�는 nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Player", meta = (WorldContext = "WorldContextObject"))
	static APawn* GetLocalPlayerPawn(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * 로컬 ?�레?�어???�정 컴포?�트�?가?�옵?�다.
	 * 
	 * @tparam TComponent 컴포?�트 ?�??
	 * @param WorldContextObject World 컨텍?�트�?가�?객체
	 * @param PlayerIndex ?�레?�어 ?�덱??(기본: 0)
	 * @return 컴포?�트 ?�는 nullptr
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
	 * 객체가 ?�효?�고 Pending Kill???�닌지 ?�인?�니??
	 * 
	 * @param Object ?�인??객체
	 * @return ?�효?�면 true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Validation")
	static bool IsValidObject(const UObject* Object);

	/**
	 * ?�터가 ?�효?�고 게임?�서 ?�용 가?�한지 ?�인?�니??
	 * 
	 * @param Actor ?�인???�터
	 * @return ?�효?�면 true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Validation")
	static bool IsValidActor(const AActor* Actor);

	/**
	 * ?�러 객체???�효?�을 ??번에 ?�인?�니??
	 * 
	 * @param Objects ?�인??객체??
	 * @return 모든 객체가 ?�효?�면 true
	 */
	static bool AreAllValid(std::initializer_list<const UObject*> Objects);

	// ============================================================================
	// Debug & Logging Helpers
	// ============================================================================

	/**
	 * 조건부 로그 출력 (Development 빌드?�서�?
	 */
	static void LogIfDevelopment(const FString& Message, ELogVerbosity::Type Verbosity = ELogVerbosity::Log);

	/**
	 * ?�러 로그?� ?�께 false�?반환?�니??
	 * return LogErrorAndReturnFalse("Something failed"); ?�턴???�용
	 */
	static bool LogErrorAndReturnFalse(const FString& ErrorMessage);

	/**
	 * 경고 로그?� ?�께 nullptr??반환?�니??
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
 * ?�버 권한 체크 매크�?
 * ?�버가 ?�닌 경우 ?�수?�서 즉시 반환?�니??
 * 
 * ?�용 ??
 * void MyFunction()
 * {
 *     HARMONIA_REQUIRE_SERVER(this);  // this가 UActorComponent??경우
 *     // ?�버 ?�용 코드...
 * }
 */
#define HARMONIA_REQUIRE_SERVER(Component) \
	if (!UHarmoniaCoreBFL::IsComponentServerAuthoritative(Component)) \
	{ \
		return; \
	}

/**
 * ?�버 권한 체크 매크�?(반환값이 ?�는 ?�수??
 * 
 * ?�용 ??
 * bool MyFunction()
 * {
 *     HARMONIA_REQUIRE_SERVER_RETURN(this, false);
 *     // ?�버 ?�용 코드...
 *     return true;
 * }
 */
#define HARMONIA_REQUIRE_SERVER_RETURN(Component, ReturnValue) \
	if (!UHarmoniaCoreBFL::IsComponentServerAuthoritative(Component)) \
	{ \
		return ReturnValue; \
	}

/**
 * ?�효??체크 매크�?
 * 
 * ?�용 ??
 * void MyFunction(AActor* Actor)
 * {
 *     HARMONIA_CHECK_VALID(Actor);
 *     // Actor ?�용 코드...
 * }
 */
#define HARMONIA_CHECK_VALID(Object) \
	if (!UHarmoniaCoreBFL::IsValidObject(Object)) \
	{ \
		return; \
	}

/**
 * ?�효??체크 매크�?(반환값이 ?�는 ?�수??
 */
#define HARMONIA_CHECK_VALID_RETURN(Object, ReturnValue) \
	if (!UHarmoniaCoreBFL::IsValidObject(Object)) \
	{ \
		return ReturnValue; \
	}

/**
 * 컴포?�트 가?�오�?+ ?�효??체크 매크�?
 * 
 * ?�용 ??
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
 * 컴포?�트 가?�오�?+ ?�효??체크 매크�?(반환�?버전)
 */
#define HARMONIA_GET_COMPONENT_OR_RETURN_VALUE(ComponentType, VarName, Actor, ReturnValue) \
	ComponentType* VarName = UHarmoniaCoreBFL::GetComponentSafe<ComponentType>(Actor); \
	if (!VarName) \
	{ \
		return ReturnValue; \
	}

/**
 * ?�브?�스??가?�오�?+ ?�효??체크 매크�?
 * 
 * ?�용 ??
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
 * ?�브?�스??가?�오�?+ ?�효??체크 매크�?(반환�?버전)
 */
#define HARMONIA_GET_SUBSYSTEM_OR_RETURN_VALUE(SubsystemType, VarName, WorldContext, ReturnValue) \
	SubsystemType* VarName = UHarmoniaCoreBFL::GetGameInstanceSubsystem<SubsystemType>(WorldContext); \
	if (!VarName) \
	{ \
		return ReturnValue; \
	}
