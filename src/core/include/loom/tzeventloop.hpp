#ifndef TZEVENTLOOP_HPP
#define TZEVENTLOOP_HPP

#include <loom/tzclasshelpermacros.hpp>

#include <memory>

class TzAbstractEventDispatcher;
class TzEventLoopPrivate;

class TzEventLoop
{
    TZ_DECLARE_PRIVATE(TzEventLoop)
public:
    explicit TzEventLoop(TzAbstractEventDispatcher *eventDispatcher);
    ~TzEventLoop();

    void exec();
    void quit();

private:
    std::unique_ptr<TzEventLoopPrivate> d_ptr;
};

#endif // TZEVENTLOOP_HPP
