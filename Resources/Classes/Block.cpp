//
// Simplified Block implementation matching Chunk usage
//
#include "Block.h"

glm::vec3 Block::getColor() const {
    switch (type) {
        case BlockType::DIRT:  return glm::vec3(0.545f, 0.271f, 0.075f);
        case BlockType::GRASS: return glm::vec3(0.200f, 0.800f, 0.200f);
        case BlockType::STONE: return glm::vec3(0.600f, 0.600f, 0.600f);
        case BlockType::AIR:
        default:               return glm::vec3(0.0f, 0.0f, 0.0f);
    }
}
