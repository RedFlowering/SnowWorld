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
        Config.Seed + 1000,
        Config.TemperatureNoiseSettings
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
        Config.Seed + 2000,
        Config.MoistureNoiseSettings
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
                const float Distance = FMath::Sqrt(dx * dx + dy * dy);
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
