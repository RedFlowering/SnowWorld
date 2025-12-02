// Copyright Snow Game Studio. All Rights Reserved.

#include "HarmoniaGameModeExample.h"
#include "System/HarmoniaCheatManager.h"
#include "Engine/World.h"

AHarmoniaGameModeExample::AHarmoniaGameModeExample()
{
	// 치트 매니저 클래스는 블루프린트나 에디터에서 설정하세요.
	// CheatClass = UHarmoniaCheatManager::StaticClass();
	// 
	// ?�는 ?�로?또는 프로젝트 설정 > General Settings > CheatClass에서 UHarmoniaCheatManager로변경하세요.
	
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameMode] Harmonia 게임모드가 생성되었습니다."));
	UE_LOG(LogTemp, Log, TEXT("[HarmoniaGameMode] 치트를 사용하려면 CheatClass를 UHarmoniaCheatManager로 설정하세요."));
}

void AHarmoniaGameModeExample::BeginPlay()
{
	Super::BeginPlay();

	if (IsCheatsEnabled())
	{
		UE_LOG(LogTemp, Warning, TEXT("========================================"));
		UE_LOG(LogTemp, Warning, TEXT("  Harmonia 치트 ?�스???�성?�됨"));
		UE_LOG(LogTemp, Warning, TEXT("  콘솔(~)???�고 'HarmoniaHelp' ?�력"));
		UE_LOG(LogTemp, Warning, TEXT("========================================"));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1, 
				10.0f, 
				FColor::Yellow, 
				TEXT("[Harmonia] 치트 ?�스???�성?�됨! 콘솔(~)?�서 'HarmoniaHelp' ?�력")
			);
		}
	}
}

bool AHarmoniaGameModeExample::IsCheatsEnabled() const
{
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			return PC->CheatManager != nullptr;
		}
	}
	return false;
}
