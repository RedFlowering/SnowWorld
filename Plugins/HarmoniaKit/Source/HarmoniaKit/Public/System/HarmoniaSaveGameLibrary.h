// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HarmoniaSaveGameSubsystem.h"
#include "HarmoniaSaveGameLibrary.generated.h"

/**
 * 블루프린트에서 세이브/로드 시스템을 쉽게 사용할 수 있도록 돕는 함수 라이브러리
 */
UCLASS()
class HARMONIAKIT_API UHarmoniaSaveGameLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 게임 저장
	 * @param WorldContextObject 월드 컨텍스트
	 * @param SaveSlotName 저장 슬롯 이름
	 * @param bUseSteamCloud 스팀 클라우드 사용 여부
	 * @return 저장 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static bool SaveGame(const UObject* WorldContextObject, const FString& SaveSlotName = TEXT("DefaultSave"), bool bUseSteamCloud = true);

	/**
	 * 게임 로드
	 * @param WorldContextObject 월드 컨텍스트
	 * @param SaveSlotName 저장 슬롯 이름
	 * @param bUseSteamCloud 스팀 클라우드 사용 여부
	 * @return 로드 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static bool LoadGame(const UObject* WorldContextObject, const FString& SaveSlotName = TEXT("DefaultSave"), bool bUseSteamCloud = true);

	/**
	 * 세이브 파일 삭제
	 * @param WorldContextObject 월드 컨텍스트
	 * @param SaveSlotName 저장 슬롯 이름
	 * @param bDeleteFromSteamCloud 스팀 클라우드에서도 삭제할지 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static bool DeleteSaveGame(const UObject* WorldContextObject, const FString& SaveSlotName = TEXT("DefaultSave"), bool bDeleteFromSteamCloud = true);

	/**
	 * 세이브 파일 존재 확인
	 * @param WorldContextObject 월드 컨텍스트
	 * @param SaveSlotName 저장 슬롯 이름
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static bool DoesSaveGameExist(const UObject* WorldContextObject, const FString& SaveSlotName = TEXT("DefaultSave"));

	/**
	 * 현재 로드된 SaveGame 반환
	 * @param WorldContextObject 월드 컨텍스트
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static UHarmoniaSaveGame* GetCurrentSaveGame(const UObject* WorldContextObject);

	/**
	 * 세이브 게임 서브시스템 반환
	 * @param WorldContextObject 월드 컨텍스트
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static UHarmoniaSaveGameSubsystem* GetSaveGameSubsystem(const UObject* WorldContextObject);

	/**
	 * 자동 세이브 활성화/비활성화
	 * @param WorldContextObject 월드 컨텍스트
	 * @param bEnabled 활성화 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static void SetAutoSaveEnabled(const UObject* WorldContextObject, bool bEnabled);

	/**
	 * 자동 세이브 간격 설정 (초 단위)
	 * @param WorldContextObject 월드 컨텍스트
	 * @param IntervalInSeconds 간격 (초 단위)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harmonia|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static void SetAutoSaveInterval(const UObject* WorldContextObject, float IntervalInSeconds);

	/**
	 * 플레이어의 Steam ID 가져오기
	 * @param PlayerController 플레이어 컨트롤러
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SaveGame")
	static FString GetPlayerSteamID(APlayerController* PlayerController);

	/**
	 * 현재 플레이어가 서버 소유주인지 확인
	 * @param PlayerController 플레이어 컨트롤러
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|SaveGame")
	static bool IsServerOwner(APlayerController* PlayerController);
};
