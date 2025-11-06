#pragma once
#include "Chunk.h"
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

class World {
public:
    World();

    void update(const glm::vec3& playerPos);
    void render() const;

    // Regenerate all currently loaded chunks (re-run noise and rebuild meshes)
    void regenerateAllChunks();

    // Get a block at global world coordinates (gx, gy, gz); returns AIR if missing
    Block getBlockGlobal(int gx, int gy, int gz) const;

private:
    std::unordered_map<int64_t, std::unique_ptr<Chunk>> chunks;
    int renderDistance;

    int64_t getChunkKey(int x, int z) const;
    
    void loadChunk(int x, int z);
    void unloadDistantChunks(const glm::vec3& playerPos);
};