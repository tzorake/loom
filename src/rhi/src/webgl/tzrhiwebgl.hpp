#pragma once

#include <loom/tzrhi.hpp>
#include <memory>

class TzRhiWebGLResourceUpdateBatch;
class TzRhiWebGLCommandBuffer;

// WebGL 2.0 backend.
//
// The WebGL 2 context is obtained by JavaScript before WASM loads — no
// C++-side context creation is needed. All GL calls are dispatched through
// the WASM_IMPORT functions declared in tzwebglimports.hpp.

class TzRhiWebGL : public TzRhi
{
public:
    TzRhiWebGL();
    ~TzRhiWebGL() override;

    TzRhiBuffer               *newBuffer(TzRhiBuffer::Type, TzRhiBuffer::UsageFlags, uint32_t) override;
    TzRhiTexture              *newTexture(TzRhiTexture::Format, int, int, TzRhiTexture::Flags) override;
    TzRhiSampler              *newSampler(TzRhiSampler::Filter, TzRhiSampler::Filter,
                                          TzRhiSampler::Filter,
                                          TzRhiSampler::AddressMode, TzRhiSampler::AddressMode) override;
    TzRhiRenderBuffer         *newRenderBuffer(TzRhiRenderBuffer::Type, int, int, int) override;
    TzRhiRenderPassDescriptor *newCompatibleRenderPassDescriptor(TzRhiRenderTarget *) override;
    TzRhiSwapChain            *newSwapChain() override;
    TzRhiShaderResourceBindings *newShaderResourceBindings() override;
    TzRhiGraphicsPipeline     *newGraphicsPipeline() override;
    TzRhiResourceUpdateBatch  *nextResourceUpdateBatch() override;

    bool beginFrame(TzRhiSwapChain *) override;
    void endFrame(TzRhiSwapChain *) override;
    TzRhiCommandBuffer *currentFrameCommandBuffer() override;
    TzRhiRenderTarget  *currentFrameRenderTarget(TzRhiSwapChain *) override;

    // WebGL/canvas: NDC +Y up (matches desktop GL), but framebuffer +Y down
    // because the canvas is displayed with CSS default transform.
    bool isYUpInNdc()         const override { return true; }
    bool isYUpInFramebuffer() const override { return false; }

private:
    std::unique_ptr<TzRhiWebGLCommandBuffer>        m_commandBuffer;
    std::unique_ptr<TzRhiWebGLResourceUpdateBatch>  m_updateBatch;
};
