// Copyright Snow Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HarmoniaGameModeExample.generated.h"

/**
 * Harmonia ê²Œì„ëª¨ë“œ ?ˆì œ
 * 
 * ??ê²Œì„ëª¨ë“œ??HarmoniaCheatManagerë¥??ë™?¼ë¡œ ?œì„±?”í•©?ˆë‹¤.
 * ?„ë¡œ?íŠ¸??ê²Œì„ëª¨ë“œë¥????´ë˜?¤ì—???ì†ë°›ê±°??
 * ê¸°ì¡´ ê²Œì„ëª¨ë“œ??CheatClassë¥?UHarmoniaCheatManagerë¡??¤ì •?˜ì„¸??
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
	 * ì¹˜íŠ¸ê°€ ?œì„±?”ë˜?ˆëŠ”ì§€ ?•ì¸
	 */
	UFUNCTION(BlueprintPure, Category = "Harmonia|Cheat")
	bool IsCheatsEnabled() const;
};
