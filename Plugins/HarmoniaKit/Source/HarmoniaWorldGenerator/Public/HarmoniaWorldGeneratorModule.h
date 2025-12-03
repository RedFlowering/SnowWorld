// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaWorldGeneratorModule.h
 * @brief HarmoniaWorldGenerator 모듈 인터페이스 정의
 * 
 * 절차적 월드 생성 기능을 제공하는 모듈입니다.
 * - Perlin Noise 기반 지형 생성
 * - Biome, 강, 호수, 도로 자동 생성
 * - 비동기 월드 생성 지원
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * @class FHarmoniaWorldGenerator
 * @brief HarmoniaWorldGenerator 모듈 클래스
 * 
 * 절차적 월드 생성 시스템의 진입점입니다.
 * UHarmoniaWorldGeneratorSubsystem을 통해 실제 기능에 접근합니다.
 * 
 * @see UHarmoniaWorldGeneratorSubsystem
 */
class FHarmoniaWorldGenerator : public FDefaultModuleImpl
{
public:
	/** @brief 모듈 시작 시 호출됩니다. */
	virtual void StartupModule() override;
	
	/** @brief 모듈 종료 시 호출됩니다. */
	virtual void ShutdownModule() override;
};
