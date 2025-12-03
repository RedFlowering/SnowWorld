// Copyright 2025 Snow Game Studio.

/**
 * HarmoniaStory Module Build Rules
 * 
 * 스토리 및 내러티브 시스템을 제공합니다:
 * - 대화 시스템 (Dialogue)
 * - 컷심 시스템 (Cutscene)
 * - 저널 시스템 (Journal)
 * - 관계 시스템 (Relationship)
 * - 퀴스트 통합
 */
using UnrealBuildTool;

public class HarmoniaStory : ModuleRules
{
	public HarmoniaStory(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Public Dependencies
		// - Core: 언리얼 핵심
		// - HarmoniaKit: 메인 플러그인 의존성
		// - HarmoniaLoadManager: 스토리 상태 저장
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"HarmoniaKit",
                "HarmoniaLoadManager",
			}
			);

		// Private Dependencies
		// - CoreUObject/Engine: 언리얼 핵심
		// - Slate/SlateCore: UI 시스템
		// - LevelSequence/MovieScene: 컷심 시스템
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "LevelSequence",
                "MovieScene",
			}
			);
	}
}
