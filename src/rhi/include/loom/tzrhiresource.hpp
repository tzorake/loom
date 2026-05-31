#pragma once

class TzRhi;

// Abstract base class for all GPU resources created by TzRhi.
//
// Resources are owned by the TzRhi that created them. The caller is responsible
// for calling destroy() before deleting the TzRhi instance. No ref-counting.

class TzRhiResource
{
public:
    enum Type {
        Buffer,
        Texture,
        Sampler,
        RenderBuffer,
        RenderPassDescriptor,
        SwapChain,
        ShaderResourceBindings,
        GraphicsPipeline
    };

    virtual ~TzRhiResource();

    virtual Type resourceType() const = 0;
    virtual void destroy() = 0;

    TzRhi *rhi() const { return m_rhi; }

protected:
    explicit TzRhiResource(TzRhi *rhi);

    TzRhi *m_rhi;
};
