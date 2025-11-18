// Copyright 2025 Snow Game Studio.

#include "HarmoniaWorldGeneratorSubsystem.h"
#include "WorldGeneratorTypes.h"
#include "PerlinNoiseHelper.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "Async/Async.h"
#include "Async/AsyncWork.h"


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

    // Check slope for structure placement
    if (X > 0 && X < Config.SizeX - 1 && Y > 0 && Y < Config.SizeY - 1)
    {
        // Get neighboring heights
        const float HeightLeft = (float)HeightData[Y * Config.SizeX + (X - 1)] / 65535.f;
        const float HeightRight = (float)HeightData[Y * Config.SizeX + (X + 1)] / 65535.f;
        const float HeightUp = (float)HeightData[(Y - 1) * Config.SizeX + X] / 65535.f;
        const float HeightDown = (float)HeightData[(Y + 1) * Config.SizeX + X] / 65535.f;

        // Calculate slope
        const float SlopeX = FMath::Abs(HeightRight - HeightLeft) * Config.MaxHeight / 200.f; // 200 = 2 tiles * 100 UE units
        const float SlopeY = FMath::Abs(HeightDown - HeightUp) * Config.MaxHeight / 200.f;
        const float Slope = FMath::Sqrt(SlopeX * SlopeX + SlopeY * SlopeY);

        // Convert to degrees
        const float SlopeDegrees = FMath::RadiansToDegrees(FMath::Atan(Slope));

        if (SlopeDegrees > Config.MaxStructureSlope)
        {
            return false;
        }
    }

    return true;
}

float UHarmoniaWorldGeneratorSubsystem::CalculateFlatness(
    int32 X,
    int32 Y,
    int32 Radius,
    const TArray<int32>& HeightData,
    const FWorldGeneratorConfig& Config)
{
    if (X < Radius || X >= Config.SizeX - Radius ||
        Y < Radius || Y >= Config.SizeY - Radius)
    {
        return 0.f; // Edge areas are not flat enough
    }

    const int32 CenterIndex = Y * Config.SizeX + X;
    const float CenterHeight = (float)HeightData[CenterIndex];

    float TotalVariance = 0.f;
    int32 SampleCount = 0;

    // Check surrounding area
    for (int32 dy = -Radius; dy <= Radius; ++dy)
    {
        for (int32 dx = -Radius; dx <= Radius; ++dx)
        {
            const int32 SampleX = X + dx;
            const int32 SampleY = Y + dy;
            const int32 SampleIndex = SampleY * Config.SizeX + SampleX;

            const float SampleHeight = (float)HeightData[SampleIndex];
            const float Variance = FMath::Abs(SampleHeight - CenterHeight);

            TotalVariance += Variance;
            SampleCount++;
        }
    }

    const float AvgVariance = TotalVariance / FMath::Max(1, SampleCount);

    // Normalize to 0-1 (0 = very steep, 1 = perfectly flat)
    // Max expected variance for "not flat" is about 5000 (out of 65535)
    const float Flatness = 1.f - FMath::Clamp(AvgVariance / 5000.f, 0.f, 1.f);

    return Flatness;
}

bool UHarmoniaWorldGeneratorSubsystem::IsFlatEnoughForStructure(
    int32 X,
    int32 Y,
    const TArray<int32>& HeightData,
    const FWorldGeneratorConfig& Config)
{
    const float Flatness = CalculateFlatness(X, Y, 3, HeightData, Config);
    return Flatness >= Config.StructureGroupSettings.MinFlatness;
}

void UHarmoniaWorldGeneratorSubsystem::GenerateBiomeMap(
    const FWorldGeneratorConfig& Config,
    const TArray<int32>& HeightData,
    TArray<FBiomeData>& OutBiomeData)
{
    if (!Config.bEnableBiomes)
    {
        OutBiomeData.Empty();
        return;
    }

    const int32 TotalSize = Config.SizeX * Config.SizeY;
    OutBiomeData.SetNumUninitialized(TotalSize);

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generating biome map: %dx%d"), Config.SizeX, Config.SizeY);
    }

    // Generate biome data for each tile
    for (int32 Y = 0; Y < Config.SizeY; ++Y)
    {
        for (int32 X = 0; X < Config.SizeX; ++X)
        {
            const int32 Index = Y * Config.SizeX + X;

            // Get height
            const float Height = (float)HeightData[Index] / 65535.f;

            // Calculate temperature and moisture
            const float Temperature = CalculateTemperature(X, Y, Height, Config);
            const float Moisture = CalculateMoisture(X, Y, Config);

            // Determine biome type
            const EBiomeType BiomeType = DetermineBiomeType(Temperature, Moisture, Height, Config);

            // Create biome data
            FBiomeData& BiomeData = OutBiomeData[Index];
            BiomeData.X = X;
            BiomeData.Y = Y;
            BiomeData.BiomeType = BiomeType;
            BiomeData.Temperature = Temperature;
            BiomeData.Moisture = Moisture;
            BiomeData.Height = Height;
        }
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Biome map generation complete!"));
    }
}

float UHarmoniaWorldGeneratorSubsystem::CalculateTemperature(
    float X,
    float Y,
    float Height,
    const FWorldGeneratorConfig& Config)
{
    // Base temperature from latitude (Y position)
    const float NormY = Y / FMath::Max(1.f, (float)(Config.SizeY - 1));
    const float LatitudeTemp = 1.0f - FMath::Abs(NormY - 0.5f) * 2.0f; // Warmer at equator (center)

    // Temperature noise for variation
    const float TempNoise = PerlinNoiseHelper::GetSimpleNoise(
        X * Config.TemperatureNoiseSettings.Frequency * 0.01f,
        Y * Config.TemperatureNoiseSettings.Frequency * 0.01f,
        Config.Seed + 1000
    );

    // Convert from [-1, 1] to [0, 1]
    const float NoiseContribution = (TempNoise + 1.f) * 0.5f;

    // Height affects temperature (higher = colder)
    const float HeightFactor = 1.0f - (Height * 0.6f);

    // Combine factors
    float Temperature = (LatitudeTemp * 0.5f + NoiseContribution * 0.3f) * HeightFactor + 0.2f;

    return FMath::Clamp(Temperature, 0.f, 1.f);
}

float UHarmoniaWorldGeneratorSubsystem::CalculateMoisture(
    float X,
    float Y,
    const FWorldGeneratorConfig& Config)
{
    // Moisture from noise
    const float MoistureNoise = PerlinNoiseHelper::GetSimpleNoise(
        X * Config.MoistureNoiseSettings.Frequency * 0.01f,
        Y * Config.MoistureNoiseSettings.Frequency * 0.01f,
        Config.Seed + 2000
    );

    // Convert from [-1, 1] to [0, 1]
    const float Moisture = (MoistureNoise + 1.f) * 0.5f;

    return FMath::Clamp(Moisture, 0.f, 1.f);
}

EBiomeType UHarmoniaWorldGeneratorSubsystem::DetermineBiomeType(
    float Temperature,
    float Moisture,
    float Height,
    const FWorldGeneratorConfig& Config)
{
    // Ocean (below sea level)
    if (Height <= Config.SeaLevel)
    {
        return EBiomeType::Ocean;
    }

    // Beach (just above sea level)
    if (Height <= Config.SeaLevel + 0.05f)
    {
        return EBiomeType::Beach;
    }

    // Mountain (very high)
    if (Height >= 0.75f)
    {
        if (Temperature < 0.3f)
        {
            return EBiomeType::Snow;
        }
        return EBiomeType::Mountain;
    }

    // Check configured biomes
    for (const FBiomeSettings& BiomeSettings : Config.BiomeSettings)
    {
        if (Temperature >= BiomeSettings.MinTemperature &&
            Temperature <= BiomeSettings.MaxTemperature &&
            Moisture >= BiomeSettings.MinMoisture &&
            Moisture <= BiomeSettings.MaxMoisture &&
            Height >= BiomeSettings.MinHeight &&
            Height <= BiomeSettings.MaxHeight)
        {
            return BiomeSettings.BiomeType;
        }
    }

    // Fallback biome determination based on temperature and moisture
    if (Temperature < 0.2f)
    {
        return EBiomeType::Tundra;
    }
    else if (Temperature < 0.4f)
    {
        if (Moisture > 0.5f)
        {
            return EBiomeType::Taiga;
        }
        return EBiomeType::Grassland;
    }
    else if (Temperature < 0.6f)
    {
        if (Moisture > 0.6f)
        {
            return EBiomeType::Forest;
        }
        else if (Moisture < 0.3f)
        {
            return EBiomeType::Desert;
        }
        return EBiomeType::Grassland;
    }
    else // Hot
    {
        if (Moisture > 0.6f)
        {
            return EBiomeType::Rainforest;
        }
        else if (Moisture > 0.3f)
        {
            if (Moisture > 0.5f)
            {
                return EBiomeType::Swamp;
            }
            return EBiomeType::Savanna;
        }
        return EBiomeType::Desert;
    }
}

const FBiomeSettings* UHarmoniaWorldGeneratorSubsystem::FindBiomeSettings(
    EBiomeType BiomeType,
    const FWorldGeneratorConfig& Config) const
{
    for (const FBiomeSettings& Settings : Config.BiomeSettings)
    {
        if (Settings.BiomeType == BiomeType)
        {
            return &Settings;
        }
    }
    return nullptr;
}

void UHarmoniaWorldGeneratorSubsystem::GenerateRivers(
    const FWorldGeneratorConfig& Config,
    const TArray<int32>& HeightData,
    const TArray<FBiomeData>& BiomeData,
    TArray<FRoadSegmentData>& OutRiverSegments)
{
    if (!Config.RiverSettings.bEnableRivers || Config.RiverSettings.RiverCount <= 0)
    {
        OutRiverSegments.Empty();
        return;
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generating %d rivers..."), Config.RiverSettings.RiverCount);
    }

    OutRiverSegments.Empty();
    FRandomStream Random(Config.Seed + 3000);

    // Find potential river sources (high altitude locations)
    TArray<FIntPoint> PotentialSources;
    for (int32 Y = 0; Y < Config.SizeY; ++Y)
    {
        for (int32 X = 0; X < Config.SizeX; ++X)
        {
            const int32 Index = Y * Config.SizeX + X;
            const float Height = (float)HeightData[Index] / 65535.f;

            // Check if this is a good river source
            if (Height >= Config.RiverSettings.MinSourceHeight && Height < 0.9f)
            {
                PotentialSources.Add(FIntPoint(X, Y));
            }
        }
    }

    if (PotentialSources.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid river sources found!"));
        return;
    }

    // Generate rivers
    const int32 RiversToGenerate = FMath::Min(Config.RiverSettings.RiverCount, PotentialSources.Num());
    for (int32 i = 0; i < RiversToGenerate; ++i)
    {
        // Pick random source
        const int32 SourceIndex = Random.RandRange(0, PotentialSources.Num() - 1);
        const FIntPoint Source = PotentialSources[SourceIndex];

        // Trace river path
        TArray<FVector> RiverPath;
        TraceRiver(Source.X, Source.Y, HeightData, Config, RiverPath);

        // Check if river is long enough
        if (RiverPath.Num() >= Config.RiverSettings.MinRiverLength)
        {
            FRoadSegmentData RiverSegment;
            RiverSegment.SplinePoints = RiverPath;
            RiverSegment.SegmentMesh = Config.RiverSettings.RiverSplineMesh;
            RiverSegment.bIsBridge = false;
            RiverSegment.Width = Config.RiverSettings.RiverWidth;
            OutRiverSegments.Add(RiverSegment);
        }
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generated %d rivers"), OutRiverSegments.Num());
    }
}

void UHarmoniaWorldGeneratorSubsystem::TraceRiver(
    int32 StartX,
    int32 StartY,
    const TArray<int32>& HeightData,
    const FWorldGeneratorConfig& Config,
    TArray<FVector>& OutRiverPath)
{
    OutRiverPath.Empty();

    int32 CurrentX = StartX;
    int32 CurrentY = StartY;
    TSet<int32> VisitedTiles;

    const int32 MaxIterations = Config.RiverSettings.MaxRiverLength;

    for (int32 i = 0; i < MaxIterations; ++i)
    {
        const int32 CurrentIndex = CurrentY * Config.SizeX + CurrentX;

        // Check if we've been here before (prevent loops)
        if (VisitedTiles.Contains(CurrentIndex))
        {
            break;
        }
        VisitedTiles.Add(CurrentIndex);

        // Get current height
        const float CurrentHeight = (float)HeightData[CurrentIndex] / 65535.f;

        // Add point to river path
        const FVector RiverPoint(
            CurrentX * 100.f,
            CurrentY * 100.f,
            CurrentHeight * Config.MaxHeight
        );
        OutRiverPath.Add(RiverPoint);

        // Check if we reached sea level
        if (CurrentHeight <= Config.SeaLevel + 0.01f)
        {
            break;
        }

        // Find lowest neighbor
        int32 LowestX = CurrentX;
        int32 LowestY = CurrentY;
        float LowestHeight = CurrentHeight;

        // Check all 8 neighbors
        for (int32 dy = -1; dy <= 1; ++dy)
        {
            for (int32 dx = -1; dx <= 1; ++dx)
            {
                if (dx == 0 && dy == 0) continue;

                const int32 NeighborX = CurrentX + dx;
                const int32 NeighborY = CurrentY + dy;

                // Check bounds
                if (NeighborX < 0 || NeighborX >= Config.SizeX ||
                    NeighborY < 0 || NeighborY >= Config.SizeY)
                {
                    continue;
                }

                const int32 NeighborIndex = NeighborY * Config.SizeX + NeighborX;
                const float NeighborHeight = (float)HeightData[NeighborIndex] / 65535.f;

                if (NeighborHeight < LowestHeight)
                {
                    LowestHeight = NeighborHeight;
                    LowestX = NeighborX;
                    LowestY = NeighborY;
                }
            }
        }

        // If we found a lower neighbor, move there
        if (LowestX != CurrentX || LowestY != CurrentY)
        {
            CurrentX = LowestX;
            CurrentY = LowestY;
        }
        else
        {
            // No lower neighbor found (we're in a depression)
            break;
        }
    }
}

void UHarmoniaWorldGeneratorSubsystem::GenerateLakes(
    const FWorldGeneratorConfig& Config,
    TArray<int32>& HeightData,
    const TArray<FBiomeData>& BiomeData,
    TArray<FVector>& OutLakeLocations)
{
    if (!Config.LakeSettings.bEnableLakes || Config.LakeSettings.LakeCount <= 0)
    {
        OutLakeLocations.Empty();
        return;
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generating %d lakes..."), Config.LakeSettings.LakeCount);
    }

    OutLakeLocations.Empty();
    FRandomStream Random(Config.Seed + 4000);

    // Generate lakes
    for (int32 i = 0; i < Config.LakeSettings.LakeCount; ++i)
    {
        // Pick random location
        const int32 LakeX = Random.RandRange(
            Config.LakeSettings.MaxLakeRadius,
            Config.SizeX - Config.LakeSettings.MaxLakeRadius - 1
        );
        const int32 LakeY = Random.RandRange(
            Config.LakeSettings.MaxLakeRadius,
            Config.SizeY - Config.LakeSettings.MaxLakeRadius - 1
        );

        const int32 LakeIndex = LakeY * Config.SizeX + LakeX;
        const float LakeHeight = (float)HeightData[LakeIndex] / 65535.f;

        // Check if height is in preferred range
        if (LakeHeight < Config.LakeSettings.PreferredMinHeight ||
            LakeHeight > Config.LakeSettings.PreferredMaxHeight)
        {
            continue;
        }

        // Pick random radius
        const int32 Radius = Random.RandRange(
            Config.LakeSettings.MinLakeRadius,
            Config.LakeSettings.MaxLakeRadius
        );

        // Store lake location (X, Y, Radius)
        OutLakeLocations.Add(FVector(LakeX, LakeY, Radius));

        // Flatten terrain for lake
        for (int32 dy = -Radius; dy <= Radius; ++dy)
        {
            for (int32 dx = -Radius; dx <= Radius; ++dx)
            {
                const int32 X = LakeX + dx;
                const int32 Y = LakeY + dy;

                // Check bounds
                if (X < 0 || X >= Config.SizeX || Y < 0 || Y >= Config.SizeY)
                {
                    continue;
                }

                // Check if within circular radius
                const float Distance = FMath::Sqrt(static_cast<float>(dx * dx + dy * dy));
                if (Distance > Radius)
                {
                    continue;
                }

                // Set height to lake level (slightly below original height)
                const int32 Index = Y * Config.SizeX + X;
                const int32 LakeHeightValue = FMath::RoundToInt((LakeHeight - 0.02f) * 65535.f);
                HeightData[Index] = FMath::Max(LakeHeightValue, FMath::RoundToInt(Config.SeaLevel * 65535.f));
            }
        }
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generated %d lakes"), OutLakeLocations.Num());
    }
}

void UHarmoniaWorldGeneratorSubsystem::GenerateRoads(
    const FWorldGeneratorConfig& Config,
    const TArray<int32>& HeightData,
    TArray<FRoadSegmentData>& OutRoadSegments)
{
    if (!Config.RoadSettings.bEnableRoads ||
        Config.RoadSettings.ConnectionPoints.Num() < 2)
    {
        OutRoadSegments.Empty();
        return;
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generating roads between %d points..."),
            Config.RoadSettings.ConnectionPoints.Num());
    }

    OutRoadSegments.Empty();

    // Connect each point to the next
    for (int32 i = 0; i < Config.RoadSettings.ConnectionPoints.Num() - 1; ++i)
    {
        const FVector Start = Config.RoadSettings.ConnectionPoints[i];
        const FVector End = Config.RoadSettings.ConnectionPoints[i + 1];

        TArray<FVector> RoadPath;
        FindPath(Start, End, HeightData, Config, RoadPath);

        if (RoadPath.Num() >= 2)
        {
            FRoadSegmentData RoadSegment;
            RoadSegment.SplinePoints = RoadPath;
            RoadSegment.SegmentMesh = Config.RoadSettings.RoadSplineMesh;
            RoadSegment.bIsBridge = false; // TODO: Detect water crossings
            RoadSegment.Width = Config.RoadSettings.RoadWidth;
            OutRoadSegments.Add(RoadSegment);
        }
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generated %d road segments"), OutRoadSegments.Num());
    }
}

void UHarmoniaWorldGeneratorSubsystem::FindPath(
    FVector Start,
    FVector End,
    const TArray<int32>& HeightData,
    const FWorldGeneratorConfig& Config,
    TArray<FVector>& OutPath)
{
    OutPath.Empty();

    // Convert world positions to grid coordinates
    const int32 StartX = FMath::RoundToInt(Start.X / 100.f);
    const int32 StartY = FMath::RoundToInt(Start.Y / 100.f);
    const int32 EndX = FMath::RoundToInt(End.X / 100.f);
    const int32 EndY = FMath::RoundToInt(End.Y / 100.f);

    // Simple straight line path for now (can be improved with A* later)
    const int32 Steps = FMath::Max(FMath::Abs(EndX - StartX), FMath::Abs(EndY - StartY));

    if (Steps == 0)
    {
        OutPath.Add(Start);
        return;
    }

    for (int32 i = 0; i <= Steps; ++i)
    {
        const float T = (float)i / (float)Steps;
        const int32 X = FMath::RoundToInt(FMath::Lerp((float)StartX, (float)EndX, T));
        const int32 Y = FMath::RoundToInt(FMath::Lerp((float)StartY, (float)EndY, T));

        // Clamp to bounds
        const int32 ClampedX = FMath::Clamp(X, 0, Config.SizeX - 1);
        const int32 ClampedY = FMath::Clamp(Y, 0, Config.SizeY - 1);

        const int32 Index = ClampedY * Config.SizeX + ClampedX;
        const float Height = (float)HeightData[Index] / 65535.f;

        OutPath.Add(FVector(
            ClampedX * 100.f,
            ClampedY * 100.f,
            Height * Config.MaxHeight + 10.f // Slightly above terrain
        ));
    }
}

void UHarmoniaWorldGeneratorSubsystem::ApplyErosion(
    const FWorldGeneratorConfig& Config,
    TArray<int32>& HeightData)
{
    if (!Config.ErosionSettings.bEnableErosion || Config.ErosionSettings.ErosionIterations <= 0)
    {
        return;
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Applying erosion simulation: %d iterations..."),
            Config.ErosionSettings.ErosionIterations);
    }

    // Convert heightmap to float for better precision during erosion
    const int32 MapSize = Config.SizeX; // Assuming square map
    TArray<float> HeightMapFloat;
    HeightMapFloat.SetNumUninitialized(HeightData.Num());

    for (int32 i = 0; i < HeightData.Num(); ++i)
    {
        HeightMapFloat[i] = (float)HeightData[i] / 65535.f;
    }

    // Run erosion simulation
    FRandomStream Random(Config.Seed + 5000);

    for (int32 Iteration = 0; Iteration < Config.ErosionSettings.ErosionIterations; ++Iteration)
    {
        SimulateDroplet(HeightMapFloat, Config, Random);

        // Progress logging
        if (Config.bEnableProgressLogging && Iteration % FMath::Max(1, Config.ErosionSettings.ErosionIterations / 10) == 0)
        {
            const float Progress = (float)Iteration / (float)Config.ErosionSettings.ErosionIterations * 100.f;
            UE_LOG(LogTemp, Log, TEXT("Erosion progress: %.1f%% (%d/%d iterations)"),
                Progress, Iteration, Config.ErosionSettings.ErosionIterations);
        }
    }

    // Convert back to int32
    for (int32 i = 0; i < HeightData.Num(); ++i)
    {
        HeightData[i] = FMath::Clamp(
            FMath::RoundToInt(HeightMapFloat[i] * 65535.f),
            0,
            65535
        );
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Erosion simulation complete!"));
    }
}

void UHarmoniaWorldGeneratorSubsystem::SimulateDroplet(
    TArray<float>& HeightMap,
    const FWorldGeneratorConfig& Config,
    FRandomStream& Random)
{
    const int32 MapSize = Config.SizeX;
    const FErosionSettings& Settings = Config.ErosionSettings;

    // Random starting position
    float PosX = Random.FRandRange(0.f, (float)(MapSize - 1));
    float PosY = Random.FRandRange(0.f, (float)(MapSize - 1));

    float DirX = 0.f;
    float DirY = 0.f;
    float Speed = Settings.InitialSpeed;
    float Water = Settings.InitialWaterVolume;
    float Sediment = 0.f;

    for (int32 Lifetime = 0; Lifetime < Settings.MaxDropletLifetime; ++Lifetime)
    {
        const int32 NodeX = FMath::FloorToInt(PosX);
        const int32 NodeY = FMath::FloorToInt(PosY);
        const int32 DropletIndex = NodeY * MapSize + NodeX;

        // Calculate droplet's offset inside the cell (0,0) = at node, (1,1) = at next node
        const float CellOffsetX = PosX - NodeX;
        const float CellOffsetY = PosY - NodeY;

        // Calculate height and gradient
        FVector2D Gradient;
        const float Height = CalculateHeightAndGradient(HeightMap, PosX, PosY, MapSize, Gradient);

        // Update direction (lerp between previous direction and gradient)
        DirX = (DirX * Settings.Inertia - Gradient.X * (1 - Settings.Inertia));
        DirY = (DirY * Settings.Inertia - Gradient.Y * (1 - Settings.Inertia));

        // Normalize direction
        const float Len = FMath::Sqrt(DirX * DirX + DirY * DirY);
        if (Len != 0)
        {
            DirX /= Len;
            DirY /= Len;
        }

        // Update position
        const float NewPosX = PosX + DirX;
        const float NewPosY = PosY + DirY;

        // Stop if out of bounds
        if (NewPosX < 0 || NewPosX >= MapSize - 1 || NewPosY < 0 || NewPosY >= MapSize - 1)
        {
            break;
        }

        // Calculate new height
        FVector2D NewGradient;
        const float NewHeight = CalculateHeightAndGradient(HeightMap, NewPosX, NewPosY, MapSize, NewGradient);
        const float DeltaHeight = NewHeight - Height;

        // Calculate sediment capacity
        const float SedimentCapacity = FMath::Max(
            -DeltaHeight * Speed * Water * Settings.SedimentCapacityFactor,
            Settings.MinSedimentCapacity
        );

        // Erode or deposit sediment
        if (Sediment > SedimentCapacity || DeltaHeight > 0)
        {
            // Deposit sediment
            const float AmountToDeposit = (DeltaHeight > 0) ?
                FMath::Min(DeltaHeight, Sediment) :
                (Sediment - SedimentCapacity) * Settings.DepositSpeed;

            Sediment -= AmountToDeposit;

            // Deposit at current and surrounding nodes
            HeightMap[DropletIndex] += AmountToDeposit * (1 - CellOffsetX) * (1 - CellOffsetY);
            HeightMap[DropletIndex + 1] += AmountToDeposit * CellOffsetX * (1 - CellOffsetY);
            HeightMap[DropletIndex + MapSize] += AmountToDeposit * (1 - CellOffsetX) * CellOffsetY;
            HeightMap[DropletIndex + MapSize + 1] += AmountToDeposit * CellOffsetX * CellOffsetY;
        }
        else
        {
            // Erode terrain
            const float AmountToErode = FMath::Min(
                (SedimentCapacity - Sediment) * Settings.ErodeSpeed,
                -DeltaHeight
            );

            // Erode from surrounding nodes
            for (int32 BrushPointIndex = 0; BrushPointIndex < Settings.ErosionRadius * Settings.ErosionRadius; ++BrushPointIndex)
            {
                const int32 OffsetX = BrushPointIndex % Settings.ErosionRadius - Settings.ErosionRadius / 2;
                const int32 OffsetY = BrushPointIndex / Settings.ErosionRadius - Settings.ErosionRadius / 2;

                const int32 ErodeX = NodeX + OffsetX;
                const int32 ErodeY = NodeY + OffsetY;

                if (ErodeX >= 0 && ErodeX < MapSize && ErodeY >= 0 && ErodeY < MapSize)
                {
                    const float WeightX = FMath::Max(0.f, Settings.ErosionRadius - FMath::Abs((float)OffsetX));
                    const float WeightY = FMath::Max(0.f, Settings.ErosionRadius - FMath::Abs((float)OffsetY));
                    const float Weight = WeightX * WeightY;

                    HeightMap[ErodeY * MapSize + ErodeX] -= AmountToErode * Weight;
                }
            }

            Sediment += AmountToErode;
        }

        // Update speed and water
        Speed = FMath::Sqrt(Speed * Speed + DeltaHeight * Settings.Gravity);
        Water *= (1 - Settings.EvaporateSpeed);

        // Update position
        PosX = NewPosX;
        PosY = NewPosY;
    }
}

float UHarmoniaWorldGeneratorSubsystem::CalculateHeightAndGradient(
    const TArray<float>& HeightMap,
    float PosX,
    float PosY,
    int32 MapSize,
    FVector2D& OutGradient)
{
    const int32 CoordX = FMath::FloorToInt(PosX);
    const int32 CoordY = FMath::FloorToInt(PosY);

    // Calculate droplet's offset inside the cell (0,0) = at node, (1,1) = at next node
    const float X = PosX - CoordX;
    const float Y = PosY - CoordY;

    // Calculate heights of the four nodes of the droplet's cell
    const int32 NodeIndexNW = CoordY * MapSize + CoordX;
    const float HeightNW = HeightMap[NodeIndexNW];
    const float HeightNE = HeightMap[NodeIndexNW + 1];
    const float HeightSW = HeightMap[NodeIndexNW + MapSize];
    const float HeightSE = HeightMap[NodeIndexNW + MapSize + 1];

    // Calculate gradients in X and Y direction
    OutGradient.X = (HeightNE - HeightNW) * (1 - Y) + (HeightSE - HeightSW) * Y;
    OutGradient.Y = (HeightSW - HeightNW) * (1 - X) + (HeightSE - HeightNE) * X;

    // Calculate bilinearly interpolated height
    const float Height = HeightNW * (1 - X) * (1 - Y) +
                        HeightNE * X * (1 - Y) +
                        HeightSW * (1 - X) * Y +
                        HeightSE * X * Y;

    return Height;
}

void UHarmoniaWorldGeneratorSubsystem::GenerateStructureGroups(
    const FWorldGeneratorConfig& Config,
    const TArray<int32>& HeightData,
    TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap,
    TArray<FWorldObjectData>& OutObjects)
{
    if (!Config.StructureGroupSettings.bEnableStructureGroups ||
        Config.StructureGroupSettings.GroupCount <= 0)
    {
        OutObjects.Empty();
        return;
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generating %d structure groups..."),
            Config.StructureGroupSettings.GroupCount);
    }

    OutObjects.Empty();
    FRandomStream Random(Config.Seed + 6000);

    // Find suitable locations for group centers (flat areas)
    TArray<FIntPoint> FlatLocations;
    const int32 SearchRadius = 5;

    for (int32 Y = SearchRadius; Y < Config.SizeY - SearchRadius; Y += 10) // Sample every 10 tiles
    {
        for (int32 X = SearchRadius; X < Config.SizeX - SearchRadius; X += 10)
        {
            if (IsFlatEnoughForStructure(X, Y, HeightData, Config))
            {
                const float Height = (float)HeightData[Y * Config.SizeX + X] / 65535.f;

                // Must be above sea level
                if (Height > Config.SeaLevel + 0.05f)
                {
                    FlatLocations.Add(FIntPoint(X, Y));
                }
            }
        }
    }

    if (FlatLocations.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No suitable flat locations found for structure groups!"));
        return;
    }

    const int32 GroupsToGenerate = FMath::Min(Config.StructureGroupSettings.GroupCount, FlatLocations.Num());

    // Generate structure groups
    for (int32 GroupIdx = 0; GroupIdx < GroupsToGenerate; ++GroupIdx)
    {
        // Pick random center location
        const int32 LocationIdx = Random.RandRange(0, FlatLocations.Num() - 1);
        const FIntPoint Center = FlatLocations[LocationIdx];
        FlatLocations.RemoveAt(LocationIdx); // Don't reuse this location

        const int32 StructureCount = Random.RandRange(
            Config.StructureGroupSettings.MinStructuresPerGroup,
            Config.StructureGroupSettings.MaxStructuresPerGroup
        );

        // Generate center structure
        {
            const int32 CenterIndex = Center.Y * Config.SizeX + Center.X;
            const float CenterHeight = (float)HeightData[CenterIndex] / 65535.f;

            FWorldObjectData CenterObj;
            CenterObj.ObjectType = EWorldObjectType::Structure;
            CenterObj.ActorClass = ActorClassMap.Contains(EWorldObjectType::Structure) ?
                ActorClassMap[EWorldObjectType::Structure] : nullptr;
            CenterObj.Location = FVector(
                Center.X * 100.f,
                Center.Y * 100.f,
                CenterHeight * Config.MaxHeight
            );
            CenterObj.Rotation = FRotator(0.f, Random.FRandRange(0.f, 360.f), 0.f);
            CenterObj.Scale = FVector::OneVector;
            CenterObj.GroupID = GroupIdx;
            CenterObj.bIsGroupCenter = true;

            OutObjects.Add(CenterObj);
        }

        // Generate surrounding structures
        const float GroupRadiusTiles = Config.StructureGroupSettings.GroupRadius / 100.f;
        const float SpacingTiles = Config.StructureGroupSettings.StructureSpacing / 100.f;

        for (int32 StructIdx = 1; StructIdx < StructureCount; ++StructIdx)
        {
            // Try to find a valid location near center
            bool bFoundLocation = false;
            int32 Attempts = 0;
            const int32 MaxAttempts = 20;

            while (!bFoundLocation && Attempts < MaxAttempts)
            {
                Attempts++;

                // Random angle and distance
                const float Angle = Random.FRandRange(0.f, 2.f * PI);
                const float Distance = Random.FRandRange(SpacingTiles, GroupRadiusTiles);

                const int32 StructX = Center.X + FMath::RoundToInt(FMath::Cos(Angle) * Distance);
                const int32 StructY = Center.Y + FMath::RoundToInt(FMath::Sin(Angle) * Distance);

                // Check if valid location
                if (StructX < 0 || StructX >= Config.SizeX ||
                    StructY < 0 || StructY >= Config.SizeY)
                {
                    continue;
                }

                if (!IsValidObjectLocation(StructX, StructY, HeightData, Config))
                {
                    continue;
                }

                // Check flatness
                const float Flatness = CalculateFlatness(StructX, StructY, 2, HeightData, Config);
                if (Flatness < Config.StructureGroupSettings.MinFlatness * 0.8f) // Slightly more lenient for non-center
                {
                    continue;
                }

                // Check distance from other structures in group
                bool bTooClose = false;
                for (const FWorldObjectData& ExistingObj : OutObjects)
                {
                    if (ExistingObj.GroupID == GroupIdx)
                    {
                        const float DistSq = FVector2D::DistSquared(
                            FVector2D(StructX, StructY),
                            FVector2D(ExistingObj.Location.X / 100.f, ExistingObj.Location.Y / 100.f)
                        );

                        if (DistSq < SpacingTiles * SpacingTiles)
                        {
                            bTooClose = true;
                            break;
                        }
                    }
                }

                if (bTooClose)
                {
                    continue;
                }

                // Found valid location!
                const int32 StructIndex = StructY * Config.SizeX + StructX;
                const float StructHeight = (float)HeightData[StructIndex] / 65535.f;

                FWorldObjectData StructObj;
                StructObj.ObjectType = EWorldObjectType::Structure;
                StructObj.ActorClass = ActorClassMap.Contains(EWorldObjectType::Structure) ?
                    ActorClassMap[EWorldObjectType::Structure] : nullptr;
                StructObj.Location = FVector(
                    StructX * 100.f,
                    StructY * 100.f,
                    StructHeight * Config.MaxHeight
                );
                StructObj.Rotation = FRotator(0.f, Random.FRandRange(0.f, 360.f), 0.f);
                StructObj.Scale = FVector(Random.FRandRange(0.9f, 1.1f));
                StructObj.GroupID = GroupIdx;
                StructObj.bIsGroupCenter = false;

                OutObjects.Add(StructObj);
                bFoundLocation = true;
            }
        }
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generated %d structure groups with %d total structures"),
            GroupsToGenerate, OutObjects.Num());
    }
}

void UHarmoniaWorldGeneratorSubsystem::GenerateWorldAsync(
    const FWorldGeneratorConfig& Config,
    TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap)
{
    if (bIsGenerating)
    {
        UE_LOG(LogTemp, Warning, TEXT("Async world generation already in progress!"));
        return;
    }

    bIsGenerating = true;
    bCancelRequested = false;
    CurrentProgress = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("Starting async world generation..."));

    // Launch async task
    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, Config, ActorClassMap]()
    {
        AsyncGenerationWorker(Config, ActorClassMap);
    });
}

void UHarmoniaWorldGeneratorSubsystem::CancelAsyncGeneration()
{
    if (bIsGenerating)
    {
        UE_LOG(LogTemp, Log, TEXT("Cancelling async world generation..."));
        bCancelRequested = true;
    }
}

void UHarmoniaWorldGeneratorSubsystem::AsyncGenerationWorker(
    FWorldGeneratorConfig Config,
    TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap)
{
    TArray<int32> HeightData;
    TArray<FWorldObjectData> Objects;
    bool bSuccess = false;

    try
    {
        // Phase 1: Generate Heightmap (40% of progress)
        UpdateProgress(0.0f);

        if (bCancelRequested) { CompleteAsyncGeneration(HeightData, Objects, false); return; }

        const int32 TotalSize = Config.SizeX * Config.SizeY;
        HeightData.SetNumUninitialized(TotalSize);

        const int32 ChunkSize = FMath::Max(1, Config.ChunkSize);
        const int32 ChunksX = FMath::DivideAndRoundUp(Config.SizeX, ChunkSize);
        const int32 ChunksY = FMath::DivideAndRoundUp(Config.SizeY, ChunkSize);
        const int32 TotalChunks = ChunksX * ChunksY;

        int32 ProcessedChunks = 0;

        // Generate heightmap
        for (int32 ChunkY = 0; ChunkY < ChunksY && !bCancelRequested; ++ChunkY)
        {
            for (int32 ChunkX = 0; ChunkX < ChunksX && !bCancelRequested; ++ChunkX)
            {
                const int32 StartX = ChunkX * ChunkSize;
                const int32 StartY = ChunkY * ChunkSize;
                const int32 EndX = FMath::Min(StartX + ChunkSize, Config.SizeX);
                const int32 EndY = FMath::Min(StartY + ChunkSize, Config.SizeY);

                for (int32 Y = StartY; Y < EndY; ++Y)
                {
                    for (int32 X = StartX; X < EndX; ++X)
                    {
                        const float Height = CalculateHeightAtLocation(X, Y, Config);
                        const int32 HeightVal = FMath::Clamp(
                            FMath::RoundToInt(Height * 65535.f),
                            0,
                            65535
                        );
                        HeightData[Y * Config.SizeX + X] = HeightVal;
                    }
                }

                ProcessedChunks++;
                const float Progress = (float)ProcessedChunks / (float)TotalChunks * 0.4f;
                UpdateProgress(Progress);
            }
        }

        if (bCancelRequested) { CompleteAsyncGeneration(HeightData, Objects, false); return; }

        // Phase 2: Apply Erosion (20% of progress)
        if (Config.ErosionSettings.bEnableErosion && Config.ErosionSettings.ErosionIterations > 0)
        {
            UpdateProgress(0.4f);

            TArray<float> HeightMapFloat;
            HeightMapFloat.SetNumUninitialized(HeightData.Num());
            for (int32 i = 0; i < HeightData.Num(); ++i)
            {
                HeightMapFloat[i] = (float)HeightData[i] / 65535.f;
            }

            FRandomStream Random(Config.Seed + 5000);
            const int32 TotalIterations = Config.ErosionSettings.ErosionIterations;

            for (int32 Iteration = 0; Iteration < TotalIterations && !bCancelRequested; ++Iteration)
            {
                SimulateDroplet(HeightMapFloat, Config, Random);

                if (Iteration % FMath::Max(1, TotalIterations / 20) == 0)
                {
                    const float ErosionProgress = (float)Iteration / (float)TotalIterations;
                    UpdateProgress(0.4f + ErosionProgress * 0.2f);
                }
            }

            for (int32 i = 0; i < HeightData.Num(); ++i)
            {
                HeightData[i] = FMath::Clamp(
                    FMath::RoundToInt(HeightMapFloat[i] * 65535.f),
                    0,
                    65535
                );
            }
        }

        if (bCancelRequested) { CompleteAsyncGeneration(HeightData, Objects, false); return; }

        // Phase 3: Generate Objects (40% of progress)
        UpdateProgress(0.6f);

        const int32 TotalTiles = Config.SizeX * Config.SizeY;
        const int32 ApproxObjectCount = FMath::CeilToInt(TotalTiles * Config.ObjectDensity);
        Objects.Reserve(ApproxObjectCount);

        ProcessedChunks = 0;

        for (int32 ChunkY = 0; ChunkY < ChunksY && !bCancelRequested; ++ChunkY)
        {
            for (int32 ChunkX = 0; ChunkX < ChunksX && !bCancelRequested; ++ChunkX)
            {
                const int32 StartX = ChunkX * ChunkSize;
                const int32 StartY = ChunkY * ChunkSize;
                const int32 EndX = FMath::Min(StartX + ChunkSize, Config.SizeX);
                const int32 EndY = FMath::Min(StartY + ChunkSize, Config.SizeY);

                for (int32 Y = StartY; Y < EndY; ++Y)
                {
                    for (int32 X = StartX; X < EndX; ++X)
                    {
                        int32 LocationSeed = Config.Seed + X * 73856093 + Y * 19349663;
                        FRandomStream Random(LocationSeed);

                        if (!IsValidObjectLocation(X, Y, HeightData, Config))
                        {
                            continue;
                        }

                        if (Random.FRand() >= Config.ObjectDensity)
                        {
                            continue;
                        }

                        EWorldObjectType ObjType = PickObjectType(Config.ObjectTypeProbabilities, Random);
                        if (ObjType == EWorldObjectType::None)
                        {
                            continue;
                        }

                        float HeightNorm = (float)HeightData[Y * Config.SizeX + X] / 65535.f;
                        FVector Location(X * 100.f, Y * 100.f, HeightNorm * Config.MaxHeight);

                        FWorldObjectData ObjData;
                        ObjData.ObjectType = ObjType;
                        ObjData.ActorClass = ActorClassMap.Contains(ObjType) ? ActorClassMap[ObjType] : nullptr;
                        ObjData.Location = Location;
                        ObjData.Rotation = FRotator(0.f, Random.FRandRange(0.f, 360.f), 0.f);
                        ObjData.Scale = FVector(Random.FRandRange(0.8f, 1.2f));

                        Objects.Add(ObjData);
                    }
                }

                ProcessedChunks++;
                const float Progress = 0.6f + ((float)ProcessedChunks / (float)TotalChunks * 0.4f);
                UpdateProgress(Progress);
            }
        }

        if (bCancelRequested)
        {
            CompleteAsyncGeneration(HeightData, Objects, false);
            return;
        }

        UpdateProgress(1.0f);
        bSuccess = true;

        UE_LOG(LogTemp, Log, TEXT("Async world generation completed successfully! Generated %d objects"), Objects.Num());
    }
    catch (const std::exception& e)
    {
        UE_LOG(LogTemp, Error, TEXT("Async world generation failed: %s"), ANSI_TO_TCHAR(e.what()));
        bSuccess = false;
    }

    CompleteAsyncGeneration(MoveTemp(HeightData), MoveTemp(Objects), bSuccess);
}

void UHarmoniaWorldGeneratorSubsystem::UpdateProgress(float Progress)
{
    CurrentProgress = Progress;

    // Broadcast progress on game thread
    AsyncTask(ENamedThreads::GameThread, [this, Progress]()
    {
        OnGenerationProgress.Broadcast(Progress);
    });
}

void UHarmoniaWorldGeneratorSubsystem::CompleteAsyncGeneration(
    TArray<int32> HeightData,
    TArray<FWorldObjectData> Objects,
    bool bSuccess)
{
    // Execute completion on game thread
    AsyncTask(ENamedThreads::GameThread, [this, HeightData = MoveTemp(HeightData), Objects = MoveTemp(Objects), bSuccess]()
    {
        bIsGenerating = false;
        bCancelRequested = false;
        CurrentProgress = 0.0f;

        OnGenerationComplete.Broadcast(HeightData, Objects, bSuccess);

        if (bSuccess)
        {
            UE_LOG(LogTemp, Log, TEXT("Async world generation completed and broadcasted to delegates"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Async world generation was cancelled or failed"));
        }
    });
}

// ===== Cave System Implementation =====

void UHarmoniaWorldGeneratorSubsystem::GenerateCaveSystem(
    const FWorldGeneratorConfig& Config,
    const TArray<int32>& HeightData,
    TArray<FCaveVolumeData>& OutCaveVolume,
    TArray<FWorldObjectData>& OutCaveEntrances)
{
    if (!Config.CaveSettings.bEnableCaves)
    {
        OutCaveVolume.Empty();
        OutCaveEntrances.Empty();
        return;
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generating cave system with 3D noise..."));
    }

    OutCaveVolume.Empty();
    OutCaveEntrances.Empty();

    // Generate 3D cave volume
    const int32 DepthLevels = FMath::CeilToInt((Config.CaveSettings.MaxCaveDepth - Config.CaveSettings.MinCaveDepth) / Config.CaveSettings.CaveScale);

    for (int32 Y = 0; Y < Config.SizeY; ++Y)
    {
        for (int32 X = 0; X < Config.SizeX; ++X)
        {
            const float SurfaceHeight = (float)HeightData[Y * Config.SizeX + X] / 65535.f * Config.MaxHeight;

            // Generate cave at multiple depth levels
            for (int32 D = 0; D < DepthLevels; ++D)
            {
                const float Depth = Config.CaveSettings.MinCaveDepth + (D * Config.CaveSettings.CaveScale);
                const float Z = SurfaceHeight - Depth;

                // Calculate 3D cave noise
                const float CaveNoise = Calculate3DCaveNoise(X, Y, Z, Config);

                // Check if this is a cave location
                if (CaveNoise > Config.CaveSettings.CaveThreshold)
                {
                    FCaveVolumeData CaveData;
                    CaveData.GridPosition = FIntVector(X, Y, D);
                    CaveData.bIsCave = true;
                    CaveData.Density = (CaveNoise - Config.CaveSettings.CaveThreshold) / (1.0f - Config.CaveSettings.CaveThreshold);
                    OutCaveVolume.Add(CaveData);
                }
            }
        }
    }

    // Generate cave entrances
    FRandomStream Random(Config.Seed + 7000);

    for (int32 i = 0; i < Config.CaveSettings.CaveEntranceCount; ++i)
    {
        // Find random location
        const int32 EntranceX = Random.RandRange(10, Config.SizeX - 10);
        const int32 EntranceY = Random.RandRange(10, Config.SizeY - 10);
        const int32 Index = EntranceY * Config.SizeX + EntranceX;
        const float Height = (float)HeightData[Index] / 65535.f;

        // Check height range
        if (Height < Config.CaveSettings.MinEntranceHeight || Height > Config.CaveSettings.MaxEntranceHeight)
        {
            continue;
        }

        FWorldObjectData Entrance;
        Entrance.ObjectType = EWorldObjectType::CaveEntrance;
        Entrance.ActorClass = nullptr; // Set by user
        Entrance.Location = FVector(
            EntranceX * 100.f,
            EntranceY * 100.f,
            Height * Config.MaxHeight
        );
        Entrance.Rotation = FRotator(0.f, Random.FRandRange(0.f, 360.f), 0.f);
        Entrance.Scale = FVector::OneVector;
        Entrance.CaveDepth = Config.CaveSettings.MaxCaveDepth;

        OutCaveEntrances.Add(Entrance);
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generated %d cave volumes and %d entrances"),
            OutCaveVolume.Num(), OutCaveEntrances.Num());
    }
}

float UHarmoniaWorldGeneratorSubsystem::Calculate3DCaveNoise(
    float X,
    float Y,
    float Z,
    const FWorldGeneratorConfig& Config)
{
    const FPerlinNoiseSettings& NoiseSettings = Config.CaveSettings.CaveNoiseSettings;

    // 3D Perlin noise using multiple octaves
    float Noise = 0.0f;
    float Amplitude = NoiseSettings.Amplitude;
    float Frequency = NoiseSettings.Frequency * 0.01f;
    float MaxValue = 0.0f;

    for (int32 Octave = 0; Octave < NoiseSettings.Octaves; ++Octave)
    {
        // Simple 3D noise using combination of 2D noises
        const float Noise1 = PerlinNoiseHelper::GetSimpleNoise(
            X * Frequency,
            Y * Frequency,
            Config.Seed + Octave * 1000
        );

        const float Noise2 = PerlinNoiseHelper::GetSimpleNoise(
            Y * Frequency,
            Z * Frequency * 0.1f, // Scale Z differently
            Config.Seed + Octave * 1000 + 500
        );

        const float Noise3 = PerlinNoiseHelper::GetSimpleNoise(
            X * Frequency,
            Z * Frequency * 0.1f,
            Config.Seed + Octave * 1000 + 250
        );

        // Combine noises for 3D effect
        Noise += (Noise1 + Noise2 + Noise3) / 3.0f * Amplitude;
        MaxValue += Amplitude;

        Amplitude *= NoiseSettings.Persistence;
        Frequency *= NoiseSettings.Lacunarity;
    }

    // Normalize to 0-1
    return (Noise / MaxValue + 1.0f) * 0.5f;
}

// ===== POI System Implementation =====

void UHarmoniaWorldGeneratorSubsystem::GeneratePOIs(
    const FWorldGeneratorConfig& Config,
    const TArray<int32>& HeightData,
    const TArray<FBiomeData>& BiomeData,
    TArray<FWorldObjectData>& OutPOIs)
{
    if (!Config.POISettings.bEnablePOI || Config.POISettings.POICount <= 0)
    {
        OutPOIs.Empty();
        return;
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generating %d POIs..."), Config.POISettings.POICount);
    }

    OutPOIs.Empty();
    FRandomStream Random(Config.Seed + 8000);

    // Calculate world center for distance-based difficulty
    const FVector WorldCenter(
        Config.SizeX * 50.f,
        Config.SizeY * 50.f,
        0.f
    );

    // Generate POIs
    int32 AttemptsRemaining = Config.POISettings.POICount * 10; // Max attempts

    while (OutPOIs.Num() < Config.POISettings.POICount && AttemptsRemaining > 0)
    {
        AttemptsRemaining--;

        // Random location
        const int32 X = Random.RandRange(10, Config.SizeX - 10);
        const int32 Y = Random.RandRange(10, Config.SizeY - 10);
        const int32 Index = Y * Config.SizeX + X;
        const float Height = (float)HeightData[Index] / 65535.f;

        // Check if valid location
        if (Height <= Config.SeaLevel + 0.05f)
        {
            continue;
        }

        const FVector POILocation(X * 100.f, Y * 100.f, Height * Config.MaxHeight);

        // Check minimum distance from existing POIs
        bool bTooClose = false;
        for (const FWorldObjectData& ExistingPOI : OutPOIs)
        {
            const float DistSq = FVector::DistSquared(POILocation, ExistingPOI.Location);
            if (DistSq < Config.POISettings.MinPOIDistance * Config.POISettings.MinPOIDistance)
            {
                bTooClose = true;
                break;
            }
        }

        if (bTooClose)
        {
            continue;
        }

        // Pick POI type
        EPOIType POIType = PickPOIType(Config.POISettings.POITypeProbabilities, Random);
        if (POIType == EPOIType::None)
        {
            continue;
        }

        // Calculate difficulty based on distance from center
        int32 Difficulty = 1;
        if (Config.POISettings.bDifficultyByDistance)
        {
            const float DistanceFromCenter = FVector::Dist(POILocation, WorldCenter);
            const float MaxDistance = FMath::Sqrt(
                static_cast<float>(Config.SizeX * Config.SizeX + Config.SizeY * Config.SizeY)
            ) * 50.f;
            Difficulty = FMath::CeilToInt((DistanceFromCenter / MaxDistance) * 10.0f);
            Difficulty = FMath::Clamp(Difficulty, 1, 10);
        }
        else
        {
            Difficulty = Random.RandRange(1, 10);
        }

        FWorldObjectData POI;
        POI.ObjectType = EWorldObjectType::POI;
        POI.ActorClass = Config.POISettings.POIActorClasses.Contains(POIType) ?
            Config.POISettings.POIActorClasses[POIType] : nullptr;
        POI.Location = POILocation;
        POI.Rotation = FRotator(0.f, Random.FRandRange(0.f, 360.f), 0.f);
        POI.Scale = FVector::OneVector;
        POI.POIType = POIType;
        POI.Difficulty = Difficulty;

        OutPOIs.Add(POI);
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generated %d POIs"), OutPOIs.Num());
    }
}

EPOIType UHarmoniaWorldGeneratorSubsystem::PickPOIType(
    const TMap<EPOIType, float>& ProbMap,
    FRandomStream& Random)
{
    if (ProbMap.Num() == 0)
    {
        return EPOIType::None;
    }

    float TotalWeight = 0.0f;
    for (const auto& Pair : ProbMap)
    {
        TotalWeight += Pair.Value;
    }

    if (TotalWeight <= 0.0f)
    {
        return EPOIType::None;
    }

    float RandomValue = Random.FRandRange(0.0f, TotalWeight);
    float CurrentWeight = 0.0f;

    for (const auto& Pair : ProbMap)
    {
        CurrentWeight += Pair.Value;
        if (RandomValue <= CurrentWeight)
        {
            return Pair.Key;
        }
    }

    return EPOIType::None;
}

// ===== Resource Distribution Implementation =====

void UHarmoniaWorldGeneratorSubsystem::GenerateResourceDistribution(
    const FWorldGeneratorConfig& Config,
    const TArray<int32>& HeightData,
    const TArray<FBiomeData>& BiomeData,
    TArray<FOreVeinData>& OutOreVeins,
    TArray<FWorldObjectData>& OutResourceNodes)
{
    if (!Config.ResourceSettings.bEnableResources || Config.ResourceSettings.OreVeinCount <= 0)
    {
        OutOreVeins.Empty();
        OutResourceNodes.Empty();
        return;
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generating %d ore veins..."), Config.ResourceSettings.OreVeinCount);
    }

    OutOreVeins.Empty();
    OutResourceNodes.Empty();
    FRandomStream Random(Config.Seed + 9000);

    // Generate ore veins
    for (int32 i = 0; i < Config.ResourceSettings.OreVeinCount; ++i)
    {
        // Random location
        const int32 VeinX = Random.RandRange(0, Config.SizeX - 1);
        const int32 VeinY = Random.RandRange(0, Config.SizeY - 1);
        const int32 Index = VeinY * Config.SizeX + VeinX;
        const float Height = (float)HeightData[Index] / 65535.f;

        // Get biome at location
        EBiomeType BiomeType = EBiomeType::None;
        if (BiomeData.Num() > Index)
        {
            BiomeType = BiomeData[Index].BiomeType;
        }

        // Pick resource type based on biome and height
        EResourceType ResourceType = PickResourceType(
            Config.ResourceSettings.ResourceTypeProbabilities,
            BiomeType,
            Height,
            Config,
            Random
        );

        if (ResourceType == EResourceType::None)
        {
            continue;
        }

        // Create vein
        FOreVeinData Vein;
        Vein.Location = FVector(VeinX * 100.f, VeinY * 100.f, Height * Config.MaxHeight);
        Vein.ResourceType = ResourceType;
        Vein.Radius = Random.FRandRange(
            Config.ResourceSettings.MinVeinRadius,
            Config.ResourceSettings.MaxVeinRadius
        );
        Vein.Richness = Random.FRandRange(0.5f, 1.0f);
        Vein.NodeCount = Random.RandRange(
            Config.ResourceSettings.MinNodesPerVein,
            Config.ResourceSettings.MaxNodesPerVein
        );

        OutOreVeins.Add(Vein);

        // Generate resource nodes within vein
        for (int32 NodeIdx = 0; NodeIdx < Vein.NodeCount; ++NodeIdx)
        {
            // Random position within vein radius
            const float Angle = Random.FRandRange(0.f, 2.f * PI);
            const float Distance = Random.FRandRange(0.f, Vein.Radius);
            const int32 NodeX = VeinX + FMath::RoundToInt(FMath::Cos(Angle) * Distance / 100.f);
            const int32 NodeY = VeinY + FMath::RoundToInt(FMath::Sin(Angle) * Distance / 100.f);

            // Check bounds
            if (NodeX < 0 || NodeX >= Config.SizeX || NodeY < 0 || NodeY >= Config.SizeY)
            {
                continue;
            }

            const int32 NodeIndex = NodeY * Config.SizeX + NodeX;
            const float NodeHeight = (float)HeightData[NodeIndex] / 65535.f;

            FWorldObjectData ResourceNode;
            ResourceNode.ObjectType = EWorldObjectType::OreVein;
            ResourceNode.ActorClass = Config.ResourceSettings.ResourceActorClasses.Contains(ResourceType) ?
                Config.ResourceSettings.ResourceActorClasses[ResourceType] : nullptr;
            ResourceNode.Location = FVector(
                NodeX * 100.f,
                NodeY * 100.f,
                NodeHeight * Config.MaxHeight
            );
            ResourceNode.Rotation = FRotator(0.f, Random.FRandRange(0.f, 360.f), 0.f);
            ResourceNode.Scale = FVector(Random.FRandRange(0.8f, 1.2f));
            ResourceNode.ResourceType = ResourceType;
            ResourceNode.ResourceAmount = Vein.Richness * Random.FRandRange(0.8f, 1.2f);

            OutResourceNodes.Add(ResourceNode);
        }
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generated %d ore veins with %d total resource nodes"),
            OutOreVeins.Num(), OutResourceNodes.Num());
    }
}

EResourceType UHarmoniaWorldGeneratorSubsystem::PickResourceType(
    const TMap<EResourceType, float>& ProbMap,
    EBiomeType BiomeType,
    float Height,
    const FWorldGeneratorConfig& Config,
    FRandomStream& Random)
{
    if (ProbMap.Num() == 0)
    {
        return EResourceType::None;
    }

    // Build weighted probability map considering biome and height
    TMap<EResourceType, float> AdjustedProbabilities;

    for (const auto& Pair : ProbMap)
    {
        float Probability = Pair.Value;

        // Apply biome multiplier
        if (Config.ResourceSettings.BiomeResourceMultipliers.Contains(BiomeType))
        {
            const auto& BiomeMultipliers = Config.ResourceSettings.BiomeResourceMultipliers[BiomeType];
            if (BiomeMultipliers.Contains(Pair.Key))
            {
                Probability *= BiomeMultipliers[Pair.Key];
            }
        }

        // Apply height range filtering
        if (Config.ResourceSettings.ResourceHeightRanges.Contains(Pair.Key))
        {
            const FVector2D& HeightRange = Config.ResourceSettings.ResourceHeightRanges[Pair.Key];
            if (Height < HeightRange.X || Height > HeightRange.Y)
            {
                Probability = 0.0f; // Outside valid height range
            }
        }

        if (Probability > 0.0f)
        {
            AdjustedProbabilities.Add(Pair.Key, Probability);
        }
    }

    // Pick from adjusted probabilities
    float TotalWeight = 0.0f;
    for (const auto& Pair : AdjustedProbabilities)
    {
        TotalWeight += Pair.Value;
    }

    if (TotalWeight <= 0.0f)
    {
        return EResourceType::None;
    }

    float RandomValue = Random.FRandRange(0.0f, TotalWeight);
    float CurrentWeight = 0.0f;

    for (const auto& Pair : AdjustedProbabilities)
    {
        CurrentWeight += Pair.Value;
        if (RandomValue <= CurrentWeight)
        {
            return Pair.Key;
        }
    }

    return EResourceType::None;
}

// ===== Splatmap Generation Implementation =====

void UHarmoniaWorldGeneratorSubsystem::GenerateSplatmap(
    const FWorldGeneratorConfig& Config,
    const TArray<int32>& HeightData,
    const TArray<FBiomeData>& BiomeData,
    TArray<FSplatmapLayerData>& OutSplatmapLayers)
{
    if (!Config.SplatmapSettings.bEnableSplatmap)
    {
        OutSplatmapLayers.Empty();
        return;
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generating splatmap layers..."));
    }

    OutSplatmapLayers.Empty();

    // Collect all unique layer names
    TSet<FName> AllLayers;

    // Add biome layers
    for (const auto& Pair : Config.SplatmapSettings.BiomeToLayerMap)
    {
        AllLayers.Add(Pair.Value);
    }

    // Add height layers
    for (const FName& LayerName : Config.SplatmapSettings.HeightLayers)
    {
        AllLayers.Add(LayerName);
    }

    // Add slope layer
    if (Config.SplatmapSettings.SlopeLayerName != NAME_None)
    {
        AllLayers.Add(Config.SplatmapSettings.SlopeLayerName);
    }

    // Generate weight data for each layer
    int32 LayerIndex = 0;
    for (const FName& LayerName : AllLayers)
    {
        FSplatmapLayerData LayerData;
        LayerData.LayerIndex = LayerIndex++;
        LayerData.LayerName = LayerName;
        LayerData.WeightData.SetNumZeroed(Config.SizeX * Config.SizeY);

        // Calculate weights for each tile
        for (int32 Y = 0; Y < Config.SizeY; ++Y)
        {
            for (int32 X = 0; X < Config.SizeX; ++X)
            {
                const int32 Index = Y * Config.SizeX + X;
                const uint8 Weight = CalculateLayerWeight(
                    X, Y, LayerName, HeightData, BiomeData, Config
                );
                LayerData.WeightData[Index] = Weight;
            }
        }

        OutSplatmapLayers.Add(LayerData);
    }

    if (Config.bEnableProgressLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Generated %d splatmap layers"), OutSplatmapLayers.Num());
    }
}

uint8 UHarmoniaWorldGeneratorSubsystem::CalculateLayerWeight(
    int32 X,
    int32 Y,
    FName LayerName,
    const TArray<int32>& HeightData,
    const TArray<FBiomeData>& BiomeData,
    const FWorldGeneratorConfig& Config)
{
    const int32 Index = Y * Config.SizeX + X;
    const float Height = (float)HeightData[Index] / 65535.f;

    uint8 Weight = 0;

    // Check slope-based layer
    if (LayerName == Config.SplatmapSettings.SlopeLayerName)
    {
        const float Slope = CalculateSlope(X, Y, HeightData, Config);
        if (Slope >= Config.SplatmapSettings.SlopeThreshold)
        {
            // Full weight for steep slopes
            Weight = 255;
        }
        else if (Slope >= Config.SplatmapSettings.SlopeThreshold - 10.0f)
        {
            // Blend for near-threshold slopes
            const float T = (Slope - (Config.SplatmapSettings.SlopeThreshold - 10.0f)) / 10.0f;
            Weight = FMath::RoundToInt(T * 255.0f);
        }
        return Weight;
    }

    // Check biome-based layer
    if (BiomeData.Num() > Index)
    {
        const EBiomeType BiomeType = BiomeData[Index].BiomeType;
        if (Config.SplatmapSettings.BiomeToLayerMap.Contains(BiomeType))
        {
            if (Config.SplatmapSettings.BiomeToLayerMap[BiomeType] == LayerName)
            {
                Weight = 255; // Full weight for matching biome
            }
        }
    }

    // Check height-based layers
    for (int32 i = 0; i < Config.SplatmapSettings.HeightLayers.Num(); ++i)
    {
        if (Config.SplatmapSettings.HeightLayers[i] == LayerName &&
            i < Config.SplatmapSettings.HeightThresholds.Num())
        {
            const float Threshold = Config.SplatmapSettings.HeightThresholds[i];

            if (Height >= Threshold)
            {
                // Check if there's a next threshold
                if (i + 1 < Config.SplatmapSettings.HeightThresholds.Num())
                {
                    const float NextThreshold = Config.SplatmapSettings.HeightThresholds[i + 1];
                    if (Height < NextThreshold)
                    {
                        // Within this layer's range
                        Weight = 255;
                    }
                    else if (Height < NextThreshold + Config.SplatmapSettings.BlendDistance * 0.01f)
                    {
                        // Blend with next layer
                        const float T = (Height - NextThreshold) / (Config.SplatmapSettings.BlendDistance * 0.01f);
                        Weight = FMath::RoundToInt((1.0f - T) * 255.0f);
                    }
                }
                else
                {
                    // Highest layer
                    Weight = 255;
                }
            }
            else if (Height >= Threshold - Config.SplatmapSettings.BlendDistance * 0.01f)
            {
                // Blend with previous layer
                const float T = (Height - (Threshold - Config.SplatmapSettings.BlendDistance * 0.01f)) /
                               (Config.SplatmapSettings.BlendDistance * 0.01f);
                Weight = FMath::RoundToInt(T * 255.0f);
            }
            break;
        }
    }

    return Weight;
}

float UHarmoniaWorldGeneratorSubsystem::CalculateSlope(
    int32 X,
    int32 Y,
    const TArray<int32>& HeightData,
    const FWorldGeneratorConfig& Config)
{
    // Check bounds
    if (X <= 0 || X >= Config.SizeX - 1 || Y <= 0 || Y >= Config.SizeY - 1)
    {
        return 0.0f;
    }

    // Get neighboring heights
    const float HeightLeft = (float)HeightData[Y * Config.SizeX + (X - 1)] / 65535.f;
    const float HeightRight = (float)HeightData[Y * Config.SizeX + (X + 1)] / 65535.f;
    const float HeightUp = (float)HeightData[(Y - 1) * Config.SizeX + X] / 65535.f;
    const float HeightDown = (float)HeightData[(Y + 1) * Config.SizeX + X] / 65535.f;

    // Calculate slope
    const float SlopeX = FMath::Abs(HeightRight - HeightLeft) * Config.MaxHeight / 200.f;
    const float SlopeY = FMath::Abs(HeightDown - HeightUp) * Config.MaxHeight / 200.f;
    const float Slope = FMath::Sqrt(SlopeX * SlopeX + SlopeY * SlopeY);

    // Convert to degrees
    return FMath::RadiansToDegrees(FMath::Atan(Slope));
}

// ========================================
// Environment System Implementation
// ========================================

DEFINE_STAT(STAT_HarmoniaWorldGeneratorSubsystem);

void UHarmoniaWorldGeneratorSubsystem::InitializeEnvironmentSystem(const FEnvironmentSystemSettings& Settings)
{
    EnvironmentSettings = Settings;
    CurrentSeason = Settings.StartingSeason;
    SeasonProgress = 0.0f;
    TotalSeasonTime = 0.0f;
    CurrentWeather = EWeatherType::Clear;
    PreviousWeather = EWeatherType::Clear;
    WeatherTransitionProgress = 1.0f;
    TimeSinceLastWeatherChange = 0.0f;
    CurrentGameTime = Settings.DayNightSettings.StartingTimeOfDay;
    TimeSpeedMultiplier = 1.0f;

    // Initialize random stream with seed
    WeatherRandom.Initialize(FMath::Rand());

    UE_LOG(LogTemp, Log, TEXT("Environment System initialized - Season: %d, Time: %.2f"),
        (int32)CurrentSeason, CurrentGameTime);
}

void UHarmoniaWorldGeneratorSubsystem::StartEnvironmentSystem()
{
    if (!EnvironmentSettings.bEnableEnvironmentSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Environment System is disabled in settings"));
        return;
    }

    bEnvironmentSystemActive = true;
    UE_LOG(LogTemp, Log, TEXT("Environment System started"));

    // Broadcast initial state
    if (OnSeasonChanged.IsBound())
    {
        OnSeasonChanged.Broadcast(CurrentSeason, SeasonProgress);
    }

    if (OnWeatherChanged.IsBound())
    {
        OnWeatherChanged.Broadcast(CurrentWeather, PreviousWeather, 0.0f);
    }

    ETimeOfDay TimeOfDay = GetCurrentTimeOfDay();
    if (OnTimeOfDayChanged.IsBound())
    {
        OnTimeOfDayChanged.Broadcast(TimeOfDay, CurrentGameTime);
    }
}

void UHarmoniaWorldGeneratorSubsystem::StopEnvironmentSystem()
{
    bEnvironmentSystemActive = false;
    UE_LOG(LogTemp, Log, TEXT("Environment System stopped"));
}

void UHarmoniaWorldGeneratorSubsystem::SetCurrentSeason(ESeasonType NewSeason, bool bBroadcast)
{
    if (CurrentSeason != NewSeason)
    {
        CurrentSeason = NewSeason;
        SeasonProgress = 0.0f;
        TotalSeasonTime = 0.0f;

        if (bBroadcast && OnSeasonChanged.IsBound())
        {
            OnSeasonChanged.Broadcast(CurrentSeason, SeasonProgress);
        }

        // Trigger weather change based on new season
        if (EnvironmentSettings.bEnableDynamicWeather)
        {
            EWeatherType NewWeather = SelectRandomWeather(CurrentSeason, EnvironmentSettings.CurrentBiome);
            ChangeWeather(NewWeather, EnvironmentSettings.WeatherTransitionDuration);
        }
    }
}

void UHarmoniaWorldGeneratorSubsystem::ChangeWeather(EWeatherType NewWeather, float TransitionDuration)
{
    if (CurrentWeather != NewWeather)
    {
        PreviousWeather = CurrentWeather;
        CurrentWeather = NewWeather;
        WeatherTransitionProgress = 0.0f;
        TimeSinceLastWeatherChange = 0.0f;

        if (OnWeatherChanged.IsBound())
        {
            OnWeatherChanged.Broadcast(CurrentWeather, PreviousWeather, TransitionDuration);
        }

        UE_LOG(LogTemp, Log, TEXT("Weather changed: %d -> %d (Transition: %.1fs)"),
            (int32)PreviousWeather, (int32)CurrentWeather, TransitionDuration);
    }
}

ETimeOfDay UHarmoniaWorldGeneratorSubsystem::GetCurrentTimeOfDay() const
{
    if (CurrentGameTime >= EnvironmentSettings.DayNightSettings.SunriseTime &&
        CurrentGameTime < EnvironmentSettings.DayNightSettings.DayStartTime)
    {
        return ETimeOfDay::Dawn;
    }
    else if (CurrentGameTime >= EnvironmentSettings.DayNightSettings.DayStartTime &&
             CurrentGameTime < EnvironmentSettings.DayNightSettings.SunsetTime)
    {
        return ETimeOfDay::Day;
    }
    else if (CurrentGameTime >= EnvironmentSettings.DayNightSettings.SunsetTime &&
             CurrentGameTime < EnvironmentSettings.DayNightSettings.NightStartTime)
    {
        return ETimeOfDay::Dusk;
    }
    else
    {
        return ETimeOfDay::Night;
    }
}

void UHarmoniaWorldGeneratorSubsystem::SetCurrentGameTime(float Hours)
{
    CurrentGameTime = FMath::Fmod(Hours, 24.0f);
    if (CurrentGameTime < 0.0f)
    {
        CurrentGameTime += 24.0f;
    }
}

float UHarmoniaWorldGeneratorSubsystem::GetSunAngle() const
{
    return CalculateSunAngleFromTime(CurrentGameTime);
}

FSeasonVisuals UHarmoniaWorldGeneratorSubsystem::GetCurrentSeasonVisuals() const
{
    const FSeasonSettings* CurrentSettings = GetSeasonSettings(CurrentSeason);
    if (!CurrentSettings)
    {
        return FSeasonVisuals();
    }

    // If transitioning between seasons, interpolate visuals
    if (EnvironmentSettings.bEnableSeasons && SeasonProgress > 0.95f)
    {
        // Get next season
        ESeasonType NextSeason = static_cast<ESeasonType>((static_cast<int32>(CurrentSeason) + 1) % 4);
        const FSeasonSettings* NextSettings = GetSeasonSettings(NextSeason);

        if (NextSettings)
        {
            // Interpolate for smooth transition (last 5% of season duration)
            float TransitionAlpha = (SeasonProgress - 0.95f) / 0.05f;
            return InterpolateSeasonVisuals(
                CurrentSettings->Visuals,
                NextSettings->Visuals,
                TransitionAlpha
            );
        }
    }

    return CurrentSettings->Visuals;
}

void UHarmoniaWorldGeneratorSubsystem::SetTimeSpeed(float SpeedMultiplier)
{
    TimeSpeedMultiplier = FMath::Max(0.0f, SpeedMultiplier);
}

void UHarmoniaWorldGeneratorSubsystem::Tick(float DeltaTime)
{
    if (!bEnvironmentSystemActive)
    {
        return;
    }

    // Apply time speed multiplier
    float ScaledDeltaTime = DeltaTime * TimeSpeedMultiplier;

    // Update systems
    UpdateDayNightCycle(ScaledDeltaTime);

    if (EnvironmentSettings.bEnableSeasons)
    {
        UpdateSeasonProgression(ScaledDeltaTime);
    }

    if (EnvironmentSettings.bEnableDynamicWeather)
    {
        UpdateWeather(ScaledDeltaTime);
    }
}

TStatId UHarmoniaWorldGeneratorSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UHarmoniaWorldGeneratorSubsystem, STATGROUP_Tickables);
}

void UHarmoniaWorldGeneratorSubsystem::UpdateDayNightCycle(float DeltaTime)
{
    ETimeOfDay PreviousTimeOfDay = GetCurrentTimeOfDay();

    // Convert real-time seconds to game-time hours
    float GameHoursPerRealSecond = 24.0f / EnvironmentSettings.DayNightSettings.DayLengthInMinutes / 60.0f;
    CurrentGameTime += DeltaTime * GameHoursPerRealSecond;

    // Wrap around 24 hours
    if (CurrentGameTime >= 24.0f)
    {
        CurrentGameTime = FMath::Fmod(CurrentGameTime, 24.0f);
    }

    // Check for time of day change
    ETimeOfDay NewTimeOfDay = GetCurrentTimeOfDay();
    if (NewTimeOfDay != PreviousTimeOfDay)
    {
        if (OnTimeOfDayChanged.IsBound())
        {
            OnTimeOfDayChanged.Broadcast(NewTimeOfDay, CurrentGameTime);
        }
    }

    // Broadcast tick event
    if (OnDayNightCycleTick.IsBound())
    {
        float SunAngle = GetSunAngle();
        OnDayNightCycleTick.Broadcast(CurrentGameTime, SunAngle);
    }
}

void UHarmoniaWorldGeneratorSubsystem::UpdateSeasonProgression(float DeltaTime)
{
    const FSeasonSettings* CurrentSettings = GetSeasonSettings(CurrentSeason);
    if (!CurrentSettings)
    {
        return;
    }

    // Calculate season duration in real seconds
    float SeasonDurationSeconds = CurrentSettings->DurationInGameDays *
                                   EnvironmentSettings.DayNightSettings.DayLengthInMinutes * 60.0f;

    TotalSeasonTime += DeltaTime;
    SeasonProgress = TotalSeasonTime / SeasonDurationSeconds;

    // Check for season change
    if (SeasonProgress >= 1.0f)
    {
        ESeasonType NextSeason = static_cast<ESeasonType>((static_cast<int32>(CurrentSeason) + 1) % 4);
        SetCurrentSeason(NextSeason, true);
    }
    else if (FMath::Fmod(TotalSeasonTime, 60.0f) < DeltaTime) // Broadcast every minute
    {
        if (OnSeasonChanged.IsBound())
        {
            OnSeasonChanged.Broadcast(CurrentSeason, SeasonProgress);
        }
    }
}

void UHarmoniaWorldGeneratorSubsystem::UpdateWeather(float DeltaTime)
{
    TimeSinceLastWeatherChange += DeltaTime;

    // Update weather transition
    if (WeatherTransitionProgress < 1.0f)
    {
        WeatherTransitionProgress += DeltaTime / EnvironmentSettings.WeatherTransitionDuration;
        WeatherTransitionProgress = FMath::Clamp(WeatherTransitionProgress, 0.0f, 1.0f);
    }

    // Check if it's time to change weather
    if (TimeSinceLastWeatherChange >= EnvironmentSettings.MinTimeBetweenWeatherChanges * 60.0f)
    {
        // Random chance to change weather (20% chance per check)
        if (WeatherRandom.FRand() < 0.2f)
        {
            EWeatherType NewWeather = SelectRandomWeather(CurrentSeason, EnvironmentSettings.CurrentBiome);
            if (NewWeather != CurrentWeather)
            {
                ChangeWeather(NewWeather, EnvironmentSettings.WeatherTransitionDuration);
            }
        }
    }
}

EWeatherType UHarmoniaWorldGeneratorSubsystem::SelectRandomWeather(ESeasonType Season, EBiomeType Biome)
{
    const FSeasonSettings* SeasonSettings = GetSeasonSettings(Season);
    if (!SeasonSettings)
    {
        return EWeatherType::Clear;
    }

    // Build probability map
    TMap<EWeatherType, float> ProbabilityMap = SeasonSettings->DefaultWeatherProbabilities;

    // Check for biome-specific overrides
    for (const FWeatherProbability& Override : SeasonSettings->BiomeWeatherOverrides)
    {
        if (Override.Biome == Biome)
        {
            // Apply biome override
            for (const auto& Pair : Override.WeatherProbabilities)
            {
                ProbabilityMap.Add(Pair.Key, Pair.Value);
            }
            break;
        }
    }

    // Calculate total probability
    float TotalProbability = 0.0f;
    for (const auto& Pair : ProbabilityMap)
    {
        TotalProbability += Pair.Value;
    }

    if (TotalProbability <= 0.0f)
    {
        return EWeatherType::Clear;
    }

    // Select random weather based on probability
    float RandomValue = WeatherRandom.FRand() * TotalProbability;
    float CumulativeProbability = 0.0f;

    for (const auto& Pair : ProbabilityMap)
    {
        CumulativeProbability += Pair.Value;
        if (RandomValue <= CumulativeProbability)
        {
            return Pair.Key;
        }
    }

    return EWeatherType::Clear;
}

float UHarmoniaWorldGeneratorSubsystem::CalculateSunAngleFromTime(float GameTime) const
{
    // 0 hours = 0 degrees (midnight)
    // 6 hours = 90 degrees (sunrise)
    // 12 hours = 180 degrees (noon)
    // 18 hours = 270 degrees (sunset)
    // 24 hours = 360 degrees (midnight)
    return (GameTime / 24.0f) * 360.0f;
}

const FSeasonSettings* UHarmoniaWorldGeneratorSubsystem::GetSeasonSettings(ESeasonType Season) const
{
    for (const FSeasonSettings& Settings : EnvironmentSettings.SeasonConfigs)
    {
        if (Settings.Season == Season)
        {
            return &Settings;
        }
    }
    return nullptr;
}

FSeasonVisuals UHarmoniaWorldGeneratorSubsystem::InterpolateSeasonVisuals(
    const FSeasonVisuals& From,
    const FSeasonVisuals& To,
    float Alpha) const
{
    FSeasonVisuals Result;
    Result.FoliageTint = FLinearColor::LerpUsingHSV(From.FoliageTint, To.FoliageTint, Alpha);
    Result.GrassTint = FLinearColor::LerpUsingHSV(From.GrassTint, To.GrassTint, Alpha);
    Result.SnowCoverage = FMath::Lerp(From.SnowCoverage, To.SnowCoverage, Alpha);
    Result.Temperature = FMath::Lerp(From.Temperature, To.Temperature, Alpha);
    return Result;
}

// ========================================
// Runtime Terrain Modification Implementation
// ========================================

#include "LandscapeEdit.h"
#include "LandscapeDataAccess.h"
#include "LandscapeComponent.h"

FTerrainModificationResult UHarmoniaWorldGeneratorSubsystem::ApplyTerrainModification(
    ALandscape* Landscape,
    const FTerrainModification& Modification)
{
    FTerrainModificationResult Result;
    Result.bSuccess = false;

    if (!Landscape)
    {
        Result.ErrorMessage = TEXT("Landscape is null");
        return Result;
    }

    // Create appropriate modification based on type
    switch (Modification.ModificationType)
    {
    case ETerrainModificationType::Raise:
        return RaiseTerrain(Landscape, Modification.Location, Modification.Radius,
                          Modification.Strength * 100.0f, Modification.FalloffType);

    case ETerrainModificationType::Lower:
        return LowerTerrain(Landscape, Modification.Location, Modification.Radius,
                          Modification.Strength * 100.0f, Modification.FalloffType);

    case ETerrainModificationType::Flatten:
        return FlattenTerrain(Landscape, Modification.Location, Modification.Radius,
                            Modification.TargetHeight, Modification.FalloffType);

    case ETerrainModificationType::Smooth:
        return SmoothTerrain(Landscape, Modification.Location, Modification.Radius,
                           Modification.SmoothIterations);

    case ETerrainModificationType::Crater:
        return CreateCrater(Landscape, Modification.Location, Modification.Radius,
                          Modification.Strength * 200.0f, Modification.FalloffType);

    case ETerrainModificationType::Hill:
        return CreateHill(Landscape, Modification.Location, Modification.Radius,
                        Modification.Strength * 200.0f, Modification.FalloffType);

    case ETerrainModificationType::SetHeight:
        return FlattenTerrain(Landscape, Modification.Location, Modification.Radius,
                            Modification.TargetHeight, ETerrainFalloffType::Sharp);

    case ETerrainModificationType::Paint:
        return PaintLandscapeLayer(Landscape, Modification.Location, Modification.Radius,
                                 Modification.PaintLayerName, Modification.Strength,
                                 Modification.FalloffType);

    default:
        Result.ErrorMessage = TEXT("Unknown modification type");
        return Result;
    }
}

FTerrainModificationResult UHarmoniaWorldGeneratorSubsystem::CreateCrater(
    ALandscape* Landscape,
    FVector Location,
    float Radius,
    float Depth,
    ETerrainFalloffType FalloffType)
{
    FTerrainModificationResult Result;
    Result.bSuccess = false;

    if (!Landscape)
    {
        Result.ErrorMessage = TEXT("Landscape is null");
        return Result;
    }

    // Get landscape data
    TArray<uint16> HeightData;
    int32 MinX, MinY, MaxX, MaxY;

    if (!GetLandscapeHeightData(Landscape, Location, Radius, HeightData, MinX, MinY, MaxX, MaxY))
    {
        Result.ErrorMessage = TEXT("Failed to get landscape height data");
        return Result;
    }

    const int32 SizeX = MaxX - MinX + 1;
    const int32 SizeY = MaxY - MinY + 1;
    const FVector2D LandscapeCoord = WorldToLandscapeCoordinates(Landscape, Location);

    // Modify heights to create crater
    for (int32 Y = 0; Y < SizeY; ++Y)
    {
        for (int32 X = 0; X < SizeX; ++X)
        {
            const int32 Index = Y * SizeX + X;
            const FVector2D CurrentCoord(MinX + X, MinY + Y);
            const float Distance = FVector2D::Distance(CurrentCoord, LandscapeCoord);

            if (Distance <= Radius)
            {
                const float Falloff = CalculateFalloff(Distance, Radius, FalloffType);
                const float CurrentHeight = (float)HeightData[Index];
                const float CraterDepth = Depth * Falloff;

                // Create crater with raised rim (20% of radius)
                float HeightChange;
                if (Distance < Radius * 0.2f)
                {
                    // Rim - slightly raised
                    HeightChange = CraterDepth * 0.3f;
                }
                else
                {
                    // Crater depression
                    HeightChange = -CraterDepth;
                }

                const float NewHeight = FMath::Clamp(CurrentHeight + HeightChange, 0.0f, 65535.0f);
                HeightData[Index] = (uint16)NewHeight;
                Result.ModifiedVertices++;
            }
        }
    }

    // Set modified data back
    if (SetLandscapeHeightData(Landscape, HeightData, MinX, MinY, MaxX, MaxY))
    {
        Result.bSuccess = true;
        Result.AffectedComponents = 1; // Simplified
    }
    else
    {
        Result.ErrorMessage = TEXT("Failed to set landscape height data");
    }

    return Result;
}

FTerrainModificationResult UHarmoniaWorldGeneratorSubsystem::FlattenTerrain(
    ALandscape* Landscape,
    FVector Location,
    float Radius,
    float TargetHeight,
    ETerrainFalloffType FalloffType)
{
    FTerrainModificationResult Result;
    Result.bSuccess = false;

    if (!Landscape)
    {
        Result.ErrorMessage = TEXT("Landscape is null");
        return Result;
    }

    TArray<uint16> HeightData;
    int32 MinX, MinY, MaxX, MaxY;

    if (!GetLandscapeHeightData(Landscape, Location, Radius, HeightData, MinX, MinY, MaxX, MaxY))
    {
        Result.ErrorMessage = TEXT("Failed to get landscape height data");
        return Result;
    }

    const int32 SizeX = MaxX - MinX + 1;
    const int32 SizeY = MaxY - MinY + 1;
    const FVector2D LandscapeCoord = WorldToLandscapeCoordinates(Landscape, Location);
    const uint16 TargetHeightValue = (uint16)FMath::Clamp(TargetHeight * 128.0f, 0.0f, 65535.0f);

    for (int32 Y = 0; Y < SizeY; ++Y)
    {
        for (int32 X = 0; X < SizeX; ++X)
        {
            const int32 Index = Y * SizeX + X;
            const FVector2D CurrentCoord(MinX + X, MinY + Y);
            const float Distance = FVector2D::Distance(CurrentCoord, LandscapeCoord);

            if (Distance <= Radius)
            {
                const float Falloff = CalculateFalloff(Distance, Radius, FalloffType);
                const float CurrentHeight = (float)HeightData[Index];
                const float NewHeight = FMath::Lerp(CurrentHeight, (float)TargetHeightValue, Falloff);

                HeightData[Index] = (uint16)FMath::Clamp(NewHeight, 0.0f, 65535.0f);
                Result.ModifiedVertices++;
            }
        }
    }

    if (SetLandscapeHeightData(Landscape, HeightData, MinX, MinY, MaxX, MaxY))
    {
        Result.bSuccess = true;
        Result.AffectedComponents = 1;
    }
    else
    {
        Result.ErrorMessage = TEXT("Failed to set landscape height data");
    }

    return Result;
}

FTerrainModificationResult UHarmoniaWorldGeneratorSubsystem::CreateHill(
    ALandscape* Landscape,
    FVector Location,
    float Radius,
    float Height,
    ETerrainFalloffType FalloffType)
{
    FTerrainModificationResult Result;
    Result.bSuccess = false;

    if (!Landscape)
    {
        Result.ErrorMessage = TEXT("Landscape is null");
        return Result;
    }

    TArray<uint16> HeightData;
    int32 MinX, MinY, MaxX, MaxY;

    if (!GetLandscapeHeightData(Landscape, Location, Radius, HeightData, MinX, MinY, MaxX, MaxY))
    {
        Result.ErrorMessage = TEXT("Failed to get landscape height data");
        return Result;
    }

    const int32 SizeX = MaxX - MinX + 1;
    const int32 SizeY = MaxY - MinY + 1;
    const FVector2D LandscapeCoord = WorldToLandscapeCoordinates(Landscape, Location);

    for (int32 Y = 0; Y < SizeY; ++Y)
    {
        for (int32 X = 0; X < SizeX; ++X)
        {
            const int32 Index = Y * SizeX + X;
            const FVector2D CurrentCoord(MinX + X, MinY + Y);
            const float Distance = FVector2D::Distance(CurrentCoord, LandscapeCoord);

            if (Distance <= Radius)
            {
                const float Falloff = CalculateFalloff(Distance, Radius, FalloffType);
                const float CurrentHeight = (float)HeightData[Index];
                const float HeightChange = Height * Falloff;
                const float NewHeight = FMath::Clamp(CurrentHeight + HeightChange, 0.0f, 65535.0f);

                HeightData[Index] = (uint16)NewHeight;
                Result.ModifiedVertices++;
            }
        }
    }

    if (SetLandscapeHeightData(Landscape, HeightData, MinX, MinY, MaxX, MaxY))
    {
        Result.bSuccess = true;
        Result.AffectedComponents = 1;
    }
    else
    {
        Result.ErrorMessage = TEXT("Failed to set landscape height data");
    }

    return Result;
}

FTerrainModificationResult UHarmoniaWorldGeneratorSubsystem::SmoothTerrain(
    ALandscape* Landscape,
    FVector Location,
    float Radius,
    int32 Iterations)
{
    FTerrainModificationResult Result;
    Result.bSuccess = false;

    if (!Landscape)
    {
        Result.ErrorMessage = TEXT("Landscape is null");
        return Result;
    }

    TArray<uint16> HeightData;
    int32 MinX, MinY, MaxX, MaxY;

    if (!GetLandscapeHeightData(Landscape, Location, Radius, HeightData, MinX, MinY, MaxX, MaxY))
    {
        Result.ErrorMessage = TEXT("Failed to get landscape height data");
        return Result;
    }

    const int32 SizeX = MaxX - MinX + 1;
    const int32 SizeY = MaxY - MinY + 1;
    const FVector2D LandscapeCoord = WorldToLandscapeCoordinates(Landscape, Location);

    // Perform smoothing iterations
    for (int32 Iter = 0; Iter < Iterations; ++Iter)
    {
        TArray<uint16> TempData = HeightData;

        for (int32 Y = 1; Y < SizeY - 1; ++Y)
        {
            for (int32 X = 1; X < SizeX - 1; ++X)
            {
                const FVector2D CurrentCoord(MinX + X, MinY + Y);
                const float Distance = FVector2D::Distance(CurrentCoord, LandscapeCoord);

                if (Distance <= Radius)
                {
                    // 3x3 box filter
                    float Sum = 0.0f;
                    for (int32 DY = -1; DY <= 1; ++DY)
                    {
                        for (int32 DX = -1; DX <= 1; ++DX)
                        {
                            const int32 SampleIndex = (Y + DY) * SizeX + (X + DX);
                            Sum += (float)HeightData[SampleIndex];
                        }
                    }

                    const int32 Index = Y * SizeX + X;
                    TempData[Index] = (uint16)(Sum / 9.0f);
                    Result.ModifiedVertices++;
                }
            }
        }

        HeightData = TempData;
    }

    if (SetLandscapeHeightData(Landscape, HeightData, MinX, MinY, MaxX, MaxY))
    {
        Result.bSuccess = true;
        Result.AffectedComponents = 1;
    }
    else
    {
        Result.ErrorMessage = TEXT("Failed to set landscape height data");
    }

    return Result;
}

FTerrainModificationResult UHarmoniaWorldGeneratorSubsystem::RaiseTerrain(
    ALandscape* Landscape,
    FVector Location,
    float Radius,
    float Amount,
    ETerrainFalloffType FalloffType)
{
    return CreateHill(Landscape, Location, Radius, Amount, FalloffType);
}

FTerrainModificationResult UHarmoniaWorldGeneratorSubsystem::LowerTerrain(
    ALandscape* Landscape,
    FVector Location,
    float Radius,
    float Amount,
    ETerrainFalloffType FalloffType)
{
    return CreateHill(Landscape, Location, Radius, -Amount, FalloffType);
}

FTerrainModificationResult UHarmoniaWorldGeneratorSubsystem::PaintLandscapeLayer(
    ALandscape* Landscape,
    FVector Location,
    float Radius,
    FName LayerName,
    float Strength,
    ETerrainFalloffType FalloffType)
{
    FTerrainModificationResult Result;
    Result.bSuccess = false;
    Result.ErrorMessage = TEXT("Landscape painting not yet fully implemented - requires layer info access");
    return Result;
}

// ========================================
// Helper Functions
// ========================================

float UHarmoniaWorldGeneratorSubsystem::CalculateFalloff(
    float Distance,
    float Radius,
    ETerrainFalloffType FalloffType) const
{
    if (Distance >= Radius)
    {
        return 0.0f;
    }

    const float NormalizedDistance = Distance / Radius;

    switch (FalloffType)
    {
    case ETerrainFalloffType::Linear:
        return 1.0f - NormalizedDistance;

    case ETerrainFalloffType::Smooth:
        // Cosine interpolation
        return 0.5f * (1.0f + FMath::Cos(NormalizedDistance * PI));

    case ETerrainFalloffType::Spherical:
        return FMath::Sqrt(1.0f - NormalizedDistance * NormalizedDistance);

    case ETerrainFalloffType::Gaussian:
        // Gaussian falloff (bell curve)
        return FMath::Exp(-4.5f * NormalizedDistance * NormalizedDistance);

    case ETerrainFalloffType::Sharp:
        return 1.0f;

    default:
        return 1.0f - NormalizedDistance;
    }
}

bool UHarmoniaWorldGeneratorSubsystem::GetLandscapeHeightData(
    ALandscape* Landscape,
    FVector Center,
    float Radius,
    TArray<uint16>& OutHeightData,
    int32& OutMinX,
    int32& OutMinY,
    int32& OutMaxX,
    int32& OutMaxY)
{
    if (!Landscape)
    {
        return false;
    }

    const FVector2D LandscapeCoord = WorldToLandscapeCoordinates(Landscape, Center);

    // Calculate bounds
    OutMinX = FMath::FloorToInt(LandscapeCoord.X - Radius);
    OutMinY = FMath::FloorToInt(LandscapeCoord.Y - Radius);
    OutMaxX = FMath::CeilToInt(LandscapeCoord.X + Radius);
    OutMaxY = FMath::CeilToInt(LandscapeCoord.Y + Radius);

    const int32 SizeX = OutMaxX - OutMinX + 1;
    const int32 SizeY = OutMaxY - OutMinY + 1;

    OutHeightData.SetNum(SizeX * SizeY);

    // Get landscape data using edit interface
    FLandscapeEditDataInterface LandscapeEdit(Landscape->GetLandscapeInfo());

    for (int32 Y = 0; Y < SizeY; ++Y)
    {
        for (int32 X = 0; X < SizeX; ++X)
        {
            const int32 Index = Y * SizeX + X;
            const int32 LandscapeX = OutMinX + X;
            const int32 LandscapeY = OutMinY + Y;

            OutHeightData[Index] = LandscapeEdit.GetHeight(LandscapeX, LandscapeY);
        }
    }

    return true;
}

bool UHarmoniaWorldGeneratorSubsystem::SetLandscapeHeightData(
    ALandscape* Landscape,
    const TArray<uint16>& HeightData,
    int32 MinX,
    int32 MinY,
    int32 MaxX,
    int32 MaxY)
{
    if (!Landscape)
    {
        return false;
    }

    const int32 SizeX = MaxX - MinX + 1;
    const int32 SizeY = MaxY - MinY + 1;

    if (HeightData.Num() != SizeX * SizeY)
    {
        return false;
    }

    // Set landscape data using edit interface
    FLandscapeEditDataInterface LandscapeEdit(Landscape->GetLandscapeInfo());

    for (int32 Y = 0; Y < SizeY; ++Y)
    {
        for (int32 X = 0; X < SizeX; ++X)
        {
            const int32 Index = Y * SizeX + X;
            const int32 LandscapeX = MinX + X;
            const int32 LandscapeY = MinY + Y;

            LandscapeEdit.SetHeight(LandscapeX, LandscapeY, HeightData[Index]);
        }
    }

    LandscapeEdit.Flush();

    return true;
}

FVector2D UHarmoniaWorldGeneratorSubsystem::WorldToLandscapeCoordinates(
    ALandscape* Landscape,
    FVector WorldLocation) const
{
    if (!Landscape)
    {
        return FVector2D::ZeroVector;
    }

    const FTransform& LandscapeTransform = Landscape->GetActorTransform();
    const FVector LocalLocation = LandscapeTransform.InverseTransformPosition(WorldLocation);

    // Convert to landscape coordinates (assuming default scale)
    const float ScaleXY = Landscape->GetActorScale3D().X;
    return FVector2D(LocalLocation.X / ScaleXY, LocalLocation.Y / ScaleXY);
}

// ========================================
// Chunk Caching System Implementation
// ========================================

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"

void UHarmoniaWorldGeneratorSubsystem::InitializeChunkCache(const FChunkCacheSettings& Settings)
{
    CacheSettings = Settings;
    ChunkCache.Empty();
    ChunkAccessOrder.Empty();

    if (CacheSettings.bEnableDiskCache)
    {
        // Create cache directory if it doesn't exist
        const FString CachePath = FPaths::ProjectSavedDir() / CacheSettings.CacheDirectory;
        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        if (!PlatformFile.DirectoryExists(*CachePath))
        {
            PlatformFile.CreateDirectory(*CachePath);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Chunk cache initialized - Max chunks: %d, Disk cache: %s"),
        CacheSettings.MaxCachedChunks, CacheSettings.bEnableDiskCache ? TEXT("Enabled") : TEXT("Disabled"));
}

bool UHarmoniaWorldGeneratorSubsystem::GetCachedChunk(FIntPoint ChunkCoordinates, FWorldChunkData& OutChunkData)
{
    if (!CacheSettings.bEnableCaching)
    {
        return false;
    }

    // Check memory cache first
    if (FWorldChunkData* CachedData = ChunkCache.Find(ChunkCoordinates))
    {
        // Check if expired
        if (CacheSettings.CacheExpirationHours > 0.0f)
        {
            FTimespan TimeSinceGeneration = FDateTime::Now() - CachedData->GenerationTime;
            if (TimeSinceGeneration.GetTotalHours() > CacheSettings.CacheExpirationHours)
            {
                // Expired - remove from cache
                ChunkCache.Remove(ChunkCoordinates);
                ChunkAccessOrder.Remove(ChunkCoordinates);
                return false;
            }
        }

        // Update access order for LRU
        ChunkAccessOrder.Remove(ChunkCoordinates);
        ChunkAccessOrder.Add(ChunkCoordinates);

        OutChunkData = *CachedData;
        return true;
    }

    // Try disk cache
    if (CacheSettings.bEnableDiskCache)
    {
        const FString FilePath = GetChunkCacheFilePath(ChunkCoordinates);
        if (FPaths::FileExists(FilePath))
        {
            TArray<uint8> FileData;
            if (FFileHelper::LoadFileToArray(FileData, *FilePath))
            {
                FMemoryReader MemoryReader(FileData, true);
                MemoryReader << OutChunkData;

                // Add to memory cache
                CacheChunk(OutChunkData);

                return true;
            }
        }
    }

    return false;
}

void UHarmoniaWorldGeneratorSubsystem::CacheChunk(const FWorldChunkData& ChunkData)
{
    if (!CacheSettings.bEnableCaching)
    {
        return;
    }

    // Evict LRU chunks if cache is full
    if (ChunkCache.Num() >= CacheSettings.MaxCachedChunks)
    {
        EvictLRUChunks();
    }

    // Calculate hash
    FWorldChunkData DataWithHash = ChunkData;
    DataWithHash.CacheHash = CalculateChunkHash(ChunkData);
    DataWithHash.GenerationTime = FDateTime::Now();

    // Add to memory cache
    ChunkCache.Add(ChunkData.ChunkCoordinates, DataWithHash);
    ChunkAccessOrder.Add(ChunkData.ChunkCoordinates);

    // Save to disk if enabled
    if (CacheSettings.bEnableDiskCache && CacheSettings.bAutoSaveCache)
    {
        const FString FilePath = GetChunkCacheFilePath(ChunkData.ChunkCoordinates);
        TArray<uint8> FileData;
        FMemoryWriter MemoryWriter(FileData, true);
        FWorldChunkData WritableData = DataWithHash;
        MemoryWriter << WritableData;

        FFileHelper::SaveArrayToFile(FileData, *FilePath);
    }
}

void UHarmoniaWorldGeneratorSubsystem::ClearChunkCache()
{
    ChunkCache.Empty();
    ChunkAccessOrder.Empty();
    UE_LOG(LogTemp, Log, TEXT("Chunk cache cleared"));
}

bool UHarmoniaWorldGeneratorSubsystem::SaveChunkCacheToDisk()
{
    if (!CacheSettings.bEnableDiskCache)
    {
        return false;
    }

    int32 SavedChunks = 0;
    for (const auto& Pair : ChunkCache)
    {
        const FString FilePath = GetChunkCacheFilePath(Pair.Key);
        TArray<uint8> FileData;
        FMemoryWriter MemoryWriter(FileData, true);
        FWorldChunkData WritableData = Pair.Value;
        MemoryWriter << WritableData;

        if (FFileHelper::SaveArrayToFile(FileData, *FilePath))
        {
            SavedChunks++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Saved %d chunks to disk cache"), SavedChunks);
    return SavedChunks > 0;
}

bool UHarmoniaWorldGeneratorSubsystem::LoadChunkCacheFromDisk()
{
    if (!CacheSettings.bEnableDiskCache)
    {
        return false;
    }

    const FString CachePath = FPaths::ProjectSavedDir() / CacheSettings.CacheDirectory;
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    TArray<FString> CacheFiles;
    PlatformFile.FindFiles(CacheFiles, *CachePath, TEXT(".chunk"));

    int32 LoadedChunks = 0;
    for (const FString& FilePath : CacheFiles)
    {
        TArray<uint8> FileData;
        if (FFileHelper::LoadFileToArray(FileData, *FilePath))
        {
            FWorldChunkData ChunkData;
            FMemoryReader MemoryReader(FileData, true);
            MemoryReader << ChunkData;

            // Validate hash
            int32 ExpectedHash = CalculateChunkHash(ChunkData);
            if (ChunkData.CacheHash == ExpectedHash)
            {
                ChunkCache.Add(ChunkData.ChunkCoordinates, ChunkData);
                ChunkAccessOrder.Add(ChunkData.ChunkCoordinates);
                LoadedChunks++;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Loaded %d chunks from disk cache"), LoadedChunks);
    return LoadedChunks > 0;
}

void UHarmoniaWorldGeneratorSubsystem::GetCacheStatistics(
    int32& OutCachedChunks,
    int32& OutMemoryUsageKB,
    int32& OutDiskCacheSize)
{
    OutCachedChunks = ChunkCache.Num();
    OutMemoryUsageKB = CalculateCacheMemoryUsage() / 1024;

    OutDiskCacheSize = 0;
    if (CacheSettings.bEnableDiskCache)
    {
        const FString CachePath = FPaths::ProjectSavedDir() / CacheSettings.CacheDirectory;
        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

        TArray<FString> CacheFiles;
        PlatformFile.FindFilesRecursively(CacheFiles, *CachePath, TEXT(".chunk"));

        for (const FString& FilePath : CacheFiles)
        {
            OutDiskCacheSize += PlatformFile.FileSize(*FilePath);
        }
    }
}

void UHarmoniaWorldGeneratorSubsystem::CleanExpiredChunks()
{
    if (CacheSettings.CacheExpirationHours <= 0.0f)
    {
        return;
    }

    TArray<FIntPoint> ExpiredChunks;
    for (const auto& Pair : ChunkCache)
    {
        FTimespan TimeSinceGeneration = FDateTime::Now() - Pair.Value.GenerationTime;
        if (TimeSinceGeneration.GetTotalHours() > CacheSettings.CacheExpirationHours)
        {
            ExpiredChunks.Add(Pair.Key);
        }
    }

    for (const FIntPoint& ChunkCoord : ExpiredChunks)
    {
        ChunkCache.Remove(ChunkCoord);
        ChunkAccessOrder.Remove(ChunkCoord);

        // Delete disk cache file
        if (CacheSettings.bEnableDiskCache)
        {
            const FString FilePath = GetChunkCacheFilePath(ChunkCoord);
            IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
            PlatformFile.DeleteFile(*FilePath);
        }
    }

    if (ExpiredChunks.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Cleaned %d expired chunks from cache"), ExpiredChunks.Num());
    }
}

bool UHarmoniaWorldGeneratorSubsystem::IsChunkCached(FIntPoint ChunkCoordinates) const
{
    return ChunkCache.Contains(ChunkCoordinates);
}

void UHarmoniaWorldGeneratorSubsystem::PreloadChunksInRegion(FIntPoint MinChunk, FIntPoint MaxChunk)
{
    for (int32 Y = MinChunk.Y; Y <= MaxChunk.Y; ++Y)
    {
        for (int32 X = MinChunk.X; X <= MaxChunk.X; ++X)
        {
            FIntPoint ChunkCoord(X, Y);
            if (!IsChunkCached(ChunkCoord) && CacheSettings.bEnableDiskCache)
            {
                FWorldChunkData ChunkData;
                GetCachedChunk(ChunkCoord, ChunkData);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Preloaded chunks in region (%d,%d) to (%d,%d)"),
        MinChunk.X, MinChunk.Y, MaxChunk.X, MaxChunk.Y);
}

// Helper functions

int32 UHarmoniaWorldGeneratorSubsystem::CalculateChunkHash(const FWorldChunkData& ChunkData) const
{
    int32 Hash = 0;

    // Hash chunk coordinates
    Hash = ChunkData.ChunkCoordinates.X * 73856093 ^ ChunkData.ChunkCoordinates.Y * 19349663;

    // Hash height data
    for (int32 Height : ChunkData.HeightData)
    {
        Hash = Hash * 31 + Height;
    }

    // Hash object count (simple)
    Hash = Hash * 31 + ChunkData.Objects.Num();

    return Hash;
}

FString UHarmoniaWorldGeneratorSubsystem::GetChunkCacheFilePath(FIntPoint ChunkCoordinates) const
{
    const FString CachePath = FPaths::ProjectSavedDir() / CacheSettings.CacheDirectory;
    return FString::Printf(TEXT("%s/chunk_%d_%d.chunk"), *CachePath, ChunkCoordinates.X, ChunkCoordinates.Y);
}

void UHarmoniaWorldGeneratorSubsystem::EvictLRUChunks()
{
    // Remove 10% of oldest chunks
    int32 ChunksToRemove = FMath::Max(1, CacheSettings.MaxCachedChunks / 10);

    for (int32 i = 0; i < ChunksToRemove && ChunkAccessOrder.Num() > 0; ++i)
    {
        FIntPoint OldestChunk = ChunkAccessOrder[0];
        ChunkCache.Remove(OldestChunk);
        ChunkAccessOrder.RemoveAt(0);
    }

    UE_LOG(LogTemp, Verbose, TEXT("Evicted %d LRU chunks from cache"), ChunksToRemove);
}

int32 UHarmoniaWorldGeneratorSubsystem::CalculateCacheMemoryUsage() const
{
    int32 TotalSize = 0;

    for (const auto& Pair : ChunkCache)
    {
        const FWorldChunkData& ChunkData = Pair.Value;

        // Estimate size of arrays
        TotalSize += ChunkData.HeightData.Num() * sizeof(int32);
        TotalSize += ChunkData.Objects.Num() * sizeof(FWorldObjectData);
        TotalSize += ChunkData.BiomeData.Num() * sizeof(FBiomeData);
        TotalSize += sizeof(FWorldChunkData); // Struct overhead
    }

    return TotalSize;
}
