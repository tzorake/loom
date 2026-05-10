#ifndef TZABSTRACTWINDOW_P_HPP
#define TZABSTRACTWINDOW_P_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzabstractwindow.hpp>

class TzAbstractWindowPrivate
{
    TZ_DECLARE_PUBLIC(TzAbstractWindow)
public:
    TzAbstractWindow *q_ptr{ nullptr };
    TzSurface *surface{ nullptr };

    TzPlatformSurface::CloseCallback closeCallback;
    TzPlatformSurface::ResizeCallback resizeCallback;
    TzPlatformSurface::KeyCallback keyCallback;
    TzPlatformSurface::MouseCallback mouseCallback;
};

#endif // TZABSTRACTWINDOW_P_HPP
