#pragma once

#include "tzshader.hpp"

// Shader stage — pairs a stage type (Vertex / Fragment) with a TzShader.
//
// Used when building a TzRhiGraphicsPipeline:
//   pipeline->setShaderStages({
//       TzRhiShaderStage(TzRhiShaderStage::Vertex,   TzShader::fromSource(vertSrc)),
//       TzRhiShaderStage(TzRhiShaderStage::Fragment, TzShader::fromSource(fragSrc)),
//   });

class TzRhiShaderStage
{
public:
    enum Type {
        Vertex,
        Fragment
    };

    TzRhiShaderStage(Type type, const TzShader &shader)
        : m_type(type), m_shader(shader) {}

    Type            type()   const { return m_type; }
    const TzShader &shader() const { return m_shader; }

private:
    Type     m_type;
    TzShader m_shader;
};
