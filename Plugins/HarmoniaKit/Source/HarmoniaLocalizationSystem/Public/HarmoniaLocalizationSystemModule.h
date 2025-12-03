// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaLocalizationSystemModule.h
 * @brief HarmoniaLocalizationSystem 모듈 인터페이스 정의
 * 
 * 동적 다국어 지원 시스템의 모듈 클래스입니다.
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * @class FHarmoniaLocalizationSystem
 * @brief HarmoniaLocalizationSystem 모듈 클래스
 * 
 * 동적 다국어 지원 시스템의 진입점입니다.
 * 실제 기능은 UHarmoniaLocalizationSubsystem을 통해 접근합니다.
 * 
 * @see UHarmoniaLocalizationSubsystem
 */
class FHarmoniaLocalizationSystem : public FDefaultModuleImpl
{
public:
	/** @brief 모듈 시작 시 호출 */
	virtual void StartupModule() override;
	
	/** @brief 모듈 종료 시 호출 */
	virtual void ShutdownModule() override;
};
