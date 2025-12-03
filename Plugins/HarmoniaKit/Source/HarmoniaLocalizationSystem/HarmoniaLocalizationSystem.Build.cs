// Copyright 2025 Snow Game Studio.

/**
 * HarmoniaLocalizationSystem Module Build Rules
 * 
 * 동적 다국어 지원 시스템을 제공합니다:
 * - 재시작 없는 언어 전환
 * - CSV/JSON 기반 텍스트 테이블
 * - 언어별 자동 폰트 전환
 * - 오디오 로컬리제이션
 * - RTL (Right-to-Left) 지원
 * - 누락 번역 감지
 */
using UnrealBuildTool;

public class HarmoniaLocalizationSystem: ModuleRules
{
    public HarmoniaLocalizationSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Core Dependencies
        // - Core/CoreUObject/Engine: 언리얼 핵심
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
        });

        // JSON parsing for localization files
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Json",
            "JsonUtilities",
        });
    }
}
