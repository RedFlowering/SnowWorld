// Copyright 2025 Snow Game Studio.

/**
 * @file HarmoniaOnlineSubsystemModule.h
 * @brief HarmoniaOnlineSubsystem 모듈 인터페이스 정의
 * 
 * Steam, EOS 등 온라인 서비스 통합을 제공합니다.
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/**
 * @class FHarmoniaOnlineSubsystem
 * @brief HarmoniaOnlineSubsystem 모듈 클래스
 * 
 * 온라인 서비스 통합의 진입점입니다.
 * Steam, EOS, 음성 채팅 등의 기능을 제공합니다.
 * 
 * @see UHarmoniaOnlineSubsystem
 */
class FHarmoniaOnlineSubsystem : public FDefaultModuleImpl
{
public:
	/** @brief 모듈 시작 시 호출 */
	virtual void StartupModule() override;
	
	/** @brief 모듈 종료 시 호출 */
	virtual void ShutdownModule() override;
};
