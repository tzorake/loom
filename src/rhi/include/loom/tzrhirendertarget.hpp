#pragma once

// Abstract render target — common base for swapchain render targets and
// offscreen render targets.
//
// TzRhiSwapChain inherits from both TzRhiResource and TzRhiRenderTarget.
// Offscreen render targets (not yet implemented) would also derive from this.

class TzRhiRenderTarget
{
public:
    virtual ~TzRhiRenderTarget();

    virtual int pixelWidth()  const = 0;
    virtual int pixelHeight() const = 0;
};
