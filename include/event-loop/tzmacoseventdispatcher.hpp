#ifndef TZMACOSEVENTDISPATCHER_HPP
#define TZMACOSEVENTDISPATCHER_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzabstracteventdispatcher.hpp>

#include <memory>

class TzMacosEventDispatcherPrivate;

class TzMacosEventDispatcher : public TzAbstractEventDispatcher
{
    TZ_DECLARE_PRIVATE(TzMacosEventDispatcher)
public:
    TzMacosEventDispatcher();
    virtual ~TzMacosEventDispatcher() override;

    virtual void processEvents() override;
    virtual void interrupt() override;

    virtual TimerHandle registerTimer(TimerInterval interval, bool singleShot, TimerCallback callback) override;
    virtual void unregisterTimer(TimerHandle) override;

    virtual NotifyHandle registerSocketNotifier(int fd, NotifyCallback callback) override;
    virtual void unregisterSocketNotifier(NotifyHandle handle) override;

private:
    std::unique_ptr<TzMacosEventDispatcherPrivate> d_ptr;
};

#endif // TZMACOSEVENTDISPATCHER_HPP
