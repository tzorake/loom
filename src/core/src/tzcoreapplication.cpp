#include <loom/tzabstracteventdispatcher.hpp>
#include <loom/tzabstractplatformintegration.hpp>
#include <loom/tzcoreapplication.hpp>
#include <loom/tzevent.hpp>
#include <loom/tzeventloop.hpp>
#include <loom/tzobject.hpp>
#include <loom/tzsignalhandler.hpp>

#include "tzcoreapplication_p.hpp"
#include "tzobject_p.hpp"

#include <algorithm>
#include <csignal>
#include <stdexcept>

TzCoreApplication *TzCoreApplication::s_instance = nullptr;

TzCoreApplicationPrivate::TzCoreApplicationPrivate(TzCoreApplication *q)
    : q_ptr(q)
{}

TzCoreApplication::TzCoreApplication(int /*argc*/, char * /*argv*/[])
    : d_ptr(new TzCoreApplicationPrivate(this))
{
    if (s_instance)
        throw std::runtime_error("TzCoreApplication: only one instance allowed");
    s_instance = this;

    d_ptr->platformIntegration = std::unique_ptr<TzAbstractPlatformIntegration>(
        createPlatformIntegration());
    d_ptr->eventDispatcher = std::unique_ptr<TzAbstractEventDispatcher>(
        d_ptr->platformIntegration->createEventDispatcher());
    d_ptr->eventLoop = std::make_unique<TzEventLoop>(d_ptr->eventDispatcher.get());
    d_ptr->sigintHandler = std::unique_ptr<TzSignalHandler>(
        TzSignalHandler::create(d_ptr->eventDispatcher.get(), SIGINT, [this](int) { quit(); }));

    d_ptr->eventDispatcher->setPreWaitCallback([this] { processPostedEvents(); });
}

TzCoreApplication::~TzCoreApplication()
{
    s_instance = nullptr;
}

TzCoreApplication *TzCoreApplication::instance()
{
    return s_instance;
}

TzAbstractPlatformIntegration *TzCoreApplication::platformIntegration() const
{
    return d_ptr->platformIntegration.get();
}

TzAbstractEventDispatcher *TzCoreApplication::eventDispatcher() const
{
    return d_ptr->eventDispatcher.get();
}

int TzCoreApplication::exec()
{
    d_ptr->exitCode = 0;
    d_ptr->eventLoop->exec();
    return d_ptr->exitCode;
}

void TzCoreApplication::quit(int exitCode)
{
    d_ptr->exitCode = exitCode;
    d_ptr->eventLoop->quit();
}

void TzCoreApplication::postEvent(TzObject *receiver, TzEvent *event)
{
    TzCoreApplication *app = instance();
    if (!app) {
        delete event;
        return;
    }
    {
        std::lock_guard lock(app->d_ptr->eventQueueMutex);
        app->d_ptr->eventQueue.push_back({receiver, std::unique_ptr<TzEvent>(event)});
    }
    app->d_ptr->eventDispatcher->wakeUp();
}

bool TzCoreApplication::sendEvent(TzObject *receiver, TzEvent *event)
{
    if (!receiver || !event)
        return false;
    return receiver->event(event);
}

void TzCoreApplication::removePostedEvents(TzObject *receiver)
{
    std::lock_guard lock(d_ptr->eventQueueMutex);
    std::erase_if(d_ptr->eventQueue, [receiver](const TzCoreApplicationPrivate::PendingEvent &pe) {
        if (pe.receiver != receiver)
            return false;
        // Reset the deferred-delete guard so deleteLater() can be reused
        // if the caller removes events without destroying the object.
        if (pe.event->type() == TzEvent::DeferredDelete)
            pe.receiver->d_ptr->pendingDelete = false;
        return true;
    });
}

void TzCoreApplication::processPostedEvents()
{
    std::vector<TzCoreApplicationPrivate::PendingEvent> batch;
    {
        std::lock_guard lock(d_ptr->eventQueueMutex);
        batch.swap(d_ptr->eventQueue);
    }
    for (TzCoreApplicationPrivate::PendingEvent &pe : batch) {
        if (pe.event->type() == TzEvent::DeferredDelete)
            delete pe.receiver; // receiver owns itself; destructor cleans up remaining events
        else
            pe.receiver->event(pe.event.get());
    }
}
