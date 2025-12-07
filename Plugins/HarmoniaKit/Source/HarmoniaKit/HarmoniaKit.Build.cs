// Copyright 2025 Snow Game Studio.

/**
 * HarmoniaKit Core Module Build Rules
 * 
 * HarmoniaKit 플러그인의 핵심 모듈로, 다음 시스템들을 포함합니다:
 * - Gameplay Ability System (GAS) 기반 전투 시스템
 * - 인벤토리 및 아이템 시스템
 * - 제작 및 강화 시스템
 * - AI 및 스쿼드 시스템
 * - 이코노미 및 거래 시스템
 * - 애니메이션 및 UI 시스템
 */
using UnrealBuildTool;

public class HarmoniaKit : ModuleRules
{
    public HarmoniaKit(ReadOnlyTargetRules Target) : base(Target)
    {
        // Core Engine Dependencies
        // - Core/CoreUObject/Engine: 언리얼 핵심
        // - InputCore/EnhancedInput: 입력 시스템
        // - UMG/Slate/SlateCore: UI 시스템
        // - CommonUI: 공통 UI 위젯
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "CoreOnline",
                "Engine",
                "InputCore",
                "EnhancedInput",
                "UMG",
                "Slate",
                "SlateCore",
                "NetCore", // For FastArraySerializer
                
                // Harmonia Plugin Dependencies
                "HarmoniaLoadManager",     // 세이브/로드 시스템
                "HarmoniaWorldGenerator",  // 월드 생성 시스템
                
                // Gameplay Systems
                "SenseSystem",             // Sense 기반 상호작용
                "GameplayAbilities",       // GAS 핵심
                "GameplayTags",            // 게임플레이 태그
                "GameplayTasks",           // 게임플레이 태스크
                "LyraGame",                // Lyra 기반 클래스
                "GameplayMessageRuntime",  // 게임플레이 메시지 시스템
                "ModularGameplay",         // ModularPlayerState
                "ModularGameplayActors",   // Modular Actors
                "ALS",                     // Advanced Locomotion System
                
                // Online & Navigation
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "CommonUI",
                "NavigationSystem",
                "AIModule",
                
                // Rendering & VFX
                "RenderCore",              // Fog of War 렌더링
                "Niagara",                 // VFX 시스템
                
                // Cinematic
                "LevelSequence",           // 컷심
                "MovieScene"               // 컷심
            });
    }
}
