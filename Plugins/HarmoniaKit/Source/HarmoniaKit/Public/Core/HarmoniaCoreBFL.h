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
 * 전역적으로 사용되는 공통 유틸리티 함수들을 제공합니다.
 * 이 라이브러리는 코드 중복을 줄이고 일관된 패턴을 제공하기 위해 설계되었습니다.
 * 
 * 주요 기능:
 * - 서버/클라이언트 권한 체크
 * - 서브시스템 접근
 * - 컴포넌트 안전 접근
 * - AbilitySystem 접근
 * - World/GameInstance 접근
 * 
 * @see UHarmoniaComponentUtils - 컴포넌트 관련 유틸리티
 * @see UHarmoniaReplicationUtils - 네트워크 관련 유틸리티
 * @see UHarmoniaCombatLibrary - 전투 관련 유틸리티
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
	 * 액터가 서버 권한을 가지고 있는지 확인합니다.
	 * 컴포넌트에서 GetOwner()->HasAuthority() 패턴을 대체합니다.
	 * 
	 * @param Actor 확인할 액터
	 * @return 서버 권한이 있으면 true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Authority")
	static bool HasServerAuthority(const AActor* Actor);

	/**
	 * 컴포넌트의 오너가 서버 권한을 가지고 있는지 확인합니다.
	 * if (!GetOwner() || !GetOwner()->HasAuthority()) 패턴을 대체합니다.
	 * 
	 * @param Component 확인할 컴포넌트
	 * @return 오너가 존재하고 서버 권한이 있으면 true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Authority")
	static bool IsComponentServerAuthoritative(const UActorComponent* Component);

	/**
	 * 서버에서만 실행되어야 하는 코드인지 확인합니다.
	 * 서버가 아닌 경우 로그를 출력합니다.
	 * 
	 * @param Component 확인할 컴포넌트
	 * @param FunctionName 로그에 표시할 함수 이름 (디버깅용)
	 * @return 서버에서 실행 중이면 true
	 */
	static bool CheckServerAuthority(const UActorComponent* Component, const FString& FunctionName = TEXT(""));

	/**
	 * 클라이언트에서 서버로 요청을 보낼지, 직접 실행할지 결정합니다.
	 * 
	 * @param Actor 확인할 액터
	 * @return 서버 RPC를 호출해야 하면 true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Authority")
	static bool ShouldCallServerRPC(const AActor* Actor);

	// ============================================================================
	// World & GameInstance Access
	// ============================================================================

	/**
	 * WorldContextObject에서 UWorld를 안전하게 가져옵니다.
	 * 
	 * @param WorldContextObject World 컨텍스트를 가진 객체
	 * @return UWorld 또는 nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|World", meta = (WorldContext = "WorldContextObject"))
	static UWorld* GetWorldSafe(const UObject* WorldContextObject);

	/**
	 * WorldContextObject에서 GameInstance를 가져옵니다.
	 * 
	 * @param WorldContextObject World 컨텍스트를 가진 객체
	 * @return UGameInstance 또는 nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|World", meta = (WorldContext = "WorldContextObject"))
	static UGameInstance* GetGameInstanceSafe(const UObject* WorldContextObject);

	/**
	 * GameInstance에서 서브시스템을 가져옵니다.
	 * GetWorld()->GetGameInstance()->GetSubsystem<T>() 패턴을 대체합니다.
	 * 
	 * @tparam TSubsystem 서브시스템 타입
	 * @param WorldContextObject World 컨텍스트를 가진 객체
	 * @return 서브시스템 또는 nullptr
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
	 * World에서 서브시스템을 가져옵니다.
	 * 
	 * @tparam TSubsystem 서브시스템 타입
	 * @param WorldContextObject World 컨텍스트를 가진 객체
	 * @return 서브시스템 또는 nullptr
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
	 * 액터에서 컴포넌트를 안전하게 가져옵니다.
	 * Actor->FindComponentByClass<T>() 패턴을 대체합니다.
	 * 
	 * @tparam TComponent 컴포넌트 타입
	 * @param Actor 검색할 액터
	 * @param bLogIfNotFound 찾지 못했을 때 로그 출력 여부
	 * @return 컴포넌트 또는 nullptr
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
	 * 컴포넌트의 오너에서 다른 컴포넌트를 가져옵니다.
	 * GetOwner()->FindComponentByClass<T>() 패턴을 대체합니다.
	 * 
	 * @tparam TComponent 컴포넌트 타입
	 * @param SourceComponent 시작점이 되는 컴포넌트
	 * @param bLogIfNotFound 찾지 못했을 때 로그 출력 여부
	 * @return 컴포넌트 또는 nullptr
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
	 * 컴포넌트를 가져오거나 없으면 생성합니다.
	 * 
	 * @tparam TComponent 컴포넌트 타입
	 * @param Actor 대상 액터
	 * @return 기존 또는 새로 생성된 컴포넌트
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
	 * 블루프린트에서 사용 가능한 컴포넌트 검색
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Component", meta = (DeterminesOutputType = "ComponentClass"))
	static UActorComponent* GetComponentByClass(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass);

	// ============================================================================
	// Ability System Access
	// ============================================================================

	/**
	 * 액터에서 AbilitySystemComponent를 가져옵니다.
	 * UAbilitySystemGlobals::GetAbilitySystemComponentFromActor() 패턴을 대체합니다.
	 * 
	 * @param Actor 검색할 액터
	 * @return ASC 또는 nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|AbilitySystem")
	static UAbilitySystemComponent* GetASC(AActor* Actor);

	/**
	 * 액터가 ASC를 가지고 있는지 확인합니다.
	 * 
	 * @param Actor 확인할 액터
	 * @return ASC가 있으면 true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|AbilitySystem")
	static bool HasASC(const AActor* Actor);

	/**
	 * ASC에서 태그를 확인합니다.
	 * 
	 * @param Actor 확인할 액터
	 * @param Tag 확인할 태그
	 * @return 태그가 있으면 true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|AbilitySystem")
	static bool HasGameplayTag(AActor* Actor, FGameplayTag Tag);

	/**
	 * ASC에 태그를 추가합니다.
	 * 
	 * @param Actor 대상 액터
	 * @param Tag 추가할 태그
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Core|AbilitySystem")
	static void AddGameplayTag(AActor* Actor, FGameplayTag Tag);

	/**
	 * ASC에서 태그를 제거합니다.
	 * 
	 * @param Actor 대상 액터
	 * @param Tag 제거할 태그
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Core|AbilitySystem")
	static void RemoveGameplayTag(AActor* Actor, FGameplayTag Tag);

	// ============================================================================
	// Player Access
	// ============================================================================

	/**
	 * 로컬 플레이어 컨트롤러를 가져옵니다.
	 * 
	 * @param WorldContextObject World 컨텍스트를 가진 객체
	 * @param PlayerIndex 플레이어 인덱스 (기본: 0)
	 * @return 플레이어 컨트롤러 또는 nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Player", meta = (WorldContext = "WorldContextObject"))
	static APlayerController* GetLocalPlayerController(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * 로컬 플레이어의 Pawn을 가져옵니다.
	 * 
	 * @param WorldContextObject World 컨텍스트를 가진 객체
	 * @param PlayerIndex 플레이어 인덱스 (기본: 0)
	 * @return Pawn 또는 nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Player", meta = (WorldContext = "WorldContextObject"))
	static APawn* GetLocalPlayerPawn(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/**
	 * 로컬 플레이어의 특정 컴포넌트를 가져옵니다.
	 * 
	 * @tparam TComponent 컴포넌트 타입
	 * @param WorldContextObject World 컨텍스트를 가진 객체
	 * @param PlayerIndex 플레이어 인덱스 (기본: 0)
	 * @return 컴포넌트 또는 nullptr
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
	 * 객체가 유효하고 Pending Kill이 아닌지 확인합니다.
	 * 
	 * @param Object 확인할 객체
	 * @return 유효하면 true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Validation")
	static bool IsValidObject(const UObject* Object);

	/**
	 * 액터가 유효하고 게임에서 사용 가능한지 확인합니다.
	 * 
	 * @param Actor 확인할 액터
	 * @return 유효하면 true
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Core|Validation")
	static bool IsValidActor(const AActor* Actor);

	/**
	 * 여러 객체의 유효성을 한 번에 확인합니다.
	 * 
	 * @param Objects 확인할 객체들
	 * @return 모든 객체가 유효하면 true
	 */
	static bool AreAllValid(std::initializer_list<const UObject*> Objects);

	// ============================================================================
	// Debug & Logging Helpers
	// ============================================================================

	/**
	 * 조건부 로그 출력 (Development 빌드에서만)
	 */
	static void LogIfDevelopment(const FString& Message, ELogVerbosity::Type Verbosity = ELogVerbosity::Log);

	/**
	 * 에러 로그와 함께 false를 반환합니다.
	 * return LogErrorAndReturnFalse("Something failed"); 패턴에 사용
	 */
	static bool LogErrorAndReturnFalse(const FString& ErrorMessage);

	/**
	 * 경고 로그와 함께 nullptr을 반환합니다.
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
 * 서버 권한 체크 매크로
 * 서버가 아닌 경우 함수에서 즉시 반환합니다.
 * 
 * 사용 예:
 * void MyFunction()
 * {
 *     HARMONIA_REQUIRE_SERVER(this);  // this가 UActorComponent일 경우
 *     // 서버 전용 코드...
 * }
 */
#define HARMONIA_REQUIRE_SERVER(Component) \
	if (!UHarmoniaCoreBFL::IsComponentServerAuthoritative(Component)) \
	{ \
		return; \
	}

/**
 * 서버 권한 체크 매크로 (반환값이 있는 함수용)
 * 
 * 사용 예:
 * bool MyFunction()
 * {
 *     HARMONIA_REQUIRE_SERVER_RETURN(this, false);
 *     // 서버 전용 코드...
 *     return true;
 * }
 */
#define HARMONIA_REQUIRE_SERVER_RETURN(Component, ReturnValue) \
	if (!UHarmoniaCoreBFL::IsComponentServerAuthoritative(Component)) \
	{ \
		return ReturnValue; \
	}

/**
 * 유효성 체크 매크로
 * 
 * 사용 예:
 * void MyFunction(AActor* Actor)
 * {
 *     HARMONIA_CHECK_VALID(Actor);
 *     // Actor 사용 코드...
 * }
 */
#define HARMONIA_CHECK_VALID(Object) \
	if (!UHarmoniaCoreBFL::IsValidObject(Object)) \
	{ \
		return; \
	}

/**
 * 유효성 체크 매크로 (반환값이 있는 함수용)
 */
#define HARMONIA_CHECK_VALID_RETURN(Object, ReturnValue) \
	if (!UHarmoniaCoreBFL::IsValidObject(Object)) \
	{ \
		return ReturnValue; \
	}

/**
 * 컴포넌트 가져오기 + 유효성 체크 매크로
 * 
 * 사용 예:
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
 * 컴포넌트 가져오기 + 유효성 체크 매크로 (반환값 버전)
 */
#define HARMONIA_GET_COMPONENT_OR_RETURN_VALUE(ComponentType, VarName, Actor, ReturnValue) \
	ComponentType* VarName = UHarmoniaCoreBFL::GetComponentSafe<ComponentType>(Actor); \
	if (!VarName) \
	{ \
		return ReturnValue; \
	}

/**
 * 서브시스템 가져오기 + 유효성 체크 매크로
 * 
 * 사용 예:
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
 * 서브시스템 가져오기 + 유효성 체크 매크로 (반환값 버전)
 */
#define HARMONIA_GET_SUBSYSTEM_OR_RETURN_VALUE(SubsystemType, VarName, WorldContext, ReturnValue) \
	SubsystemType* VarName = UHarmoniaCoreBFL::GetGameInstanceSubsystem<SubsystemType>(WorldContext); \
	if (!VarName) \
	{ \
		return ReturnValue; \
	}
