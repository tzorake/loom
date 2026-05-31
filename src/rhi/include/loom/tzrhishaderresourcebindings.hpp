#pragma once

#include "tzrhiresource.hpp"
#include <vector>
#include <cstdint>

class TzRhiBuffer;
class TzRhiTexture;
class TzRhiSampler;

// Describes which shader resources (UBOs, samplers) are bound at each slot.
//
// Build a set of TzRhiShaderResourceBinding entries, set them with
// setBindings(), then call create(). Pass to pipeline and command buffer.

class TzRhiShaderResourceBinding
{
public:
    enum StageFlag {
        VertexStage   = 1 << 0,
        FragmentStage = 1 << 1
    };
    using StageFlags = unsigned int;

    static TzRhiShaderResourceBinding uniformBuffer(
        int binding, StageFlags stages,
        TzRhiBuffer *buf, uint32_t offset = 0, uint32_t size = 0)
    {
        TzRhiShaderResourceBinding b;
        b.m_binding = binding;
        b.m_stages  = stages;
        b.m_type    = UniformBuffer;
        b.m_buf     = buf;
        b.m_ubOffset = offset;
        b.m_ubSize   = size;
        return b;
    }

    static TzRhiShaderResourceBinding sampledTexture(
        int binding, StageFlags stages,
        TzRhiTexture *tex, TzRhiSampler *sampler)
    {
        TzRhiShaderResourceBinding b;
        b.m_binding  = binding;
        b.m_stages   = stages;
        b.m_type     = SampledTexture;
        b.m_tex      = tex;
        b.m_sampler  = sampler;
        return b;
    }

    enum Type { UniformBuffer, SampledTexture };

    int          binding()  const { return m_binding; }
    StageFlags   stages()   const { return m_stages; }
    Type         type()     const { return m_type; }
    TzRhiBuffer  *buffer()  const { return m_buf; }
    uint32_t     ubOffset() const { return m_ubOffset; }
    uint32_t     ubSize()   const { return m_ubSize; }
    TzRhiTexture *texture() const { return m_tex; }
    TzRhiSampler *sampler() const { return m_sampler; }

private:
    int          m_binding{0};
    StageFlags   m_stages{0};
    Type         m_type{UniformBuffer};
    TzRhiBuffer  *m_buf{nullptr};
    uint32_t     m_ubOffset{0};
    uint32_t     m_ubSize{0};
    TzRhiTexture *m_tex{nullptr};
    TzRhiSampler *m_sampler{nullptr};
};

class TzRhiShaderResourceBindings : public TzRhiResource
{
public:
    ~TzRhiShaderResourceBindings() override;

    TzRhiResource::Type resourceType() const override
    {
        return TzRhiResource::ShaderResourceBindings;
    }

    void setBindings(std::vector<TzRhiShaderResourceBinding> bindings)
    {
        m_bindings = std::move(bindings);
    }

    const std::vector<TzRhiShaderResourceBinding> &bindings() const
    {
        return m_bindings;
    }

    virtual bool create() = 0;

protected:
    explicit TzRhiShaderResourceBindings(TzRhi *rhi);

    std::vector<TzRhiShaderResourceBinding> m_bindings;
};
