#include "Chunk.h"
#include "World.h"
#include <Lib/Glad/include/glad/glad.h>
#include <cstring>

Chunk::Chunk(glm::ivec2 position) : position(position) {
    // Initialize all blocks to air
    memset(blocks, 0, sizeof(blocks));

    // Generate OpenGL buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

Block Chunk::getBlock(int x, int y, int z) const {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_HEIGHT || z < 0 || z >= CHUNK_SIZE)
        return Block{BlockType::AIR};
    return blocks[x][y][z];
}

void Chunk::setBlock(int x, int y, int z, Block block) {
    if (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_HEIGHT && z >= 0 && z < CHUNK_SIZE) {
        blocks[x][y][z] = block;
        needsMeshUpdate = true;
    }
}

void Chunk::generateMeshWithWorld(const World& world) {
    meshVertices.clear();

    const int baseX = position.x * CHUNK_SIZE;
    const int baseZ = position.y * CHUNK_SIZE;

    auto isSolidGlobal = [&](int gx, int gy, int gz) -> bool {
        if (gy < 0 || gy >= CHUNK_HEIGHT) return false; // outside vertical bounds => air
        return world.getBlockGlobal(gx, gy, gz).isSolid();
    };

    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            for (int x = 0; x < CHUNK_SIZE; x++) {
                Block current = getBlock(x, y, z);
                if (!current.isSolid()) continue;

                const int gx = baseX + x;
                const int gz = baseZ + z;

                if (!isSolidGlobal(gx-1, y, gz))
                    addFace({x, y, z}, {-1, 0, 0}, current.type);
                if (!isSolidGlobal(gx+1, y, gz))
                    addFace({x, y, z}, {1, 0, 0}, current.type);
                if (!isSolidGlobal(gx, y-1, gz))
                    addFace({x, y, z}, {0, -1, 0}, current.type);
                if (!isSolidGlobal(gx, y+1, gz))
                    addFace({x, y, z}, {0, 1, 0}, current.type);
                if (!isSolidGlobal(gx, y, gz-1))
                    addFace({x, y, z}, {0, 0, -1}, current.type);
                if (!isSolidGlobal(gx, y, gz+1))
                    addFace({x, y, z}, {0, 0, 1}, current.type);
            }
        }
    }
    
    uploadMeshToGPU();
}

void Chunk::uploadMeshToGPU() {
    // Upload mesh to GPU
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, meshVertices.size() * sizeof(float), meshVertices.data(), GL_STATIC_DRAW);
    
    // Vertex attributes (position + normal + color)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    vertexCount = meshVertices.size() / 9;
    needsMeshUpdate = false;
}

void Chunk::addFace(const glm::vec3& position, const glm::vec3& normal, BlockType type) {
    glm::vec3 color = Block{type}.getColor();
    
    // Small offset to prevent z-fighting at chunk boundaries
    const float epsilon = 0.001f;
    const float offsetX = normal.x * epsilon;
    const float offsetY = normal.y * epsilon;
    const float offsetZ = normal.z * epsilon;

    const float x = position.x + offsetX;
    const float y = position.y + offsetY;
    const float z = position.z + offsetZ;

    auto pushVertex = [&](float px, float py, float pz) {
        meshVertices.push_back(px);
        meshVertices.push_back(py);
        meshVertices.push_back(pz);
        meshVertices.push_back(normal.x);
        meshVertices.push_back(normal.y);
        meshVertices.push_back(normal.z);
        meshVertices.push_back(color.r);
        meshVertices.push_back(color.g);
        meshVertices.push_back(color.b);
    };

    // Build two triangles (A,B,C) and (A,C,D) with CCW winding facing the normal
    if (normal.x == 1.0f) {
        // Right face (x+1)
        // A: (x+1,y,  z+1), B: (x+1,y+1,z+1), C: (x+1,y+1,z), D: (x+1,y,  z)
        pushVertex(x+1, y,   z+1);
        pushVertex(x+1, y+1, z+1);
        pushVertex(x+1, y+1, z  );
        pushVertex(x+1, y,   z+1);
        pushVertex(x+1, y+1, z  );
        pushVertex(x+1, y,   z  );
    } else if (normal.x == -1.0f) {
        // Left face (x)
        // A: (x,y,  z), B: (x,y+1,z), C: (x,y+1,z+1), D: (x,y,  z+1)
        pushVertex(x, y,   z  );
        pushVertex(x, y+1, z  );
        pushVertex(x, y+1, z+1);
        pushVertex(x, y,   z  );
        pushVertex(x, y+1, z+1);
        pushVertex(x, y,   z+1);
    } else if (normal.y == 1.0f) {
        // Top face (y+1)
        // A: (x,  y+1,z+1), B: (x+1,y+1,z+1), C: (x+1,y+1,z), D: (x,  y+1,z)
        pushVertex(x,   y+1, z+1);
        pushVertex(x+1, y+1, z+1);
        pushVertex(x+1, y+1, z  );
        pushVertex(x,   y+1, z+1);
        pushVertex(x+1, y+1, z  );
        pushVertex(x,   y+1, z  );
    } else if (normal.y == -1.0f) {
        // Bottom face (y)
        // A: (x,  y,z), B: (x+1,y,z), C: (x+1,y,z+1), D: (x,  y,z+1)
        pushVertex(x,   y, z  );
        pushVertex(x+1, y, z  );
        pushVertex(x+1, y, z+1);
        pushVertex(x,   y, z  );
        pushVertex(x+1, y, z+1);
        pushVertex(x,   y, z+1);
    } else if (normal.z == 1.0f) {
        // Front face (z+1)
        // A: (x,  y,  z+1), B: (x+1,y,  z+1), C: (x+1,y+1,z+1), D: (x,  y+1,z+1)
        pushVertex(x,   y,   z+1);
        pushVertex(x+1, y,   z+1);
        pushVertex(x+1, y+1, z+1);
        pushVertex(x,   y,   z+1);
        pushVertex(x+1, y+1, z+1);
        pushVertex(x,   y+1, z+1);
    } else if (normal.z == -1.0f) {
        // Back face (z)
        // A: (x+1,y,  z), B: (x+1,y+1,z), C: (x,  y+1,z), D: (x,  y,  z)
        pushVertex(x+1, y,   z);
        pushVertex(x+1, y+1, z);
        pushVertex(x,   y+1, z);
        pushVertex(x+1, y,   z);
        pushVertex(x,   y+1, z);
        pushVertex(x,   y,   z);
    }
}

void Chunk::render() const {
    if (vertexCount == 0) return;
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}