// Copyright Snow Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaCheatLibrary.generated.h"

/**
 * HarmoniaCheatLibrary
 * 
 * 블루?�린?�에??치트 매니?�???�게 ?�근?????�도�??�는 ?�수 ?�이브러리입?�다.
 * 
 * ?�️ 주의: ???�이브러리는 Shipping 빌드?�서 컴파?�되지�?치트 매니?�가 ?�으므�?
 *          모든 ?�수가 ?�질?�으�??�작?��? ?�습?�다.
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaCheatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * ?�재 ?�레?�어??치트 매니?��?가?�옵?�다.
	 * @param WorldContextObject ?�드 컨텍?�트
	 * @return 치트 매니?� (?�으�?nullptr)
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cheat", meta = (WorldContext = "WorldContextObject"))
	static class UHarmoniaCheatManager* GetHarmoniaCheatManager(const UObject* WorldContextObject);

	/**
	 * 치트 매니?�가 ?�성?�되???�는지 ?�인?�니??
	 * @param WorldContextObject ?�드 컨텍?�트
	 * @return 치트 매니?� ?�용 가???��?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cheat", meta = (WorldContext = "WorldContextObject"))
	static bool IsCheatManagerAvailable(const UObject* WorldContextObject);

	/**
	 * 치트가 ?�재 ?�성?�되???�는지 ?�인 (Development 빌드??-cheat 커맨?�라???�션 ?�요)
	 * @param WorldContextObject ?�드 컨텍?�트
	 * @return 치트 ?�성???��?
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cheat", meta = (WorldContext = "WorldContextObject"))
	static bool IsCheatsEnabled(const UObject* WorldContextObject);

	// ==================== Quick Cheat Functions ====================

	/**
	 * 빠른 치트: ?�레?�어�??�전???�복?�니??
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickHeal(const UObject* WorldContextObject);

	/**
	 * 빠른 치트: 무적 모드�??��??�니??
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickToggleInvincible(const UObject* WorldContextObject);

	/**
	 * 빠른 치트: 골드 10000??지급합?�다.
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickGiveGold(const UObject* WorldContextObject, int32 Amount = 10000);

	/**
	 * 빠른 치트: 비행 모드�??��??�니??
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickToggleFly(const UObject* WorldContextObject);

	/**
	 * 빠른 치트: ?�동 ?�도�?2배로 ?�정?�니??
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickDoubleSpeed(const UObject* WorldContextObject);

	/**
	 * 빠른 치트: 치트�?모두 리셋?�니??
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|Cheat|Quick", meta = (WorldContext = "WorldContextObject"))
	static void QuickResetCheats(const UObject* WorldContextObject);
};
