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

	// ========================================
	// Advanced Noise Functions
	// ========================================

	/**
	 * Generate Simplex Noise (improved Perlin noise)
	 * Better performance and visual quality than Perlin
	 * @param X - X coordinate
	 * @param Y - Y coordinate
	 * @param Seed - Random seed
	 * @param Settings - Noise generation parameters
	 * @return Noise value in range [-1, 1]
	 */
	static float GetSimplexNoise(
		float X,
		float Y,
		int32 Seed,
		const FPerlinNoiseSettings& Settings);

	/**
	 * Generate Worley Noise (Cellular/Voronoi)
	 * Creates cellular patterns useful for stone, cracks, etc.
	 * @param X - X coordinate
	 * @param Y - Y coordinate
	 * @param Seed - Random seed
	 * @param CellSize - Size of each cell
	 * @param bInvert - Whether to invert the result (for different patterns)
	 * @return Noise value in range [0, 1]
	 */
	static float GetWorleyNoise(
		float X,
		float Y,
		int32 Seed,
		float CellSize = 100.0f,
		bool bInvert = false);

	/**
	 * Generate Domain Warped Noise
	 * Warps the noise domain to create more organic patterns
	 * @param X - X coordinate
	 * @param Y - Y coordinate
	 * @param Seed - Random seed
	 * @param Settings - Noise generation parameters
	 * @param WarpStrength - How much to warp the domain (0-1)
	 * @return Noise value in range [-1, 1]
	 */
	static float GetDomainWarpedNoise(
		float X,
		float Y,
		int32 Seed,
		const FPerlinNoiseSettings& Settings,
		float WarpStrength = 0.5f);

	/**
	 * Generate Fractal Brownian Motion (FBM) using Simplex
	 * Higher quality multi-octave noise
	 */
	static float GetFBMSimplex(
		float X,
		float Y,
		int32 Seed,
		const FPerlinNoiseSettings& Settings);

	/**
	 * Generate Billowy Noise (absolute Simplex)
	 * Creates cloud-like or billowy terrain
	 */
	static float GetBillowyNoise(
		float X,
		float Y,
		int32 Seed,
		const FPerlinNoiseSettings& Settings);

	/**
	 * Combine multiple noise types with weights
	 * Allows creating complex terrain by mixing noise functions
	 */
	static float GetCombinedNoise(
		float X,
		float Y,
		int32 Seed,
		const FPerlinNoiseSettings& Settings,
		float PerlinWeight = 0.5f,
		float SimplexWeight = 0.3f,
		float WorleyWeight = 0.2f);

private:
	/**
	 * Simplex noise 2D implementation
	 * Based on Stefan Gustavson's implementation
	 */
	static float SimplexNoise2D(float X, float Y);

	/**
	 * Hash function for Worley noise
	 */
	static FVector2D Hash2D(int32 X, int32 Y, int32 Seed);

	/**
	 * Get cell point for Worley noise
	 */
	static FVector2D GetCellPoint(int32 CellX, int32 CellY, int32 Seed);

	/**
	 * Fast floor function
	 */
	static int32 FastFloor(float Value)
	{
		return Value > 0 ? (int32)Value : (int32)Value - 1;
	}

	/**
	 * Gradient dot product for Simplex noise
	 */
	static float GradientDot(int32 Hash, float X, float Y);

	// Simplex noise permutation table
	static const uint8 Permutation[512];
};
