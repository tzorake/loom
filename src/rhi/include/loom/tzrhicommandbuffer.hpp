#pragma once

#include <cstdint>

class TzRhiRenderTarget;
class TzRhiRenderPassDescriptor;
class TzRhiResourceUpdateBatch;
class TzRhiGraphicsPipeline;
class TzRhiShaderResourceBindings;
class TzRhiBuffer;

// Command buffer — records GPU draw commands for one frame.
//
// Obtained each frame via TzRhi::currentFrameCommandBuffer().
// Not a TzRhiResource — no create()/destroy(). The backend owns and recycles
// the instance across frames.

class TzRhiCommandBuffer
{
public:
    enum IndexFormat { UInt16, UInt32 };

    // ── Clear value for a render pass ─────────────────────────────────────────
    struct ClearValue {
        float r{0.0f}, g{0.0f}, b{0.0f}, a{1.0f};
        float depth{1.0f};
        uint32_t stencil{0};
    };

    virtual ~TzRhiCommandBuffer();

    // ── Render pass ───────────────────────────────────────────────────────────

    // Begin a render pass.
    //   uploads — optional batch to execute before the first draw; pass nullptr
    //             if no uploads are needed this frame.
    virtual void beginPass(TzRhiRenderTarget *rt,
                           TzRhiRenderPassDescriptor *rpd,
                           const ClearValue &clearValue,
                           TzRhiResourceUpdateBatch *uploads = nullptr) = 0;

    // End the current render pass.
    //   uploads — optional batch to execute after the pass (for next-frame prep).
    virtual void endPass(TzRhiResourceUpdateBatch *uploads = nullptr) = 0;

    // ── Pipeline state ────────────────────────────────────────────────────────
    virtual void setGraphicsPipeline(TzRhiGraphicsPipeline *ps) = 0;

    // Pass nullptr to use the pipeline's own shader resource bindings.
    virtual void setShaderResources(TzRhiShaderResourceBindings *srb = nullptr) = 0;

    // ── Viewport / scissor ────────────────────────────────────────────────────
    virtual void setViewport(float x, float y, float w, float h,
                             float minDepth = 0.0f, float maxDepth = 1.0f) = 0;
    virtual void setScissor(int x, int y, int w, int h) = 0;

    // ── Vertex / index input ──────────────────────────────────────────────────
    //   startBinding — first vertex-buffer binding slot
    //   count        — number of vertex buffers
    //   bufs         — array of count TzRhiBuffer pointers
    //   offsets      — byte offsets into each buffer (may be nullptr for all-zero)
    //   indexBuf     — index buffer, or nullptr for non-indexed draws
    //   indexOffset  — byte offset into the index buffer
    //   indexFormat  — UInt16 or UInt32
    virtual void setVertexInput(int startBinding, int count,
                                const TzRhiBuffer *const *bufs,
                                const uint32_t *offsets,
                                TzRhiBuffer *indexBuf = nullptr,
                                uint32_t indexOffset = 0,
                                IndexFormat indexFormat = UInt16) = 0;

    // ── Draw calls ────────────────────────────────────────────────────────────
    virtual void draw(uint32_t vertexCount, uint32_t instanceCount = 1,
                      uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;

    virtual void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1,
                             uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                             uint32_t firstInstance = 0) = 0;

    // ── Debug markers (optional; no-op in release builds) ────────────────────
    virtual void debugMarkBegin(const char *name) = 0;
    virtual void debugMarkEnd() = 0;
};
