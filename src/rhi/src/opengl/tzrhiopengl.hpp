#pragma once

#include <loom/tzrhi.hpp>
#include "tzglcontext.hpp"

#include <memory>
#include <vector>

class TzRhiOpenGLResourceUpdateBatch;
class TzRhiOpenGLCommandBuffer;

// OpenGL 3.3 Core backend.
//
// Created by TzRhi::create(Backend::OpenGL).

class TzRhiOpenGL : public TzRhi
{
public:
    TzRhiOpenGL();
    ~TzRhiOpenGL() override;

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

    bool isYUpInNdc()         const override { return true; }
    bool isYUpInFramebuffer() const override { return true; }

private:
    std::unique_ptr<TzRhiOpenGLCommandBuffer>        m_commandBuffer;
    std::unique_ptr<TzRhiOpenGLResourceUpdateBatch>  m_updateBatch;
};
