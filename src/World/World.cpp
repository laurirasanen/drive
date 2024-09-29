#include "World.h"
#include "../Log.h"
#include "src/Renderer/Renderer.h"

namespace drive
{

World::World(std::shared_ptr<Renderer> renderer) : m_renderer(renderer)
{
    LOG_DEBUG("Creating World");
    m_terrain = std::make_unique<Terrain>(renderer);
    m_sky     = std::make_unique<Sky>(renderer);

    // Test icosphere
    auto                      testSphere = Icosphere(glm::vec3(0, 0, 105), 5.0f, 0);
    std::vector<Vertex_P_N_C> vertices;
    vertices.reserve(testSphere.positions.size());
    for (unsigned int i = 0; i < testSphere.positions.size(); i++)
    {
        vertices.push_back(
            {testSphere.positions[i], testSphere.normals[i], glm::vec3(0.5, 0.5, 0.5)}
        );
    }
    renderer->CreateBuffer(
        m_testSphereVertexBuffer,
        VertexBuffer,
        vertices.data(),
        sizeof(Vertex_P),
        static_cast<uint32_t>(vertices.size())
    );
    renderer->CreateBuffer(
        m_testSphereIndexBuffer,
        IndexBuffer,
        testSphere.indices.data(),
        sizeof(Index),
        static_cast<uint32_t>(testSphere.indices.size())
    );

    // test plane
    std::vector<Vertex_P_C> planeVertices = {
        {{-0.5, -0.5, 100.0}, {1.0f, 0.0f, -0.1f}},
        { {0.5, -0.5, 100.0}, {0.0f, 1.0f, -0.1f}},
        {  {0.5, 0.5, 100.0},  {0.0f, 0.0f, 0.9f}},
        { {-0.5, 0.5, 100.0},  {1.0f, 1.0f, 0.9f}},
    };
    std::vector<Index> planeIndices = {0, 1, 2, 2, 3, 0};
    renderer->CreateBuffer(
        m_testPlaneVertexBuffer,
        VertexBuffer,
        planeVertices.data(),
        sizeof(Vertex_P_C),
        static_cast<uint32_t>(planeVertices.size())
    );
    renderer->CreateBuffer(
        m_testPlaneIndexBuffer,
        IndexBuffer,
        planeIndices.data(),
        sizeof(Index),
        static_cast<uint32_t>(planeIndices.size())
    );
}

World::~World()
{
    LOG_DEBUG("Destroying World");
}

void World::Frame()
{
}

void World::Tick(std::shared_ptr<Camera> camera)
{
    {
        std::scoped_lock worldLock {m_worldMutex};
        m_terrain->SetObserverPosition(camera->transform.position);
    }
}

void World::Render()
{
    m_terrain->Render();

    m_renderer->BindPipeline(RenderPipeline::TERRAIN);
    m_renderer->DrawWithBuffers(m_testSphereVertexBuffer, m_testSphereIndexBuffer);

    m_renderer->BindPipeline(RenderPipeline::TEST);
    m_renderer->DrawWithBuffers(m_testPlaneVertexBuffer, m_testPlaneIndexBuffer);

    m_sky->Render();
}
} // namespace drive
