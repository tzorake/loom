#include <loom/tzrhi.hpp>

#if defined(LOOM_RHI_OPENGL)
#  include "opengl/tzrhiopengl.hpp"
#elif defined(LOOM_RHI_WEBGL)
#  include "webgl/tzrhiwebgl.hpp"
#endif

// ── TzRhi ─────────────────────────────────────────────────────────────────────

TzRhi::TzRhi(Backend backend)
    : m_backend(backend)
{}

TzRhi::~TzRhi() = default;

std::unique_ptr<TzRhi> TzRhi::create(Backend backend)
{
#if defined(LOOM_RHI_OPENGL)
    if (backend == Backend::OpenGL)
        return std::make_unique<TzRhiOpenGL>();
#endif

#if defined(LOOM_RHI_WEBGL)
    if (backend == Backend::WebGL)
        return std::make_unique<TzRhiWebGL>();
#endif

    (void)backend;
    return nullptr;
}
