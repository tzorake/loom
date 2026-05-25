#ifndef TZPLATFORMWINDOW_P_HPP
#define TZPLATFORMWINDOW_P_HPP

#include "tzplatformwindow.hpp"
#include <loom/tzclasshelpermacros.hpp>

class TzPlatformWindowPrivate
{
    TZ_DECLARE_PUBLIC(TzPlatformWindow)
public:
    TzPlatformWindow *q_ptr{nullptr};
    TzSurface *surface{nullptr};

    TzPlatformSurface::CloseCallback closeCallback;
    TzPlatformSurface::ResizeCallback resizeCallback;
    TzPlatformSurface::KeyCallback keyCallback;
    TzPlatformSurface::MouseCallback mouseCallback;
};

#endif // TZPLATFORMWINDOW_P_HPP
