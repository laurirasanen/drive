#pragma once

#include <memory>
#include <vector>

#include <glm/vec2.hpp>

#include "../Renderer/Buffer.h"
#include "../Renderer/DataTypes.h"
#include "Tree.h"

namespace drive
{
#define CHUNK_SIZE 64

struct Chunk
{
    glm::ivec2 position;
    glm::vec2  worldPosition;
    glm::vec2  worldCenter;

    std::vector<Vertex_P_N_C> vertices;
    std::vector<Index>        indices;

    std::shared_ptr<Buffer> vertexBuffer;
    std::shared_ptr<Buffer> indexBuffer;

    std::vector<Tree> trees;

    Chunk(glm::ivec2 pos)
    {
        position      = pos;
        worldPosition = ChunkToWorld(pos);
        worldCenter   = worldPosition + glm::vec2(0.5f * CHUNK_SIZE);
    }

    static constexpr glm::vec2 ChunkToWorld(glm::ivec2 pos)
    {
        return pos * CHUNK_SIZE;
    }

    static constexpr glm::ivec2 WorldToChunk(glm::vec2 pos)
    {
        return glm::ivec2(pos.x / CHUNK_SIZE, pos.y / CHUNK_SIZE);
    }

    bool IsInFrame(const std::shared_ptr<Camera> /*camera*/) const
    {
        // TODO check bounds in frustum
        return true;
    }
};
}; // namespace drive
