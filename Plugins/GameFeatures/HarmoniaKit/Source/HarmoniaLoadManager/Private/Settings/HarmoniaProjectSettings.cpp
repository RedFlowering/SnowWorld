// Copyright 2025 Snow Game Studio.

#include "Settings/HarmoniaProjectSettings.h"
#include "Macro/HarmoniaMacroGenerator.h"

#if WITH_EDITOR
void UHarmoniaProjectSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // 프로젝트 세팅 변경 시 자동 매크로 생성
    FHarmoniaMacroGenerator::GenerateMacroHeaderFromConfig();
    FHarmoniaMacroGenerator::GenerateFunctionLibraryFromConfig();
}
#endif