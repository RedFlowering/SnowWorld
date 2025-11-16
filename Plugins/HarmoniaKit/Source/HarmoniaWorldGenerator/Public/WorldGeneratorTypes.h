// Copyright 2025 Snow Game Studio.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WorldGeneratorTypes.generated.h"

UENUM(BlueprintType)
enum class EWorldObjectType : uint8
{
	None        UMETA(DisplayName = "None"),
	Tree        UMETA(DisplayName = "Tree"),
	Rock        UMETA(DisplayName = "Rock"),
	Resource    UMETA(DisplayName = "Resource"),
	Structure   UMETA(DisplayName = "Structure"),
};

USTRUCT(BlueprintType)
struct FWorldObjectData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AActor> ActorClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWorldObjectType ObjectType = EWorldObjectType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Scale = FVector::OneVector;
};

USTRUCT(BlueprintType)
struct FPerlinNoiseSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	int32 Octaves = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Persistence = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Lacunarity = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Frequency = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float Amplitude = 1.0f;
};

/**
 * World Generation Configuration
 * Seed-based generation ensures same world in multiplayer
 */
USTRUCT(BlueprintType)
struct FWorldGeneratorConfig
{
	GENERATED_BODY()

	// Seed for deterministic world generation (multiplayer sync)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	int32 Seed = 0;

	// World size (recommend power of 2)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	int32 SizeX = 512;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	int32 SizeY = 512;

	// Maximum terrain height
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	float MaxHeight = 1000.f;

	// Object spawn density (0.0 - 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	float ObjectDensity = 0.008f;

	// Sea level threshold (0.0 - 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	float SeaLevel = 0.42f;

	// Object type spawn probabilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	TMap<EWorldObjectType, float> ObjectTypeProbabilities;

	// Noise generation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen")
	FPerlinNoiseSettings NoiseSettings;

	// Chunk size for generation (prevents infinite loop detection)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Performance")
	int32 ChunkSize = 64;

	// Max chunks to process per frame (0 = all at once)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Performance")
	int32 MaxChunksPerFrame = 0;

	// Enable progress logging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Debug")
	bool bEnableProgressLogging = false;
};
