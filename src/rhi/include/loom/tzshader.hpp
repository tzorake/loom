#pragma once

#include <string>

// GLSL shader source holder.
//
// Desktop backend: expects GLSL 3.30 core (#version 330 core).
// WebGL backend:   expects GLSL ES 3.00 (#version 300 es).
//
// The caller is responsible for supplying the correct preamble. TzShader
// itself is backend-agnostic — it just holds the source text.

class TzShader
{
public:
    TzShader() = default;

    static TzShader fromSource(const char *glsl)
    {
        TzShader s;
        s.m_source = glsl;
        return s;
    }

    const std::string &source() const { return m_source; }
    bool               isEmpty() const { return m_source.empty(); }

private:
    std::string m_source;
};
