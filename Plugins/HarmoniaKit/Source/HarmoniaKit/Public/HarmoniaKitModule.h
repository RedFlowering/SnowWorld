// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaKitModule.h
 * @brief HarmoniaKit 플러그인 코어 모듈 정의
 * 
 * HarmoniaKit의 중심 모듈로, 모든 게임플레이 시스템의
 * 기반이 되는 클래스들을 포함합니다.
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * @class FHarmoniaKit
 * @brief HarmoniaKit 플러그인 코어 모듈 클래스
 * 
 * 플러그인의 주요 진입점으로, 게임플레이 태그 등록 등
 * 초기화 작업을 수행합니다.
 * 
 * @see HarmoniaGameplayTags
 * @see UHarmoniaGameServiceSubsystem
 */
class FHarmoniaKit : public FDefaultModuleImpl
{
public:
	/** @brief 모듈 시작 시 호출, 게임플레이 태그 등록 */
	virtual void StartupModule() override;
	
	/** @brief 모듈 종료 시 호출 */
	virtual void ShutdownModule() override;
};
