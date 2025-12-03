// Copyright 2025 Snow Game Studio.

/**
 * @file PerlinNoiseHelper.cpp
 * @brief Perlin Noise 유틸리티 함수 구현
 * 
 * Ken Perlin의 개선된 노이즈 알고리즘을 기반으로 합니다.
 */

#include "PerlinNoiseHelper.h"

//=============================================================================
// Permutation Table
//=============================================================================

// Permutation table for Simplex noise (Ken Perlin's improved noise)
const uint8 PerlinNoiseHelper::Permutation[512] = {
	151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
	8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
	35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
	134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
	55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
	18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
	250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
	189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
	172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
	228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
	107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
	// Repeat the table to avoid wrapping
	151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
	8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
	35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
	134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
	55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
	18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
	250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
	189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
	172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
	228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
	107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

// ========================================
// Advanced Noise Function Implementations
// ========================================

float PerlinNoiseHelper::GetSimplexNoise(
	float X,
	float Y,
	int32 Seed,
	const FPerlinNoiseSettings& Settings)
{
	float Noise = 0.0f;
	float Amplitude = Settings.Amplitude;
	float Frequency = Settings.Frequency;

	for (int32 Octave = 0; Octave < Settings.Octaves; ++Octave)
	{
		const float OffsetX = (X + Seed * 10.0f) * Frequency;
		const float OffsetY = (Y + Seed * 20.0f) * Frequency;

		const float NoiseValue = SimplexNoise2D(OffsetX, OffsetY);
		Noise += NoiseValue * Amplitude;

		Amplitude *= Settings.Persistence;
		Frequency *= Settings.Lacunarity;
	}

	return FMath::Clamp(Noise, -1.0f, 1.0f);
}

float PerlinNoiseHelper::GetWorleyNoise(
	float X,
	float Y,
	int32 Seed,
	float CellSize,
	bool bInvert)
{
	// Scale coordinates to cell space
	const float ScaledX = X / CellSize;
	const float ScaledY = Y / CellSize;

	// Get integer cell coordinates
	const int32 CellX = FastFloor(ScaledX);
	const int32 CellY = FastFloor(ScaledY);

	// Find minimum distance to cell points
	float MinDistance = FLT_MAX;

	// Check 3x3 grid of cells around current cell
	for (int32 OffsetY = -1; OffsetY <= 1; ++OffsetY)
	{
		for (int32 OffsetX = -1; OffsetX <= 1; ++OffsetX)
		{
			const int32 CheckX = CellX + OffsetX;
			const int32 CheckY = CellY + OffsetY;

			// Get random point within this cell
			const FVector2D CellPoint = GetCellPoint(CheckX, CheckY, Seed);
			const FVector2D CellCenter(CheckX + CellPoint.X, CheckY + CellPoint.Y);

			// Calculate distance to this point
			const float DX = ScaledX - CellCenter.X;
			const float DY = ScaledY - CellCenter.Y;
			const float Distance = FMath::Sqrt(DX * DX + DY * DY);

			MinDistance = FMath::Min(MinDistance, Distance);
		}
	}

	// Normalize distance (assuming max distance is sqrt(2))
	float Result = FMath::Clamp(MinDistance / 1.414f, 0.0f, 1.0f);

	// Invert if requested (creates different patterns)
	if (bInvert)
	{
		Result = 1.0f - Result;
	}

	return Result;
}

float PerlinNoiseHelper::GetDomainWarpedNoise(
	float X,
	float Y,
	int32 Seed,
	const FPerlinNoiseSettings& Settings,
	float WarpStrength)
{
	// Generate warp offsets using noise
	const float WarpX = GetSimpleNoise(X * 0.5f, Y * 0.5f, Seed + 1) * WarpStrength * 100.0f;
	const float WarpY = GetSimpleNoise(X * 0.5f, Y * 0.5f, Seed + 2) * WarpStrength * 100.0f;

	// Sample noise at warped position
	return GetSimplexNoise(X + WarpX, Y + WarpY, Seed, Settings);
}

float PerlinNoiseHelper::GetFBMSimplex(
	float X,
	float Y,
	int32 Seed,
	const FPerlinNoiseSettings& Settings)
{
	float FBM = 0.0f;
	float Amplitude = Settings.Amplitude;
	float Frequency = Settings.Frequency;
	float MaxValue = 0.0f; // For normalization

	for (int32 Octave = 0; Octave < Settings.Octaves; ++Octave)
	{
		const float OffsetX = (X + Seed * 10.0f) * Frequency;
		const float OffsetY = (Y + Seed * 20.0f) * Frequency;

		const float NoiseValue = SimplexNoise2D(OffsetX, OffsetY);
		FBM += NoiseValue * Amplitude;
		MaxValue += Amplitude;

		Amplitude *= Settings.Persistence;
		Frequency *= Settings.Lacunarity;
	}

	// Normalize to [-1, 1]
	return MaxValue > 0.0f ? FBM / MaxValue : 0.0f;
}

float PerlinNoiseHelper::GetBillowyNoise(
	float X,
	float Y,
	int32 Seed,
	const FPerlinNoiseSettings& Settings)
{
	float Billowy = 0.0f;
	float Amplitude = Settings.Amplitude;
	float Frequency = Settings.Frequency;

	for (int32 Octave = 0; Octave < Settings.Octaves; ++Octave)
	{
		const float OffsetX = (X + Seed * 10.0f) * Frequency;
		const float OffsetY = (Y + Seed * 20.0f) * Frequency;

		const float NoiseValue = FMath::Abs(SimplexNoise2D(OffsetX, OffsetY));
		Billowy += NoiseValue * Amplitude;

		Amplitude *= Settings.Persistence;
		Frequency *= Settings.Lacunarity;
	}

	return FMath::Clamp(Billowy, 0.0f, 1.0f);
}

float PerlinNoiseHelper::GetCombinedNoise(
	float X,
	float Y,
	int32 Seed,
	const FPerlinNoiseSettings& Settings,
	float PerlinWeight,
	float SimplexWeight,
	float WorleyWeight)
{
	// Normalize weights
	const float TotalWeight = PerlinWeight + SimplexWeight + WorleyWeight;
	if (TotalWeight <= 0.0f)
	{
		return 0.0f;
	}

	const float NormPerlin = PerlinWeight / TotalWeight;
	const float NormSimplex = SimplexWeight / TotalWeight;
	const float NormWorley = WorleyWeight / TotalWeight;

	// Generate each noise type
	const float PerlinNoise = GetEarthLikeHeight(X, Y, Seed, Settings);
	const float SimplexNoise = GetSimplexNoise(X, Y, Seed + 100, Settings);
	const float WorleyNoise = (GetWorleyNoise(X, Y, Seed + 200, 100.0f, false) * 2.0f - 1.0f); // Convert to [-1,1]

	// Combine
	return PerlinNoise * NormPerlin + SimplexNoise * NormSimplex + WorleyNoise * NormWorley;
}

// ========================================
// Private Helper Functions
// ========================================

float PerlinNoiseHelper::SimplexNoise2D(float X, float Y)
{
	// Simplex noise constants
	const float F2 = 0.5f * (FMath::Sqrt(3.0f) - 1.0f);
	const float G2 = (3.0f - FMath::Sqrt(3.0f)) / 6.0f;

	// Skew input space to determine which simplex cell we're in
	const float S = (X + Y) * F2;
	const int32 I = FastFloor(X + S);
	const int32 J = FastFloor(Y + S);

	const float T = (I + J) * G2;
	const float X0 = X - (I - T);
	const float Y0 = Y - (J - T);

	// Determine which simplex we are in
	int32 I1, J1;
	if (X0 > Y0)
	{
		I1 = 1;
		J1 = 0;
	}
	else
	{
		I1 = 0;
		J1 = 1;
	}

	// Offsets for middle corner in (x,y) unskewed coords
	const float X1 = X0 - I1 + G2;
	const float Y1 = Y0 - J1 + G2;
	const float X2 = X0 - 1.0f + 2.0f * G2;
	const float Y2 = Y0 - 1.0f + 2.0f * G2;

	// Work out the hashed gradient indices
	const int32 II = I & 255;
	const int32 JJ = J & 255;
	const int32 GI0 = Permutation[II + Permutation[JJ]] % 12;
	const int32 GI1 = Permutation[II + I1 + Permutation[JJ + J1]] % 12;
	const int32 GI2 = Permutation[II + 1 + Permutation[JJ + 1]] % 12;

	// Calculate contribution from three corners
	float N0, N1, N2;

	float T0 = 0.5f - X0 * X0 - Y0 * Y0;
	if (T0 < 0.0f)
	{
		N0 = 0.0f;
	}
	else
	{
		T0 *= T0;
		N0 = T0 * T0 * GradientDot(GI0, X0, Y0);
	}

	float T1 = 0.5f - X1 * X1 - Y1 * Y1;
	if (T1 < 0.0f)
	{
		N1 = 0.0f;
	}
	else
	{
		T1 *= T1;
		N1 = T1 * T1 * GradientDot(GI1, X1, Y1);
	}

	float T2 = 0.5f - X2 * X2 - Y2 * Y2;
	if (T2 < 0.0f)
	{
		N2 = 0.0f;
	}
	else
	{
		T2 *= T2;
		N2 = T2 * T2 * GradientDot(GI2, X2, Y2);
	}

	// Add contributions and scale to [-1, 1]
	return 70.0f * (N0 + N1 + N2);
}

float PerlinNoiseHelper::GradientDot(int32 Hash, float X, float Y)
{
	// Convert hash to 12 gradient directions
	const int32 H = Hash & 7;
	const float U = H < 4 ? X : Y;
	const float V = H < 4 ? Y : X;
	return ((H & 1) ? -U : U) + ((H & 2) ? -2.0f * V : 2.0f * V);
}

FVector2D PerlinNoiseHelper::Hash2D(int32 X, int32 Y, int32 Seed)
{
	// Simple hash function for 2D coordinates
	const int32 Hash = X * 374761393 + Y * 668265263 + Seed;
	const int32 H1 = (Hash ^ (Hash >> 13)) * 1274126177;
	const int32 H2 = (H1 ^ (H1 >> 16)) * 1274126177;

	const float FX = (float)((H1 & 0xFFFF) / 65535.0f);
	const float FY = (float)((H2 & 0xFFFF) / 65535.0f);

	return FVector2D(FX, FY);
}

FVector2D PerlinNoiseHelper::GetCellPoint(int32 CellX, int32 CellY, int32 Seed)
{
	// Return random point within cell [0,1] x [0,1]
	return Hash2D(CellX, CellY, Seed);
}