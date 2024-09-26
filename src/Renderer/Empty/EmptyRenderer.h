#include "../Renderer.h"

namespace drive
{
class EmptyRenderer : public Renderer
{
  public:
    EmptyRenderer() {};
    ~EmptyRenderer() {};

    EmptyRenderer(const EmptyRenderer&)            = delete;
    EmptyRenderer(EmptyRenderer&&)                 = delete;
    EmptyRenderer& operator=(const EmptyRenderer&) = delete;
    EmptyRenderer& operator=(EmptyRenderer&&)      = delete;

    virtual void SetWindow(std::shared_ptr<Window> /*window*/) override
    {
    }

    virtual void ResetViewport() override
    {
    }

    virtual void SetViewport(Rect /*rect*/) override
    {
    }

    virtual void ClearViewport() override
    {
    }

    virtual void Resize() override
    {
    }

    virtual float GetAspect() override
    {
        return 0.0f;
    }

    virtual void Begin() override
    {
    }

    virtual void Submit() override
    {
    }

    virtual void Present() override
    {
    }

    virtual void UpdateUniforms(const std::shared_ptr<Camera> /*camera*/) override
    {
    }

    virtual void DrawTest() override
    {
    }

    virtual RendererType Type() const override
    {
        return RendererType::EMPTY;
    }

    virtual void WaitForIdle() override
    {
    }

    virtual void* GetCommandBuffer() override
    {
        return nullptr;
    }

    virtual void CreateBuffer(
        std::shared_ptr<Buffer>& /*buffer*/,
        BufferType /*bufferType*/,
        void* /*data*/,
        uint32_t /*elementSize*/,
        uint32_t /*elementCount*/
    ) override
    {
    }
};
} // namespace drive
