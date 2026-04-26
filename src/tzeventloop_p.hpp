#ifndef TZEVENTLOOP_P
#define TZEVENTLOOP_P

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzeventloop.hpp>

class TzAbstractEventDispatcher;

class TzEventLoopPrivate
{
    TZ_DECLARE_PUBLIC(TzEventLoop)
public:
    explicit TzEventLoopPrivate(TzAbstractEventDispatcher *dispatcher);

private:
    TzEventLoop *q_ptr{ nullptr };

    TzAbstractEventDispatcher *dispatcher{ nullptr };
};

#endif // TZEVENTLOOP_P
