// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "WorldGeneratorTypes.h"

/**
 * 퍼린 노이즈, 랜덤 관련 헬퍼 함수
 */
class PerlinNoiseHelper
{
public:
	// 여러 옥타브 조합으로 Earth-like 노이즈 리턴 (자연스러운 지형)
	static float GetEarthLikeHeight(float X, float Y, int32 Seed, const FPerlinNoiseSettings& Settings)
	{
		float h = 0.f;
		float amplitude = Settings.Amplitude;
		float frequency = Settings.Frequency;

		for (int32 i = 0; i < Settings.Octaves; ++i)
		{
			float n = FMath::PerlinNoise2D(FVector2D((X + Seed * 10) * frequency, (Y + Seed * 20) * frequency));
			h += n * amplitude;

			amplitude *= Settings.Persistence;
			frequency *= Settings.Lacunarity;
		}

		// 정규화: 보통 -1~1 범위로 (오버/언더슈팅 방지)
		h = FMath::Clamp(h / 2.0f, -1.0f, 1.0f);

		return h;
	}
};