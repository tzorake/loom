#ifndef TZCOREAPPLICATION_P_HPP
#define TZCOREAPPLICATION_P_HPP

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzcoreapplication.hpp>

#include <loom/tzevent.hpp>

#include <memory>
#include <mutex>
#include <vector>

class TzAbstractPlatformIntegration;
class TzAbstractEventDispatcher;
class TzEventLoop;
class TzSignalHandler;
class TzObject;

class TzCoreApplicationPrivate
{
    TZ_DECLARE_PUBLIC(TzCoreApplication)
public:
    TzCoreApplicationPrivate();

    TzCoreApplication *q_ptr{nullptr};

    std::unique_ptr<TzAbstractPlatformIntegration> platformIntegration;
    std::unique_ptr<TzAbstractEventDispatcher> eventDispatcher;
    std::unique_ptr<TzEventLoop> eventLoop;
    std::unique_ptr<TzSignalHandler> sigintHandler;

    int exitCode{0};

    struct PendingEvent
    {
        TzObject *receiver;
        std::unique_ptr<TzEvent> event;
    };
    std::mutex eventQueueMutex;
    std::vector<PendingEvent> eventQueue;
};

#endif // TZCOREAPPLICATION_P_HPP
