// Copyright 2025 Snow Game Studio.

#include "HarmoniaWorldGeneratorSubsystem.h"
#include "WorldGeneratorTypes.h"
#include "PerlinNoiseHelper.h"
#include "Landscape.h"
#include "LandscapeProxy.h"

void UHarmoniaWorldGeneratorSubsystem::GenerateWorld(
    const FWorldGeneratorConfig& Config,
    TArray<int32>& OutHeightData,
    TArray<FWorldObjectData>& OutObjects,
    TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap)
{
    // Generate heightmap first
    GenerateHeightmap(Config, OutHeightData);

    // Generate objects based on heightmap
    GenerateObjects(Config, OutHeightData, ActorClassMap, OutObjects);
}

void UHarmoniaWorldGeneratorSubsystem::GenerateHeightmapOnly(
    const FWorldGeneratorConfig& Config,
    TArray<int32>& OutHeightData)
{
    GenerateHeightmap(Config, OutHeightData);
}

void UHarmoniaWorldGeneratorSubsystem::GenerateObjectsInRegion(
    const FWorldGeneratorConfig& Config,
    const TArray<int32>& HeightData,
    TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap,
    int32 RegionMinX,
    int32 RegionMinY,
    int32 RegionMaxX,
    int32 RegionMaxY,
    TArray<FWorldObjectData>& OutObjects)
{
    if (HeightData.Num() != Config.SizeX * Config.SizeY)
    {
        UE_LOG(LogTemp, Error, TEXT("HeightData size mismatch! Expected %d, got %d"),
            Config.SizeX * Config.SizeY, HeightData.Num());
        return;
    }

    OutObjects.Empty();

    // Clamp region bounds
    RegionMinX = FMath::Clamp(RegionMinX, 0, Config.SizeX - 1);
    RegionMinY = FMath::Clamp(RegionMinY, 0, Config.SizeY - 1);
    RegionMaxX = FMath::Clamp(RegionMaxX, 0, Config.SizeX - 1);
    RegionMaxY = FMath::Clamp(RegionMaxY, 0, Config.SizeY - 1);

    // Initialize random stream with seed
    FRandomStream Random(Config.Seed);

    // Calculate expected object count for region
    int32 RegionArea = (RegionMaxX - RegionMinX + 1) * (RegionMaxY - RegionMinY + 1);
    int32 ExpectedObjects = FMath::CeilToInt(RegionArea * Config.ObjectDensity);
    OutObjects.Reserve(ExpectedObjects);

    // Generate objects in region
    for (int32 Y = RegionMinY; Y <= RegionMaxY; ++Y)
    {
        for (int32 X = RegionMinX; X <= RegionMaxX; ++X)
        {
            // Use location-based seed for deterministic placement
            int32 LocationSeed = Config.Seed + X * 73856093 + Y * 19349663;
            Random.Initialize(LocationSeed);

            if (!IsValidObjectLocation(X, Y, HeightData, Config))
            {
                continue;
            }

            // Check spawn probability
            if (Random.FRand() >= Config.ObjectDensity)
            {
                continue;
            }

            // Pick object type
            EWorldObjectType ObjType = PickObjectType(Config.ObjectTypeProbabilities, Random);
            if (ObjType == EWorldObjectType::None)
            {
                continue;
            }

            // Create object data
            float HeightNorm = (float)HeightData[Y * Config.SizeX + X] / 65535.f;
            FVector Location(X * 100.f, Y * 100.f, HeightNorm * Config.MaxHeight);

            FWorldObjectData ObjData;
            ObjData.ObjectType = ObjType;
            ObjData.ActorClass = ActorClassMap.Contains(ObjType) ? ActorClassMap[ObjType] : nullptr;
            ObjData.Location = Location;
            ObjData.Rotation = FRotator(0.f, Random.FRandRange(0.f, 360.f), 0.f);
            ObjData.Scale = FVector(Random.FRandRange(0.8f, 1.2f));

            OutObjects.Add(ObjData);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Generated %d objects in region (%d,%d) to (%d,%d)"),
        OutObjects.Num(), RegionMinX, RegionMinY, RegionMaxX, RegionMaxY);
}

void UHarmoniaWorldGeneratorSubsystem::GenerateHeightmap(
    const FWorldGeneratorConfig& Config,
    TArray<int32>& OutHeightData)
{
    const int32 TotalSize = Config.SizeX * Config.SizeY;
    OutHeightData.SetNumUninitialized(TotalSize);

    const int32 ChunkSize = FMath::Max(1, Config.ChunkSize);
    const int32 ChunksX = FMath::DivideAndRoundUp(Config.SizeX, ChunkSize);
    const int32 ChunksY = FMath::DivideAndRoundUp(Config.SizeY, ChunkSize);
    const int32 TotalChunks = ChunksX * ChunksY;

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generating heightmap: %dx%d (%d total chunks, chunk size: %d)"),
            Config.SizeX, Config.SizeY, TotalChunks, ChunkSize);
    }

    int32 ProcessedChunks = 0;

    // Process chunks
    for (int32 ChunkY = 0; ChunkY < ChunksY; ++ChunkY)
    {
        for (int32 ChunkX = 0; ChunkX < ChunksX; ++ChunkX)
        {
            // Calculate chunk bounds
            const int32 StartX = ChunkX * ChunkSize;
            const int32 StartY = ChunkY * ChunkSize;
            const int32 EndX = FMath::Min(StartX + ChunkSize, Config.SizeX);
            const int32 EndY = FMath::Min(StartY + ChunkSize, Config.SizeY);

            // Generate heightmap for this chunk
            for (int32 Y = StartY; Y < EndY; ++Y)
            {
                for (int32 X = StartX; X < EndX; ++X)
                {
                    const float Height = CalculateHeightAtLocation(X, Y, Config);

                    // Clamp and convert to 16-bit heightmap value
                    const int32 HeightVal = FMath::Clamp(
                        FMath::RoundToInt(Height * 65535.f),
                        0,
                        65535
                    );

                    OutHeightData[Y * Config.SizeX + X] = HeightVal;
                }
            }

            ProcessedChunks++;

            // Progress logging (every 10% or so)
            if (Config.bEnableProgressLogging && ProcessedChunks % FMath::Max(1, TotalChunks / 10) == 0)
            {
                const float Progress = (float)ProcessedChunks / (float)TotalChunks * 100.f;
                UE_LOG(LogTemp, Log, TEXT("Heightmap generation progress: %.1f%% (%d/%d chunks)"),
                    Progress, ProcessedChunks, TotalChunks);
            }
        }
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Heightmap generation complete!"));
    }
}

void UHarmoniaWorldGeneratorSubsystem::GenerateObjects(
    const FWorldGeneratorConfig& Config,
    const TArray<int32>& HeightData,
    TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap,
    TArray<FWorldObjectData>& OutObjects)
{
    if (HeightData.Num() != Config.SizeX * Config.SizeY)
    {
        UE_LOG(LogTemp, Error, TEXT("HeightData size mismatch!"));
        return;
    }

    OutObjects.Empty();

    // Calculate approximate number of objects to reserve memory
    const int32 TotalTiles = Config.SizeX * Config.SizeY;
    const int32 ApproxObjectCount = FMath::CeilToInt(TotalTiles * Config.ObjectDensity);
    OutObjects.Reserve(ApproxObjectCount);

    const int32 ChunkSize = FMath::Max(1, Config.ChunkSize);
    const int32 ChunksX = FMath::DivideAndRoundUp(Config.SizeX, ChunkSize);
    const int32 ChunksY = FMath::DivideAndRoundUp(Config.SizeY, ChunkSize);
    const int32 TotalChunks = ChunksX * ChunksY;

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generating objects: %d chunks (expected ~%d objects)"),
            TotalChunks, ApproxObjectCount);
    }

    int32 ProcessedChunks = 0;

    // Process chunks
    for (int32 ChunkY = 0; ChunkY < ChunksY; ++ChunkY)
    {
        for (int32 ChunkX = 0; ChunkX < ChunksX; ++ChunkX)
        {
            const int32 StartX = ChunkX * ChunkSize;
            const int32 StartY = ChunkY * ChunkSize;
            const int32 EndX = FMath::Min(StartX + ChunkSize, Config.SizeX);
            const int32 EndY = FMath::Min(StartY + ChunkSize, Config.SizeY);

            // Generate objects in this chunk
            for (int32 Y = StartY; Y < EndY; ++Y)
            {
                for (int32 X = StartX; X < EndX; ++X)
                {
                    // Use location-based seed for deterministic placement
                    int32 LocationSeed = Config.Seed + X * 73856093 + Y * 19349663;
                    FRandomStream Random(LocationSeed);

                    if (!IsValidObjectLocation(X, Y, HeightData, Config))
                    {
                        continue;
                    }

                    // Check spawn probability
                    if (Random.FRand() >= Config.ObjectDensity)
                    {
                        continue;
                    }

                    // Pick object type
                    EWorldObjectType ObjType = PickObjectType(Config.ObjectTypeProbabilities, Random);
                    if (ObjType == EWorldObjectType::None)
                    {
                        continue;
                    }

                    // Create object data
                    float HeightNorm = (float)HeightData[Y * Config.SizeX + X] / 65535.f;
                    FVector Location(X * 100.f, Y * 100.f, HeightNorm * Config.MaxHeight);

                    FWorldObjectData ObjData;
                    ObjData.ObjectType = ObjType;
                    ObjData.ActorClass = ActorClassMap.Contains(ObjType) ? ActorClassMap[ObjType] : nullptr;
                    ObjData.Location = Location;
                    ObjData.Rotation = FRotator(0.f, Random.FRandRange(0.f, 360.f), 0.f);
                    ObjData.Scale = FVector(Random.FRandRange(0.8f, 1.2f));

                    OutObjects.Add(ObjData);
                }
            }

            ProcessedChunks++;

            // Progress logging
            if (Config.bEnableProgressLogging && ProcessedChunks % FMath::Max(1, TotalChunks / 10) == 0)
            {
                const float Progress = (float)ProcessedChunks / (float)TotalChunks * 100.f;
                UE_LOG(LogTemp, Log, TEXT("Object generation progress: %.1f%% (%d/%d chunks, %d objects)"),
                    Progress, ProcessedChunks, TotalChunks, OutObjects.Num());
            }
        }
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Object generation complete! Total objects: %d"), OutObjects.Num());
    }
}

EWorldObjectType UHarmoniaWorldGeneratorSubsystem::PickObjectType(
    const TMap<EWorldObjectType, float>& ProbMap,
    FRandomStream& Random)
{
    if (ProbMap.Num() == 0)
    {
        return EWorldObjectType::None;
    }

    // Calculate total weight
    float TotalWeight = 0.f;
    for (const auto& Elem : ProbMap)
    {
        TotalWeight += Elem.Value;
    }

    if (TotalWeight <= 0.f)
    {
        return EWorldObjectType::None;
    }

    // Pick random value
    const float RandValue = Random.FRandRange(0.f, TotalWeight);
    float Cumulative = 0.f;

    // Find matching type
    for (const auto& Elem : ProbMap)
    {
        Cumulative += Elem.Value;
        if (RandValue <= Cumulative)
        {
            return Elem.Key;
        }
    }

    // Fallback to first element
    return ProbMap.CreateConstIterator()->Key;
}

float UHarmoniaWorldGeneratorSubsystem::CalculateHeightAtLocation(
    float X,
    float Y,
    const FWorldGeneratorConfig& Config)
{
    // Normalize coordinates
    const float NormX = X / FMath::Max(1.f, (float)(Config.SizeX - 1));
    const float NormY = Y / FMath::Max(1.f, (float)(Config.SizeY - 1));

    // Generate height using Perlin noise
    const float HeightNorm = PerlinNoiseHelper::GetEarthLikeHeight(
        NormX * 10.f,
        NormY * 10.f,
        Config.Seed,
        Config.NoiseSettings
    );

    // Convert from [-1, 1] to [0, 1]
    float Height01 = (HeightNorm + 1.f) * 0.5f;

    // Apply sea level (minimum height)
    Height01 = FMath::Max(Height01, Config.SeaLevel);

    // Clamp to valid range
    return FMath::Clamp(Height01, 0.f, 1.f);
}

bool UHarmoniaWorldGeneratorSubsystem::IsValidObjectLocation(
    int32 X,
    int32 Y,
    const TArray<int32>& HeightData,
    const FWorldGeneratorConfig& Config)
{
    // Check bounds
    if (X < 0 || X >= Config.SizeX || Y < 0 || Y >= Config.SizeY)
    {
        return false;
    }

    // Get height at this location
    const float HeightNorm = (float)HeightData[Y * Config.SizeX + X] / 65535.f;

    // Must be above sea level with some margin
    const float MinHeight = Config.SeaLevel + 0.02f;
    if (HeightNorm <= MinHeight)
    {
        return false;
    }

    // Don't place on very steep slopes (optional)
    // You can add slope calculation here if needed

    return true;
}
