#include "World.h"
#include "../Log.h"

namespace drive
{

World::World(std::shared_ptr<Renderer> renderer)
{
    LOG_DEBUG("Creating World");
    m_terrain = std::make_unique<Terrain>(renderer);
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
}
} // namespace drive
