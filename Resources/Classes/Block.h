#pragma once

#include <glm/glm.hpp>

// Keep AIR = 0 so memset in Chunk initializes to AIR
enum class BlockType : int {
    AIR = 0,
    DIRT,
    GRASS,
    STONE
};

struct Block {
    BlockType type;

    // Solid if it's not AIR
    bool isSolid() const {
        return type != BlockType::AIR;
    }

    // Basic color per block type
    glm::vec3 getColor() const;
};

