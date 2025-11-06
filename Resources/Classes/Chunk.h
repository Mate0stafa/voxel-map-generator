#pragma once

#include "Block.h"
#include <glm/glm.hpp>
#include <vector>

class World;

constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_HEIGHT = 16;

class Chunk {
public:
    Chunk(glm::ivec2 position);

    Block getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, Block block);

    void generateMesh();
    void render() const;

    // Generate mesh using world-aware neighbor checks (across chunk borders)
    void generateMeshWithWorld(const World& world);
    
    bool needsMeshUpdate = true;
    glm::ivec2 position;
    
private:
    Block blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE];
    unsigned int VAO, VBO;
    std::vector<float> meshVertices;
    size_t vertexCount = 0;
    
    void addFace(const glm::vec3& position, const glm::vec3& normal, BlockType type);
};
