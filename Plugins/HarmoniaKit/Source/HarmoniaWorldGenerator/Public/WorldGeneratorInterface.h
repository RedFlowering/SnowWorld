// Copyright 2025 Snow Game Studio.

/**
 * @file WorldGeneratorInterface.h
 * @brief World Generator Interface Definition
 * 
 * Defines the interface for various world generation strategies.
 * Implement this interface to create custom world generators.
 */

#pragma once

#include "WorldGeneratorTypes.h"

/**
 * @class IWorldGenerator
 * @brief Abstract interface for world generation strategies
 * 
 * Use this interface to implement different world generation algorithms.
 * Examples: PerlinWorldGenerator, VoronoiWorldGenerator, etc.
 */
class IWorldGenerator
{
public:
    virtual ~IWorldGenerator() = default;

    /**
     * @brief Generate world objects based on configuration
     * @param Config World generation parameters
     * @param OutObjects Generated world objects (output)
     * @param ActorClassMap Mapping from object types to actor classes
     */
    virtual void GenerateWorldObjects(
        const FWorldGeneratorConfig& Config,
        TArray<FWorldObjectData>& OutObjects,
        TMap<EWorldObjectType, TSoftClassPtr<AActor>> ActorClassMap) = 0;
};