#include "World.h"
#include "WorldGeneration.h"
#include <algorithm>
#include <cmath>

World::World() : renderDistance(8) {
    // Initialize with empty world
}

void World::update(const glm::vec3& playerPos) {
    int chunkX = static_cast<int>(std::floor(playerPos.x / static_cast<float>(CHUNK_SIZE)));
    int chunkZ = static_cast<int>(std::floor(playerPos.z / static_cast<float>(CHUNK_SIZE)));

    for (int x = chunkX - renderDistance; x <= chunkX + renderDistance; x++) {
        for (int z = chunkZ - renderDistance; z <= chunkZ + renderDistance; z++) {
            if (chunks.find(getChunkKey(x, z)) == chunks.end()) {
                loadChunk(x, z);
            }
        }
    }
    unloadDistantChunks(playerPos);
}

void World::loadChunk(int x, int z) {
    auto chunk = std::make_unique<Chunk>(glm::ivec2(x, z));
    WorldGeneration::generateChunk(*chunk);
    // Insert first so neighbors can see it
    chunks[getChunkKey(x, z)] = std::move(chunk);

    // Generate mesh with world-aware neighbor checks for this chunk
    chunks[getChunkKey(x, z)]->generateMeshWithWorld(*this);

    // Refresh neighbor meshes so shared borders get culled properly
    const int nx[4] = { x-1, x+1, x,   x   };
    const int nz[4] = { z,   z,   z-1, z+1 };
    for (int i = 0; i < 4; ++i) {
        int64_t nkey = getChunkKey(nx[i], nz[i]);
        auto it = chunks.find(nkey);
        if (it != chunks.end()) {
            it->second->generateMeshWithWorld(*this);
        }
    }
}

void World::unloadDistantChunks(const glm::vec3& playerPos) {
    int centerX = static_cast<int>(std::floor(playerPos.x / static_cast<float>(CHUNK_SIZE)));
    int centerZ = static_cast<int>(std::floor(playerPos.z / static_cast<float>(CHUNK_SIZE)));

    auto it = chunks.begin();
    while (it != chunks.end()) {
        int64_t key = it->first;
        int x = static_cast<int>(key >> 32);
        int z = static_cast<int>(key & 0xFFFFFFFF);

        if (std::abs(x - centerX) > renderDistance || std::abs(z - centerZ) > renderDistance) {
            it = chunks.erase(it);
        } else {
            ++it;
        }
    }
}

void World::render() const {
    for (const auto& [key, chunk] : chunks) {
        chunk->render();
    }
}

int64_t World::getChunkKey(int x, int z) const {
    return ((int64_t)x << 32) | (int64_t)z;
}

Block World::getBlockGlobal(int gx, int gy, int gz) const {
    if (gy < 0 || gy >= CHUNK_HEIGHT) return Block{BlockType::AIR};

    auto floorDiv = [](int a, int b) -> int {
        // floor division for negatives
        int q = a / b;
        int r = a % b;
        if ((r != 0) && ((r > 0) != (b > 0)) && (a < 0)) --q;
        return q;
    };

    int cx = floorDiv(gx, CHUNK_SIZE);
    int cz = floorDiv(gz, CHUNK_SIZE);

    int lx = gx - cx * CHUNK_SIZE;
    int lz = gz - cz * CHUNK_SIZE;

    int64_t key = getChunkKey(cx, cz);
    auto it = chunks.find(key);
    if (it == chunks.end()) {
        return Block{BlockType::AIR};
    }
    return it->second->getBlock(lx, gy, lz);
}

void World::regenerateAllChunks() {
    for (auto& kv : chunks) {
        auto& chunk = kv.second;
        WorldGeneration::generateChunk(*chunk);
    }
    // After content changes, rebuild meshes with neighbor awareness
    for (auto& kv : chunks) {
        kv.second->generateMeshWithWorld(*this);
    }
}