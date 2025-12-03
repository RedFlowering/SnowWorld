// Copyright 2025 Snow Game Studio.

/**
 * HarmoniaLoadManager Module Build Rules
 * 
 * 데이터 테이블 및 세이브/로드 시스템을 제공합니다:
 * - 레지스트리 기반 데이터 테이블 관리
 * - 비동기 데이터 로드
 * - 매크로 기반 코드 생성 유틸리티
 * - 세이브 인터페이스 정의
 */
using System.IO;
using UnrealBuildTool;

public class HarmoniaLoadManager : ModuleRules
{
    public HarmoniaLoadManager(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // Include private macro headers for external access
        PublicIncludePaths.AddRange(
            new string[]
            {
                Path.Combine(ModuleDirectory, "Private/Macro")
            }
            );

        // Core Dependencies
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
            );

        // Private Dependencies
        // - CoreUObject/Engine: 언리얼 핵심
        // - Projects: 프로젝트 설정 접근
        // - DeveloperSettings: 런타임 설정
        // - GameplayTags: 태그 기반 필터링
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Projects",
                "DeveloperSettings",
                "GameplayTags",
            }
            );
    }
}