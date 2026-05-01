#ifndef TZABSTRACTWINDOW_P_HPP
#define TZABSTRACTWINDOW_P_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzabstractwindow.hpp>

class TzAbstractWindowPrivate
{
    TZ_DECLARE_PUBLIC(TzAbstractWindow)
public:
    explicit TzAbstractWindowPrivate();

    TzAbstractWindow *q_ptr{ nullptr };

    TzAbstractWindow::CloseCallback  closeCallback;
    TzAbstractWindow::ResizeCallback resizeCallback;
    TzAbstractWindow::KeyCallback    keyCallback;
    TzAbstractWindow::MouseCallback  mouseCallback;
};

#endif // TZABSTRACTWINDOW_P_HPP
