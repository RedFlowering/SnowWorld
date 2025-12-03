// Copyright 2025 Snow Game Studio.

/**
 * HarmoniaWorldGenerator Module Build Rules
 * 
 * 이 모듈은 절차적 월드 생성 시스템을 제공합니다.
 * - Perlin Noise 기반 지형 하이트맵 생성
 * - Biome, 강, 호수, 도로 자동 생성
 * - A* 경로 탐색 기반 도로/강 라우팅
 * - Poisson Disk Sampling을 이용한 오브젝트 배치
 * - 동굴 시스템, POI, 자원 분포 생성
 * - 환경 시스템 (계절, 날씨, 낮/밤 주기)
 * - 비동기 월드 생성 지원
 */
using UnrealBuildTool;

public class HarmoniaWorldGenerator : ModuleRules
{
    public HarmoniaWorldGenerator(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Core Dependencies
        // - Core/CoreUObject/Engine: 언리얼 핵심 기능
        // - Landscape: 지형 편집 및 Splatmap 시스템
        // - Foliage: 폴리지 배치 시스템
        // - HarmoniaLoadManager: 세이브/로드 인터페이스
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Landscape",
                "Foliage",
                "HarmoniaLoadManager",
            });

        // Render Dependencies (for heightmap texture generation)
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "RenderCore",
                "RHI",
            });

        // Editor-only modules for landscape editing
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "LandscapeEditor",
                    "UnrealEd",
                });
        }
    }
}
