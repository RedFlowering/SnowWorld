// Copyright Snow Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaCheatLibrary.generated.h"

/**
 * HarmoniaCheatLibrary
 * 
 * 블루프린트에서 치트 매니저에 쉽게 접근할 수 있도록 하는 함수 라이브러리입니다.
 * 
 * ⚠️ 주의: 이 라이브러리는 Shipping 빌드에서 컴파일되지만 치트 매니저가 없으므로
 *          모든 함수가 실질적으로 동작하지 않습니다.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaCheatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 현재 플레이어의 치트 매니저를 가져옵니다.
	 * @param WorldContextObject 월드 컨텍스트
	 * @return 치트 매니저 (없으면 nullptr)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cheat", meta = (WorldContext = "WorldContextObject"))
	static class UHarmoniaCheatManager* GetHarmoniaCheatManager(const UObject* WorldContextObject);

	/**
	 * 치트 매니저가 활성화되어 있는지 확인합니다.
	 * @param WorldContextObject 월드 컨텍스트
	 * @return 치트 매니저 사용 가능 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cheat", meta = (WorldContext = "WorldContextObject"))
	static bool IsCheatManagerAvailable(const UObject* WorldContextObject);

	/**
	 * 치트가 현재 활성화되어 있는지 확인 (Development 빌드나 -cheat 커맨드라인 옵션 필요)
	 * @param WorldContextObject 월드 컨텍스트
	 * @return 치트 활성화 여부
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cheat", meta = (WorldContext = "WorldContextObject"))
	static bool IsCheatsEnabled(const UObject* WorldContextObject);

	// ==================== Quick Cheat Functions ====================

	/**
	 * 빠른 치트: 플레이어를 완전히 회복합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickHeal(const UObject* WorldContextObject);

	/**
	 * 빠른 치트: 무적 모드를 토글합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickToggleInvincible(const UObject* WorldContextObject);

	/**
	 * 빠른 치트: 골드 10000을 지급합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickGiveGold(const UObject* WorldContextObject, int32 Amount = 10000);

	/**
	 * 빠른 치트: 비행 모드를 토글합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickToggleFly(const UObject* WorldContextObject);

	/**
	 * 빠른 치트: 이동 속도를 2배로 설정합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickDoubleSpeed(const UObject* WorldContextObject);

	/**
	 * 빠른 치트: 치트를 모두 리셋합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickResetCheats(const UObject* WorldContextObject);
};
