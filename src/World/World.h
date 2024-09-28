#pragma once

#include <memory>
#include <mutex>

#include "Sky.h"
#include "Terrain.h"

namespace drive
{
class World
{
  public:
    World() = delete;
    World(std::shared_ptr<Renderer> renderer);
    ~World();

    World(const World&)            = delete;
    World(World&&)                 = delete;
    World& operator=(const World&) = delete;
    World& operator=(World&&)      = delete;

    void Frame();
    void Tick(std::shared_ptr<Camera> camera);
    void Render();

  private:
    std::mutex m_worldMutex;

    std::unique_ptr<Terrain> m_terrain;
    std::unique_ptr<Sky>     m_sky;
};
} // namespace drive
