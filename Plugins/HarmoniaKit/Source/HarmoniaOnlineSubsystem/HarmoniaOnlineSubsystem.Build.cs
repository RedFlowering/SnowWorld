// Copyright 2025 Snow Game Studio.

/**
 * HarmoniaOnlineSubsystem Module Build Rules
 * 
 * 온라인 서비스 통합을 제공합니다:
 * - Steam 및 EOS (Epic Online Services) 통합
 * - 멀티플레이어 세션 관리
 * - 음성 채팅 (EOS Voice Chat)
 * - 비동기 온라인 작업 처리
 * - 크로스 플랫폼 지원
 */
using UnrealBuildTool;

public class HarmoniaOnlineSubsystem: ModuleRules
{
    public HarmoniaOnlineSubsystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Core Dependencies
        // - Core/CoreUObject/Engine: 언리얼 핵심
        // - OnlineSubsystem: 온라인 서비스 인터페이스
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "OnlineSubsystem",
            "OnlineSubsystemUtils"
        });

        // Private Dependencies
        // - Sockets: 네트워크 통신
        // - Json/JsonUtilities: API 데이터 파싱
        // - HTTP: REST API 통신
        // - EOSVoiceChat: EOS 음성 채팅
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Sockets",
            "Json",
            "JsonUtilities",
            "HTTP",
            "EOSVoiceChat"
        });

        // Platform-specific Online Subsystems (Steam for PC/Mac/Linux)
        if (Target.Platform == UnrealTargetPlatform.Win64 ||
            Target.Platform == UnrealTargetPlatform.Linux ||
            Target.Platform == UnrealTargetPlatform.Mac)
        {
            PrivateDependencyModuleNames.Add("OnlineSubsystemSteam");
        }

        // EOS is cross-platform (PC, Console, Mobile)
        PrivateDependencyModuleNames.Add("OnlineSubsystemEOS");
    }
}
