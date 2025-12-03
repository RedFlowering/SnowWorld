// Copyright 2025 Snow Game Studio.

/**
 * HarmoniaModSystem Module Build Rules
 * 
 * 모드 시스템을 제공합니다:
 * - 모드 로드/언로드 관리
 * - 모드 의존성 해결
 * - Steam Workshop 통합
 * - 모드 버전 관리
 * - 모드 설정 UI
 */
using UnrealBuildTool;

public class HarmoniaModSystem: ModuleRules
{
    public HarmoniaModSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Core Dependencies
        // - Core/CoreUObject/Engine: 언리얼 핵심
        // - GameplayTags: 모드 태그 시스템
        // - Json/JsonUtilities: 모드 메타데이터 파싱
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameplayTags",
            "Json",
            "JsonUtilities"
        });

        // Private Dependencies
        // - Slate/SlateCore: 모드 설정 UI
        // - OnlineSubsystem: Steam Workshop 통합
        // - HarmoniaLoadManager: 모드 데이터 로드
        // - DeveloperSettings: 모드 설정
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "HarmoniaLoadManager",
            "DeveloperSettings"
        });
    }
}
