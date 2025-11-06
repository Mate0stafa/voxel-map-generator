#pragma once
#include "Chunk.h"
#include "PerlinNoise.h"
#include <memory>

class WorldGeneration {
public:
    static void initialize(unsigned int seed = 0);
    static void generateChunk(Chunk& chunk);
    
    // Control the animation phase/time for dynamic noise
    static void setAnimationTime(float t);

private:
    static std::unique_ptr<PerlinNoise> perlin;
    static unsigned int currentSeed;
    static float animationTime;

    static constexpr float TERRAIN_SCALE = 0.01f;
    static constexpr float CAVE_SCALE = 0.05f;
    static constexpr int TERRAIN_OCTAVES = 4;
    static constexpr int CAVE_OCTAVES = 3;
    
    static float getHeight(float x, float z);
    static float getCaveDensity(float x, float y, float z);
};