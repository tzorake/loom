#ifndef TZEVENTLOOP_P
#define TZEVENTLOOP_P

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzeventloop.hpp>

class TzAbstractEventDispatcher;

class TzEventLoopPrivate
{
    TZ_DECLARE_PUBLIC(TzEventLoop)
public:
    explicit TzEventLoopPrivate(TzAbstractEventDispatcher *eventDispatcher);

private:
    TzEventLoop *q_ptr{ nullptr };

    TzAbstractEventDispatcher *eventDispatcher{ nullptr };
};

#endif // TZEVENTLOOP_P
