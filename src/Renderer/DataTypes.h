#pragma once

#include <memory>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "../Components/Camera.h"

namespace drive
{
struct SimpleVertex
{
    glm::vec3 position;
    glm::vec3 color;
};

struct TerrainVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

// For fullscreen triangle
struct EmptyVertex
{
};

struct Index
{
    uint32_t index;

    Index()
    {
        index = 0;
    }

    Index(const uint32_t i)
    {
        index = i;
    }
};

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 eye;

    UniformBufferObject(const std::shared_ptr<Camera> cam)
    {
        model = glm::identity<glm::mat4>();
        view  = cam->view;
        proj  = cam->proj;
        eye   = cam->transform.position;
    }
};

} // namespace drive
