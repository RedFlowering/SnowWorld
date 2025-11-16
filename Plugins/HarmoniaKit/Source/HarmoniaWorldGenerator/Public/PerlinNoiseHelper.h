// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "WorldGeneratorTypes.h"

/**
 * Perlin Noise Helper for World Generation
 * Provides optimized noise generation functions
 */
class PerlinNoiseHelper
{
public:
	/**
	 * Generate earth-like terrain height using multi-octave Perlin noise
	 * @param X - World X coordinate
	 * @param Y - World Y coordinate
	 * @param Seed - Random seed for deterministic generation
	 * @param Settings - Noise generation parameters
	 * @return Height value in range [-1, 1]
	 */
	static float GetEarthLikeHeight(
		float X,
		float Y,
		int32 Seed,
		const FPerlinNoiseSettings& Settings)
	{
		float Height = 0.f;
		float Amplitude = Settings.Amplitude;
		float Frequency = Settings.Frequency;

		// Multi-octave noise generation
		for (int32 Octave = 0; Octave < Settings.Octaves; ++Octave)
		{
			// Offset by seed to ensure different worlds
			const float OffsetX = (X + Seed * 10.f) * Frequency;
			const float OffsetY = (Y + Seed * 20.f) * Frequency;

			// Generate Perlin noise for this octave
			const float NoiseValue = FMath::PerlinNoise2D(FVector2D(OffsetX, OffsetY));

			// Accumulate weighted noise
			Height += NoiseValue * Amplitude;

			// Update amplitude and frequency for next octave
			Amplitude *= Settings.Persistence;
			Frequency *= Settings.Lacunarity;
		}

		// Normalize to [-1, 1] range
		// Perlin noise typically outputs [-1, 1], but multi-octave can exceed this
		Height = FMath::Clamp(Height / 2.0f, -1.0f, 1.0f);

		return Height;
	}

	/**
	 * Generate simple Perlin noise (single octave)
	 * @param X - X coordinate
	 * @param Y - Y coordinate
	 * @param Seed - Random seed
	 * @return Noise value in range [-1, 1]
	 */
	static float GetSimpleNoise(float X, float Y, int32 Seed)
	{
		return FMath::PerlinNoise2D(FVector2D(X + Seed * 10.f, Y + Seed * 20.f));
	}

	/**
	 * Generate turbulence noise (absolute value of noise)
	 * Useful for creating rocky/rough terrain features
	 */
	static float GetTurbulence(
		float X,
		float Y,
		int32 Seed,
		const FPerlinNoiseSettings& Settings)
	{
		float Turbulence = 0.f;
		float Amplitude = Settings.Amplitude;
		float Frequency = Settings.Frequency;

		for (int32 Octave = 0; Octave < Settings.Octaves; ++Octave)
		{
			const float OffsetX = (X + Seed * 10.f) * Frequency;
			const float OffsetY = (Y + Seed * 20.f) * Frequency;

			const float NoiseValue = FMath::Abs(FMath::PerlinNoise2D(FVector2D(OffsetX, OffsetY)));
			Turbulence += NoiseValue * Amplitude;

			Amplitude *= Settings.Persistence;
			Frequency *= Settings.Lacunarity;
		}

		return FMath::Clamp(Turbulence, 0.f, 1.f);
	}

	/**
	 * Generate ridged noise (inverted absolute noise)
	 * Useful for mountain ridges
	 */
	static float GetRidgedNoise(
		float X,
		float Y,
		int32 Seed,
		const FPerlinNoiseSettings& Settings)
	{
		float Ridge = 0.f;
		float Amplitude = Settings.Amplitude;
		float Frequency = Settings.Frequency;

		for (int32 Octave = 0; Octave < Settings.Octaves; ++Octave)
		{
			const float OffsetX = (X + Seed * 10.f) * Frequency;
			const float OffsetY = (Y + Seed * 20.f) * Frequency;

			const float NoiseValue = 1.f - FMath::Abs(FMath::PerlinNoise2D(FVector2D(OffsetX, OffsetY)));
			Ridge += NoiseValue * Amplitude;

			Amplitude *= Settings.Persistence;
			Frequency *= Settings.Lacunarity;
		}

		return FMath::Clamp(Ridge, 0.f, 1.f);
	}
};
