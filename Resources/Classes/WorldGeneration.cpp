#include "WorldGeneration.h"
#include <cmath>

std::unique_ptr<PerlinNoise> WorldGeneration::perlin = nullptr;
unsigned int WorldGeneration::currentSeed = 0;
float WorldGeneration::animationTime = 0.0f;

void WorldGeneration::initialize(unsigned int seed) {
    currentSeed = seed;
    perlin = std::make_unique<PerlinNoise>(seed);
}

void WorldGeneration::setAnimationTime(float t) {
    animationTime = t;
}

float WorldGeneration::getHeight(float x, float z) {
    // Compress terrain into chunk vertical range leaving top layers as air
    const float maxTerrain = static_cast<float>(CHUNK_HEIGHT - 4);
    // Animate via time on Y channel for variety
    float n = perlin->octaveNoise(x * TERRAIN_SCALE, animationTime * 0.2f, z * TERRAIN_SCALE, TERRAIN_OCTAVES);
    // Clamp to [0,1] in case the noise impl overshoots slightly
    if (n < 0.0f) n = 0.0f;
    if (n > 1.0f) n = 1.0f;
    return n * maxTerrain;
}

float WorldGeneration::getCaveDensity(float x, float y, float z) {
    return perlin->octaveNoise(x * CAVE_SCALE, y * CAVE_SCALE + animationTime * 0.3f, z * CAVE_SCALE, CAVE_OCTAVES);
}

void WorldGeneration::generateChunk(Chunk& chunk) {
    if (!perlin) initialize();
    
    const glm::ivec2 chunkPos = chunk.position;
    const int worldX = chunkPos.x * CHUNK_SIZE;
    const int worldZ = chunkPos.y * CHUNK_SIZE;
    
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            const float globalX = worldX + x;
            const float globalZ = worldZ + z;
            float height = getHeight(globalX, globalZ);
            int surface = static_cast<int>(height);

            for (int y = 0; y < CHUNK_HEIGHT; y++) {
                Block block;

                if (y == 0) {
                    block.type = BlockType::STONE; // Bedrock
                } else if (y < surface - 5) {
                    // Underground stone with occasional caves
                    block.type = (getCaveDensity(globalX, static_cast<float>(y), globalZ) > 0.45f)
                                 ? BlockType::AIR : BlockType::STONE;
                } else if (y < surface - 1) {
                    // Dirt layer near the surface
                    block.type = BlockType::DIRT;
                } else if (y == surface - 1 && surface > 0) {
                    // Topmost surface block
                    block.type = BlockType::GRASS;
                } else {
                    // Air above surface and in deliberately empty top layers
                    block.type = BlockType::AIR;
                }
                
                chunk.setBlock(x, y, z, block);
            }
        }
    }
}