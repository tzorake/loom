#pragma once

#include "tzrhiresource.hpp"

// Render pass compatibility token.
//
// Obtained via TzRhi::newCompatibleRenderPassDescriptor(renderTarget).
// Must be set on both TzRhiSwapChain and TzRhiGraphicsPipeline before use.
// Two descriptors are compatible if they describe the same attachment formats
// and sample counts — the pipeline can then be used with that render target.

class TzRhiRenderPassDescriptor : public TzRhiResource
{
public:
    ~TzRhiRenderPassDescriptor() override;

    TzRhiResource::Type resourceType() const override
    {
        return TzRhiResource::RenderPassDescriptor;
    }

    // Returns true if this descriptor is compatible with other — i.e. a
    // pipeline created with one can be used with a render target that uses the
    // other.
    virtual bool isCompatible(const TzRhiRenderPassDescriptor *other) const = 0;

protected:
    explicit TzRhiRenderPassDescriptor(TzRhi *rhi);
};
