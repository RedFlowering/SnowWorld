// Copyright Snow Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HarmoniaGameModeExample.generated.h"

/**
 * Harmonia 게임모드 예제
 * 
 * 이 게임모드는 HarmoniaCheatManager를 자동으로 생성합니다.
 * 프로젝트의 게임모드를 이 클래스에서 상속받거나
 * 기존 게임모드의 CheatClass를 UHarmoniaCheatManager로 설정하세요.
 */
UCLASS()
class HARMONIAKIT_API AHarmoniaGameModeExample : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHarmoniaGameModeExample();

protected:
	virtual void BeginPlay() override;

	/**
	 * 치트가 활성화되어 있는지 확인
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cheat")
	bool IsCheatsEnabled() const;
};
