#pragma once

#include <memory>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#include "../Components/Camera.h"
#include "../Components/Rect.h"
#include "../Window/Window.h"

namespace drive
{
enum RendererType
{
    VULKAN,
};

class Renderer
{
  public:
    Renderer() = default;
    virtual ~Renderer() {};

    Renderer(const Renderer&)            = delete;
    Renderer(Renderer&&)                 = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&)      = delete;

    virtual void         SetWindow(std::shared_ptr<Window> window)            = 0;
    virtual void         ResetViewport()                                      = 0;
    virtual void         SetViewport(Rect rect)                               = 0;
    virtual void         ClearViewport()                                      = 0;
    virtual void         Resize()                                             = 0;
    virtual float        GetAspect()                                          = 0;
    virtual void         Begin()                                              = 0;
    virtual void         Submit()                                             = 0;
    virtual void         Present()                                            = 0;
    virtual void         UpdateUniforms(const std::shared_ptr<Camera> camera) = 0;
    virtual void         DrawTest()                                           = 0;
    virtual RendererType Type() const                                         = 0;
    virtual void         WaitForIdle()                                        = 0;
};
} // namespace drive
