// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaModSystemModule.h
 * @brief HarmoniaModSystem 모듈 인터페이스 정의
 * 
 * 모드 로드/언로드, 의존성 해결, Workshop 통합 등
 * 모드 시스템의 핵심 기능을 제공합니다.
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * @class FHarmoniaModSystem
 * @brief HarmoniaModSystem 모듈 클래스
 * 
 * 모드 시스템의 진입점입니다.
 * 실제 기능은 UHarmoniaModSubsystem을 통해 접근합니다.
 * 
 * @see UHarmoniaModSubsystem
 */
class FHarmoniaModSystem : public FDefaultModuleImpl
{
public:
	/** @brief 모듈 시작 시 호출 */
	virtual void StartupModule() override;
	
	/** @brief 모듈 종료 시 호출 */
	virtual void ShutdownModule() override;
};
