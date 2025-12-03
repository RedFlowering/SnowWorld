// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaLoadManagerModule.h
 * @brief HarmoniaLoadManager 모듈 인터페이스 정의
 * 
 * 데이터 테이블 관리 및 세이브 시스템의 핵심 모듈입니다.
 */

#pragma once

#include "Modules/ModuleManager.h"

/**
 * @class FHarmoniaLoadManagerModule
 * @brief HarmoniaLoadManager 모듈 클래스
 * 
 * 레지스트리 기반 데이터 테이블 관리와
 * 세이브/로드 시스템의 진입점입니다.
 * 
 * @see UHarmoniaLoadManager
 * @see IHarmoniaSaveInterface
 */
class FHarmoniaLoadManagerModule : public IModuleInterface
{
public:
	/** @brief 모듈 시작 시 호출 */
	virtual void StartupModule() override;
	
	/** @brief 모듈 종료 시 호출 */
	virtual void ShutdownModule() override;
};