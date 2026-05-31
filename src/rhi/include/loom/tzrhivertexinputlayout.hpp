#pragma once

#include <vector>
#include <cstdint>

// Vertex input layout — describes how vertex buffers are bound and how
// their contents map to shader input attributes.

class TzRhiVertexInputBinding
{
public:
    enum Classification {
        PerVertex,
        PerInstance
    };

    TzRhiVertexInputBinding() = default;
    TzRhiVertexInputBinding(uint32_t stride,
                            Classification classification = PerVertex,
                            int instanceStepRate = 1)
        : m_stride(stride)
        , m_classification(classification)
        , m_instanceStepRate(instanceStepRate)
    {}

    uint32_t       stride()           const { return m_stride; }
    Classification classification()   const { return m_classification; }
    int            instanceStepRate() const { return m_instanceStepRate; }

private:
    uint32_t       m_stride{0};
    Classification m_classification{PerVertex};
    int            m_instanceStepRate{1};
};

class TzRhiVertexInputAttribute
{
public:
    enum Format {
        Float4,
        Float3,
        Float2,
        Float1,
        UNorm4,   // vec4 normalised from 4×uint8
        UNorm2,   // vec2 normalised from 2×uint8
        UInt4,
        UInt3,
        UInt2,
        UInt1,
        SInt4,
        SInt3,
        SInt2,
        SInt1
    };

    TzRhiVertexInputAttribute() = default;
    TzRhiVertexInputAttribute(int binding, int location,
                              Format format, uint32_t offset)
        : m_binding(binding)
        , m_location(location)
        , m_format(format)
        , m_offset(offset)
    {}

    int      binding()  const { return m_binding; }
    int      location() const { return m_location; }
    Format   format()   const { return m_format; }
    uint32_t offset()   const { return m_offset; }

private:
    int      m_binding{0};
    int      m_location{0};
    Format   m_format{Float4};
    uint32_t m_offset{0};
};

class TzRhiVertexInputLayout
{
public:
    void setBindings(std::vector<TzRhiVertexInputBinding> bindings)
    {
        m_bindings = std::move(bindings);
    }
    void setAttributes(std::vector<TzRhiVertexInputAttribute> attrs)
    {
        m_attributes = std::move(attrs);
    }

    const std::vector<TzRhiVertexInputBinding>  &bindings()   const { return m_bindings; }
    const std::vector<TzRhiVertexInputAttribute> &attributes() const { return m_attributes; }

private:
    std::vector<TzRhiVertexInputBinding>   m_bindings;
    std::vector<TzRhiVertexInputAttribute> m_attributes;
};
