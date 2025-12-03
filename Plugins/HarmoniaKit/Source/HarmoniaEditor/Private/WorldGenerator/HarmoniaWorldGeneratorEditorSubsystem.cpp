// Copyright 2025 Snow Game Studio.

#include "WorldGenerator/HarmoniaWorldGeneratorEditorSubsystem.h"
#include "HarmoniaWorldGeneratorSubsystem.h"
#include "DrawDebugHelpers.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/GameInstance.h"

// ========================================
// Helper Functions
// ========================================

UHarmoniaWorldGeneratorSubsystem* UHarmoniaWorldGeneratorEditorSubsystem::GetWorldGeneratorSubsystem(UWorld* World) const
{
	if (!World || !World->GetGameInstance())
	{
		return nullptr;
	}

	return World->GetGameInstance()->GetSubsystem<UHarmoniaWorldGeneratorSubsystem>();
}

// ========================================
// Debug Visualization Functions
// ========================================

void UHarmoniaWorldGeneratorEditorSubsystem::DrawBiomeDebugVisualization(
	UWorld* World,
	const TArray<FBiomeData>& BiomeData,
	const FWorldGeneratorConfig& Config,
	float Duration)
{
	if (!World || BiomeData.Num() == 0)
	{
		return;
	}

	const int32 MapSize = Config.SizeX;
	const float TileSize = 100.0f; // Default landscape quad size

	// Draw biome grid
	for (int32 Y = 0; Y < MapSize; Y += 10) // Sample every 10 tiles for performance
	{
		for (int32 X = 0; X < MapSize; X += 10)
		{
			const int32 Index = Y * MapSize + X;
			if (Index >= BiomeData.Num())
			{
				continue;
			}

			const FBiomeData& Biome = BiomeData[Index];
			const FVector Location(X * TileSize, Y * TileSize, 0.0f);
			const FColor BiomeColor = GetBiomeDebugColor(Biome.BiomeType);

			// Draw biome point
			DrawDebugPoint(World, Location, 15.0f, BiomeColor, false, Duration);

			// Draw biome label every 50 tiles
			if (X % 50 == 0 && Y % 50 == 0)
			{
				FString BiomeName;
				switch (Biome.BiomeType)
				{
					case EBiomeType::Ocean: BiomeName = TEXT("Ocean"); break;
					case EBiomeType::Beach: BiomeName = TEXT("Beach"); break;
					case EBiomeType::Desert: BiomeName = TEXT("Desert"); break;
					case EBiomeType::Grassland: BiomeName = TEXT("Grassland"); break;
					case EBiomeType::Forest: BiomeName = TEXT("Forest"); break;
					case EBiomeType::Taiga: BiomeName = TEXT("Taiga"); break;
					case EBiomeType::Tundra: BiomeName = TEXT("Tundra"); break;
					case EBiomeType::Rainforest: BiomeName = TEXT("Rainforest"); break;
					case EBiomeType::Savanna: BiomeName = TEXT("Savanna"); break;
					case EBiomeType::Mountain: BiomeName = TEXT("Mountain"); break;
					case EBiomeType::Snow: BiomeName = TEXT("Snow"); break;
					case EBiomeType::Swamp: BiomeName = TEXT("Swamp"); break;
					default: BiomeName = TEXT("Unknown"); break;
				}
				DrawDebugString(World, Location + FVector(0, 0, 100), BiomeName, nullptr, BiomeColor, Duration);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Drew biome debug visualization (%d biomes)"), BiomeData.Num());
}

void UHarmoniaWorldGeneratorEditorSubsystem::DrawRoadDebugVisualization(
	UWorld* World,
	const TArray<FRoadSegmentData>& RoadSegments,
	float Duration)
{
	if (!World || RoadSegments.Num() == 0)
	{
		return;
	}

	const FColor RoadColor = FColor::Orange;
	const float RoadThickness = 10.0f;

	for (const FRoadSegmentData& Segment : RoadSegments)
	{
		// Draw spline points
		for (int32 i = 0; i < Segment.SplinePoints.Num() - 1; ++i)
		{
			DrawDebugLine(World, Segment.SplinePoints[i], Segment.SplinePoints[i + 1], RoadColor, false, Duration, 0, RoadThickness);
		}

		// Draw endpoint markers
		if (Segment.SplinePoints.Num() > 0)
		{
			DrawDebugSphere(World, Segment.SplinePoints[0], 50.0f, 8, RoadColor, false, Duration);
			DrawDebugSphere(World, Segment.SplinePoints.Last(), 50.0f, 8, RoadColor, false, Duration);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Drew road debug visualization (%d segments)"), RoadSegments.Num());
}

void UHarmoniaWorldGeneratorEditorSubsystem::DrawRiverDebugVisualization(
	UWorld* World,
	const TArray<FRoadSegmentData>& RiverSegments,
	float Duration)
{
	if (!World || RiverSegments.Num() == 0)
	{
		return;
	}

	const FColor RiverColor = FColor::Cyan;
	const float RiverThickness = 8.0f;

	for (const FRoadSegmentData& Segment : RiverSegments)
	{
		// Draw spline points
		for (int32 i = 0; i < Segment.SplinePoints.Num() - 1; ++i)
		{
			DrawDebugLine(World, Segment.SplinePoints[i], Segment.SplinePoints[i + 1], RiverColor, false, Duration, 0, RiverThickness);

			// Draw flow direction arrows
			const FVector Direction = (Segment.SplinePoints[i + 1] - Segment.SplinePoints[i]).GetSafeNormal();
			const FVector MidPoint = (Segment.SplinePoints[i] + Segment.SplinePoints[i + 1]) * 0.5f;
			DrawDebugDirectionalArrow(World, MidPoint, MidPoint + Direction * 100.0f, 50.0f, RiverColor, false, Duration);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Drew river debug visualization (%d segments)"), RiverSegments.Num());
}

void UHarmoniaWorldGeneratorEditorSubsystem::DrawStructureDebugVisualization(
	UWorld* World,
	const TArray<FWorldObjectData>& Objects,
	float Duration)
{
	if (!World || Objects.Num() == 0)
	{
		return;
	}

	for (const FWorldObjectData& Object : Objects)
	{
		FColor ObjectColor;
		float BoxSize = 100.0f;

		switch (Object.ObjectType)
		{
			case EWorldObjectType::Structure:
				ObjectColor = FColor::White;
				BoxSize = 200.0f;
				break;
			case EWorldObjectType::POI:
				ObjectColor = FColor::Yellow;
				BoxSize = 250.0f;
				break;
			default:
				ObjectColor = FColor::Magenta;
				BoxSize = 100.0f;
				break;
		}

		// Draw structure box
		DrawDebugBox(World, Object.Location, FVector(BoxSize), Object.Rotation.Quaternion(), ObjectColor, false, Duration, 0, 5.0f);

		// Draw structure label
		FString ObjectName;
		switch (Object.ObjectType)
		{
			case EWorldObjectType::Structure: ObjectName = TEXT("Structure"); break;
			case EWorldObjectType::POI: ObjectName = TEXT("POI"); break;
			default: ObjectName = TEXT("Object"); break;
		}
		DrawDebugString(World, Object.Location + FVector(0, 0, 200), ObjectName, nullptr, ObjectColor, Duration);
	}

	UE_LOG(LogTemp, Log, TEXT("Drew structure debug visualization (%d objects)"), Objects.Num());
}

void UHarmoniaWorldGeneratorEditorSubsystem::DrawCaveDebugVisualization(
	UWorld* World,
	const TArray<FWorldObjectData>& CaveEntrances,
	float Duration)
{
	if (!World || CaveEntrances.Num() == 0)
	{
		return;
	}

	const FColor CaveColor = FColor(64, 32, 0); // Brown

	for (const FWorldObjectData& Entrance : CaveEntrances)
	{
		// Draw cave entrance sphere
		DrawDebugSphere(World, Entrance.Location, 200.0f, 16, CaveColor, false, Duration, 0, 10.0f);

		// Draw label
		DrawDebugString(World, Entrance.Location + FVector(0, 0, 300), TEXT("Cave Entrance"), nullptr, CaveColor, Duration);
	}

	UE_LOG(LogTemp, Log, TEXT("Drew cave debug visualization (%d entrances)"), CaveEntrances.Num());
}

void UHarmoniaWorldGeneratorEditorSubsystem::DrawResourceDebugVisualization(
	UWorld* World,
	const TArray<FOreVeinData>& OreVeins,
	const TArray<FWorldObjectData>& ResourceNodes,
	float Duration)
{
	if (!World)
	{
		return;
	}

	// Draw ore veins
	for (const FOreVeinData& Vein : OreVeins)
	{
		const FColor VeinColor = GetResourceDebugColor(Vein.ResourceType);

		// Draw vein sphere
		DrawDebugSphere(World, Vein.Location, Vein.Radius, 12, VeinColor, false, Duration, 0, 5.0f);

		// Draw vein label
		FString ResourceName;
		switch (Vein.ResourceType)
		{
			case EResourceType::IronOre: ResourceName = TEXT("Iron Vein"); break;
			case EResourceType::GoldOre: ResourceName = TEXT("Gold Vein"); break;
			case EResourceType::Coal: ResourceName = TEXT("Coal Vein"); break;
			case EResourceType::CopperOre: ResourceName = TEXT("Copper Vein"); break;
			case EResourceType::CrystalOre: ResourceName = TEXT("Crystal Vein"); break;
			case EResourceType::Gems: ResourceName = TEXT("Gem Vein"); break;
			default: ResourceName = TEXT("Ore Vein"); break;
		}
		DrawDebugString(World, Vein.Location + FVector(0, 0, 100), ResourceName, nullptr, VeinColor, Duration);
	}

	// Draw individual resource nodes
	for (const FWorldObjectData& Node : ResourceNodes)
	{
		const FColor NodeColor = FColor::Green;
		DrawDebugPoint(World, Node.Location, 20.0f, NodeColor, false, Duration);
	}

	UE_LOG(LogTemp, Log, TEXT("Drew resource debug visualization (%d veins, %d nodes)"), OreVeins.Num(), ResourceNodes.Num());
}

void UHarmoniaWorldGeneratorEditorSubsystem::DrawPOIDebugVisualization(
	UWorld* World,
	const TArray<FWorldObjectData>& POIs,
	float Duration)
{
	if (!World || POIs.Num() == 0)
	{
		return;
	}

	for (const FWorldObjectData& POI : POIs)
	{
		// Use POI type to determine color
		const FColor POIColor = GetPOIDebugColor(POI.POIType);

		// Draw POI marker
		DrawDebugCylinder(World, POI.Location, POI.Location + FVector(0, 0, 500), 100.0f, 12, POIColor, false, Duration, 0, 10.0f);
		DrawDebugSphere(World, POI.Location + FVector(0, 0, 500), 150.0f, 12, POIColor, false, Duration);

		// Draw label
		FString POIName;
		switch (POI.POIType)
		{
			case EPOIType::Dungeon: POIName = TEXT("Dungeon"); break;
			case EPOIType::Treasure: POIName = TEXT("Treasure"); break;
			case EPOIType::QuestLocation: POIName = TEXT("Quest Location"); break;
			case EPOIType::Boss: POIName = TEXT("Boss Arena"); break;
			case EPOIType::Camp: POIName = TEXT("Camp"); break;
			case EPOIType::Ruins: POIName = TEXT("Ruins"); break;
			default: POIName = TEXT("POI"); break;
		}
		DrawDebugString(World, POI.Location + FVector(0, 0, 700), POIName, nullptr, POIColor, Duration);
	}

	UE_LOG(LogTemp, Log, TEXT("Drew POI debug visualization (%d POIs)"), POIs.Num());
}

void UHarmoniaWorldGeneratorEditorSubsystem::DrawHeightmapDebugVisualization(
	UWorld* World,
	const TArray<int32>& HeightData,
	const FWorldGeneratorConfig& Config,
	float Duration)
{
	if (!World || HeightData.Num() == 0)
	{
		return;
	}

	const int32 MapSize = Config.SizeX;
	const float TileSize = 100.0f; // Default landscape quad size

	// Draw heightmap grid (sample every 20 tiles for performance)
	for (int32 Y = 0; Y < MapSize; Y += 20)
	{
		for (int32 X = 0; X < MapSize; X += 20)
		{
			const int32 Index = Y * MapSize + X;
			if (Index >= HeightData.Num())
			{
				continue;
			}

			const float Height = HeightData[Index];
			const FVector Location(X * TileSize, Y * TileSize, Height);

			// Color based on height
			FColor HeightColor;
			if (Height < 0)
			{
				HeightColor = FColor::Blue; // Water
			}
			else if (Height < 100)
			{
				HeightColor = FColor::Green; // Low land
			}
			else if (Height < 300)
			{
				HeightColor = FColor::Yellow; // Hills
			}
			else
			{
				HeightColor = FColor::White; // Mountains
			}

			DrawDebugPoint(World, Location, 10.0f, HeightColor, false, Duration);

			// Draw connections to neighbors
			if (X < MapSize - 20)
			{
				const int32 NextIndex = Y * MapSize + (X + 20);
				if (NextIndex < HeightData.Num())
				{
					const float NextHeight = HeightData[NextIndex];
					const FVector NextLocation((X + 20) * TileSize, Y * TileSize, NextHeight);
					DrawDebugLine(World, Location, NextLocation, FColor::Silver, false, Duration);
				}
			}
			if (Y < MapSize - 20)
			{
				const int32 NextIndex = (Y + 20) * MapSize + X;
				if (NextIndex < HeightData.Num())
				{
					const float NextHeight = HeightData[NextIndex];
					const FVector NextLocation(X * TileSize, (Y + 20) * TileSize, NextHeight);
					DrawDebugLine(World, Location, NextLocation, FColor::Silver, false, Duration);
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Drew heightmap debug visualization (%dx%d)"), MapSize, MapSize);
}

void UHarmoniaWorldGeneratorEditorSubsystem::ClearDebugVisualizations(UWorld* World)
{
	if (!World)
	{
		return;
	}

	FlushPersistentDebugLines(World);
	FlushDebugStrings(World);

	UE_LOG(LogTemp, Log, TEXT("Cleared all debug visualizations"));
}

// ========================================
// Editor Utility Functions
// ========================================

void UHarmoniaWorldGeneratorEditorSubsystem::EditorQuickPreview(
	UWorld* World,
	const FWorldGeneratorConfig& Config,
	bool bShowBiomes,
	bool bShowRoads,
	bool bShowRivers,
	bool bShowStructures,
	bool bShowResources)
{
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("EditorQuickPreview: Invalid world"));
		return;
	}

	UHarmoniaWorldGeneratorSubsystem* GeneratorSubsystem = GetWorldGeneratorSubsystem(World);
	if (!GeneratorSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("EditorQuickPreview: Could not find WorldGeneratorSubsystem"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Starting editor quick preview..."));

	// Clear existing visualizations
	ClearDebugVisualizations(World);

	// Generate world data
	TArray<int32> HeightData;
	TMap<EWorldObjectType, TSoftClassPtr<AActor>> DummyActorMap;

	GeneratorSubsystem->GenerateHeightmapOnly(Config, HeightData);

	// Generate and visualize biomes
	if (bShowBiomes)
	{
		TArray<FBiomeData> BiomeData;
		GeneratorSubsystem->GenerateBiomeMap(Config, HeightData, BiomeData);
		DrawBiomeDebugVisualization(World, BiomeData, Config, 30.0f);
	}

	// Generate and visualize roads
	if (bShowRoads)
	{
		TArray<FRoadSegmentData> RoadSegments;
		GeneratorSubsystem->GenerateRoads(Config, HeightData, RoadSegments);
		DrawRoadDebugVisualization(World, RoadSegments, 30.0f);
	}

	// Generate and visualize rivers
	if (bShowRivers)
	{
		TArray<FBiomeData> BiomeData;
		GeneratorSubsystem->GenerateBiomeMap(Config, HeightData, BiomeData);

		TArray<FRoadSegmentData> RiverSegments;
		GeneratorSubsystem->GenerateRivers(Config, HeightData, BiomeData, RiverSegments);
		DrawRiverDebugVisualization(World, RiverSegments, 30.0f);
	}

	// Generate and visualize structures
	if (bShowStructures)
	{
		TArray<FWorldObjectData> StructureObjects;
		GeneratorSubsystem->GenerateStructureGroups(Config, HeightData, DummyActorMap, StructureObjects);
		DrawStructureDebugVisualization(World, StructureObjects, 30.0f);
	}

	// Generate and visualize resources
	if (bShowResources)
	{
		TArray<FBiomeData> BiomeData;
		GeneratorSubsystem->GenerateBiomeMap(Config, HeightData, BiomeData);

		TArray<FOreVeinData> OreVeins;
		TArray<FWorldObjectData> ResourceNodes;
		GeneratorSubsystem->GenerateResourceDistribution(Config, HeightData, BiomeData, OreVeins, ResourceNodes);
		DrawResourceDebugVisualization(World, OreVeins, ResourceNodes, 30.0f);
	}

	UE_LOG(LogTemp, Log, TEXT("Editor quick preview complete!"));
}

bool UHarmoniaWorldGeneratorEditorSubsystem::ExportHeightmapToPNG(
	const TArray<int32>& HeightData,
	const FWorldGeneratorConfig& Config,
	const FString& FilePath)
{
	if (HeightData.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("ExportHeightmapToPNG: Empty height data"));
		return false;
	}

	const int32 MapSize = Config.SizeX;
	TArray<FColor> ImageData;
	ImageData.SetNum(MapSize * MapSize);

	// Find min/max for normalization
	int32 MinHeight = TNumericLimits<int32>::Max();
	int32 MaxHeight = TNumericLimits<int32>::Min();
	for (int32 Height : HeightData)
	{
		MinHeight = FMath::Min(MinHeight, Height);
		MaxHeight = FMath::Max(MaxHeight, Height);
	}

	const int32 HeightRange = MaxHeight - MinHeight;

	// Convert heightmap to grayscale image
	for (int32 Y = 0; Y < MapSize; ++Y)
	{
		for (int32 X = 0; X < MapSize; ++X)
		{
			const int32 Index = Y * MapSize + X;
			if (Index < HeightData.Num())
			{
				const int32 Height = HeightData[Index];
				const uint8 Normalized = (HeightRange > 0) ?
					((Height - MinHeight) * 255 / HeightRange) : 128;
				ImageData[Index] = FColor(Normalized, Normalized, Normalized, 255);
			}
		}
	}

	// Save to PNG
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (ImageWrapper.IsValid() && ImageWrapper->SetRaw(ImageData.GetData(), ImageData.GetAllocatedSize(), MapSize, MapSize, ERGBFormat::BGRA, 8))
	{
		const TArray64<uint8>& CompressedData = ImageWrapper->GetCompressed();
		if (FFileHelper::SaveArrayToFile(CompressedData, *FilePath))
		{
			UE_LOG(LogTemp, Log, TEXT("Exported heightmap to: %s"), *FilePath);
			return true;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to export heightmap to: %s"), *FilePath);
	return false;
}

bool UHarmoniaWorldGeneratorEditorSubsystem::ExportBiomeMapToPNG(
	const TArray<FBiomeData>& BiomeData,
	const FWorldGeneratorConfig& Config,
	const FString& FilePath)
{
	if (BiomeData.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("ExportBiomeMapToPNG: Empty biome data"));
		return false;
	}

	const int32 MapSize = Config.SizeX;
	TArray<FColor> ImageData;
	ImageData.SetNum(MapSize * MapSize);

	// Convert biome map to colored image
	for (int32 Y = 0; Y < MapSize; ++Y)
	{
		for (int32 X = 0; X < MapSize; ++X)
		{
			const int32 Index = Y * MapSize + X;
			if (Index < BiomeData.Num())
			{
				ImageData[Index] = GetBiomeDebugColor(BiomeData[Index].BiomeType);
			}
		}
	}

	// Save to PNG
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (ImageWrapper.IsValid() && ImageWrapper->SetRaw(ImageData.GetData(), ImageData.GetAllocatedSize(), MapSize, MapSize, ERGBFormat::BGRA, 8))
	{
		const TArray64<uint8>& CompressedData = ImageWrapper->GetCompressed();
		if (FFileHelper::SaveArrayToFile(CompressedData, *FilePath))
		{
			UE_LOG(LogTemp, Log, TEXT("Exported biome map to: %s"), *FilePath);
			return true;
		}
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to export biome map to: %s"), *FilePath);
	return false;
}

// ========================================
// Color Helper Functions
// ========================================

FColor UHarmoniaWorldGeneratorEditorSubsystem::GetBiomeDebugColor(EBiomeType BiomeType)
{
	switch (BiomeType)
	{
		case EBiomeType::Ocean:          return FColor(0, 105, 148);      // Deep Blue
		case EBiomeType::Beach:          return FColor(238, 214, 175);    // Sandy
		case EBiomeType::Desert:         return FColor(237, 201, 175);    // Desert Tan
		case EBiomeType::Grassland:      return FColor(124, 252, 0);      // Lawn Green
		case EBiomeType::Forest:         return FColor(34, 139, 34);      // Forest Green
		case EBiomeType::Taiga:          return FColor(0, 100, 0);        // Dark Green
		case EBiomeType::Tundra:         return FColor(176, 224, 230);    // Powder Blue
		case EBiomeType::Rainforest:     return FColor(0, 128, 0);        // Green
		case EBiomeType::Savanna:        return FColor(189, 183, 107);    // Dark Khaki
		case EBiomeType::Mountain:       return FColor(139, 137, 137);    // Gray
		case EBiomeType::Snow:           return FColor(255, 250, 250);    // Snow White
		case EBiomeType::Swamp:          return FColor(85, 107, 47);      // Dark Olive Green
		default:                         return FColor::Magenta;           // Unknown
	}
}

FColor UHarmoniaWorldGeneratorEditorSubsystem::GetResourceDebugColor(EResourceType ResourceType)
{
	switch (ResourceType)
	{
		case EResourceType::IronOre:    return FColor(192, 192, 192);   // Silver
		case EResourceType::GoldOre:    return FColor(255, 215, 0);     // Gold
		case EResourceType::Coal:       return FColor(0, 0, 0);         // Black
		case EResourceType::CopperOre:  return FColor(184, 115, 51);    // Copper
		case EResourceType::CrystalOre: return FColor(230, 230, 250);   // Lavender
		case EResourceType::Gems:       return FColor(255, 0, 255);     // Magenta
		default:                        return FColor::White;            // Default
	}
}

FColor UHarmoniaWorldGeneratorEditorSubsystem::GetPOIDebugColor(EPOIType POIType)
{
	switch (POIType)
	{
		case EPOIType::Dungeon:      return FColor::Red;
		case EPOIType::Treasure:     return FColor(255, 215, 0);  // Gold
		case EPOIType::QuestLocation: return FColor::Cyan;
		case EPOIType::Boss:         return FColor(128, 0, 128);  // Purple
		case EPOIType::Camp:         return FColor::Yellow;
		case EPOIType::Ruins:        return FColor(139, 137, 137); // Gray
		default:                     return FColor::Magenta;
	}
}
