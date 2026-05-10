#ifndef TZWINDOW_P_HPP
#define TZWINDOW_P_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzwindow.hpp>

#include "tzobject_p.hpp"

#include <functional>

class TzAbstractWindow;
class TzScene;
class TzTimer;

class TzWindowPrivate : public TzObjectPrivate
{
    TZ_DECLARE_PUBLIC(TzWindow)
public:
    ~TzWindowPrivate() override;

    TzAbstractWindow *platformWindow{ nullptr };
    TzScene *scene{ nullptr };
    TzTimer *paintTimer{ nullptr };
    std::function<void()> onClose;
};

#endif // TZWINDOW_P_HPP
