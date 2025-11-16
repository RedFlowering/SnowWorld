// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LyraSaveGameSubsystem.h"
#include "LyraSaveGameLibrary.generated.h"

/**
 * 블루프린트에서 세이브/로드 시스템을 쉽게 사용할 수 있도록 하는 함수 라이브러리
 */
UCLASS()
class LYRAGAME_API ULyraSaveGameLibrary : public UBlueprintFunctionLibrary
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
	UFUNCTION(BlueprintCallable, Category = "Lyra|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static bool SaveGame(const UObject* WorldContextObject, const FString& SaveSlotName = TEXT("DefaultSave"), bool bUseSteamCloud = true);

	/**
	 * 게임 로드
	 * @param WorldContextObject 월드 컨텍스트
	 * @param SaveSlotName 로드할 슬롯 이름
	 * @param bUseSteamCloud 스팀 클라우드에서 로드 여부
	 * @return 로드 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static bool LoadGame(const UObject* WorldContextObject, const FString& SaveSlotName = TEXT("DefaultSave"), bool bUseSteamCloud = true);

	/**
	 * 세이브 파일 삭제
	 * @param WorldContextObject 월드 컨텍스트
	 * @param SaveSlotName 삭제할 슬롯 이름
	 * @param bDeleteFromSteamCloud 스팀 클라우드에서도 삭제 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static bool DeleteSaveGame(const UObject* WorldContextObject, const FString& SaveSlotName = TEXT("DefaultSave"), bool bDeleteFromSteamCloud = true);

	/**
	 * 세이브 파일 존재 여부 확인
	 * @param WorldContextObject 월드 컨텍스트
	 * @param SaveSlotName 확인할 슬롯 이름
	 */
	UFUNCTION(BlueprintPure, Category = "Lyra|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static bool DoesSaveGameExist(const UObject* WorldContextObject, const FString& SaveSlotName = TEXT("DefaultSave"));

	/**
	 * 현재 로드된 세이브 게임 반환
	 * @param WorldContextObject 월드 컨텍스트
	 */
	UFUNCTION(BlueprintPure, Category = "Lyra|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static ULyraSaveGame* GetCurrentSaveGame(const UObject* WorldContextObject);

	/**
	 * 세이브 게임 서브시스템 가져오기
	 * @param WorldContextObject 월드 컨텍스트
	 */
	UFUNCTION(BlueprintPure, Category = "Lyra|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static ULyraSaveGameSubsystem* GetSaveGameSubsystem(const UObject* WorldContextObject);

	/**
	 * 자동 저장 활성화/비활성화
	 * @param WorldContextObject 월드 컨텍스트
	 * @param bEnabled 활성화 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static void SetAutoSaveEnabled(const UObject* WorldContextObject, bool bEnabled);

	/**
	 * 자동 저장 간격 설정
	 * @param WorldContextObject 월드 컨텍스트
	 * @param IntervalInSeconds 간격 (초 단위)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lyra|SaveGame", meta = (WorldContext = "WorldContextObject"))
	static void SetAutoSaveInterval(const UObject* WorldContextObject, float IntervalInSeconds);

	/**
	 * 플레이어의 스팀 ID 가져오기
	 * @param PlayerController 플레이어 컨트롤러
	 */
	UFUNCTION(BlueprintPure, Category = "Lyra|SaveGame")
	static FString GetPlayerSteamID(APlayerController* PlayerController);

	/**
	 * 현재 플레이어가 서버 소유주인지 확인
	 * @param PlayerController 플레이어 컨트롤러
	 */
	UFUNCTION(BlueprintPure, Category = "Lyra|SaveGame")
	static bool IsServerOwner(APlayerController* PlayerController);
};
