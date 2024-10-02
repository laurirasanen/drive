#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <memory>
#include <vector>

#include <glm/vec2.hpp>

#include "../Math.h"
#include "../Renderer/Buffer.h"
#include "../Renderer/DataTypes.h"
#include "../Renderer/Renderer.h"
#include "Icosphere.h"
#include "src/Components/Camera.h"

namespace drive
{
const glm::vec3 LeafColor(0.0, 0.5, 0.0);
const float     LeafRadiusMin  = 1.0f;
const float     LeafRadiusMax  = 2.5f;
const float     LeafHeightBias = 1.5f;

struct Tree
{
    glm::vec3 position;
    glm::vec3 halfExtents;
    // glm::mat4x4               modelMatrix;
    std::shared_ptr<Renderer> renderer;

    // TODO use model matrix instead, all spheres are the same...
    std::shared_ptr<Buffer> leafVertexBuffer;
    std::shared_ptr<Buffer> leafIndexBuffer;

    Tree(std::shared_ptr<Renderer> rend, glm::vec3 pos)
    {
        position    = pos;
        renderer    = rend;
        halfExtents = glm::vec3(
            Math::RandomFloat(LeafRadiusMin, LeafRadiusMax),
            Math::RandomFloat(LeafRadiusMin, LeafRadiusMax),
            Math::RandomFloat(LeafRadiusMin, LeafRadiusMax) * LeafHeightBias
        );
        // modelMatrix = glm::identity<glm::mat4x4>();
        // modelMatrix = glm::translate(modelMatrix, position);
        // modelMatrix = glm::scale(modelMatrix, halfExtents);

        // if (leafVertexBuffer == nullptr || leafIndexBuffer == nullptr)
        {
            // auto icosphere = Icosphere();
            auto icosphere = Icosphere(position);

            std::vector<Vertex_P_N_C> vertices;
            vertices.reserve(icosphere.positions.size());
            for (unsigned int i = 0; i < icosphere.positions.size(); i++)
            {
                vertices.push_back({icosphere.positions[i], icosphere.normals[i], LeafColor});
            }

            renderer->CreateBuffer(
                leafVertexBuffer,
                VertexBuffer,
                vertices.data(),
                sizeof(Vertex_P_N_C),
                static_cast<uint32_t>(vertices.size())
            );
            renderer->CreateBuffer(
                leafIndexBuffer,
                IndexBuffer,
                icosphere.indices.data(),
                sizeof(Index),
                static_cast<uint32_t>(icosphere.indices.size())
            );
        }
    }

    bool IsInFrame(const std::shared_ptr<Camera> /*camera*/) const
    {
        // TODO check bounds in frustum
        return true;
    }

    void Render() const
    {
        // renderer->SetModelMatrix(&modelMatrix);
        renderer->BindPipeline(RenderPipeline::TERRAIN);
        renderer->DrawWithBuffers(leafVertexBuffer, leafIndexBuffer);
    }
};
}; // namespace drive
