#pragma once

#include "tzrhiresource.hpp"
#include <cstdint>

// GPU buffer — vertex, index, or uniform data.
//
// Type controls upload frequency; UsageFlags describe how the buffer is bound.
// After setting all properties call create() once. Dynamic buffers may be
// updated every frame via TzRhiResourceUpdateBatch::updateDynamicBuffer().

class TzRhiBuffer : public TzRhiResource
{
public:
    enum Type {
        Immutable, // Uploaded once, never changed again
        Static,    // Changed infrequently
        Dynamic    // Changed every frame (or nearly so)
    };

    enum UsageFlag {
        VertexBuffer  = 1 << 0,
        IndexBuffer   = 1 << 1,
        UniformBuffer = 1 << 2
    };
    using UsageFlags = unsigned int;

    ~TzRhiBuffer() override;

    TzRhiResource::Type resourceType() const override { return TzRhiResource::Buffer; }

    void setType(Type t)         { m_type = t; }
    void setUsage(UsageFlags u)  { m_usage = u; }
    void setSize(uint32_t bytes) { m_size = bytes; }

    Type        type()  const { return m_type; }
    UsageFlags  usage() const { return m_usage; }
    uint32_t    size()  const { return m_size; }

    virtual bool create() = 0;

protected:
    TzRhiBuffer(TzRhi *rhi, Type type, UsageFlags usage, uint32_t size);

    Type       m_type;
    UsageFlags m_usage;
    uint32_t   m_size;
};
