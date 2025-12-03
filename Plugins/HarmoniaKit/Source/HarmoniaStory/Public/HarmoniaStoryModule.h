// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaStoryModule.h
 * @brief HarmoniaStory 모듈 인터페이스 정의
 * 
 * 스토리 및 내러티브 시스템의 모듈 클래스입니다.
 * 대화, 컷심, 저널, 관계 시스템을 포함합니다.
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * @class FHarmoniaStoryModule
 * @brief HarmoniaStory 모듈 클래스
 * 
 * 스토리 시스템의 진입점입니다.
 * 대화, 컷심, 저널, 관계 시스템을 초기화합니다.
 * 
 * @see UHarmoniaDialogueSubsystem
 * @see UHarmoniaCutsceneSubsystem
 */
class FHarmoniaStoryModule : public IModuleInterface
{
public:
	/** @brief 모듈 시작 시 호출 */
	virtual void StartupModule() override;
	
	/** @brief 모듈 종료 시 호출 */
	virtual void ShutdownModule() override;
};
