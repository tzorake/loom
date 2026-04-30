#include <event-loop/tzcoreapplication.hpp>
#include <event-loop/tzabstractplatformintegration.hpp>
#include <event-loop/tzabstracteventdispatcher.hpp>
#include <event-loop/tzeventloop.hpp>
#include <event-loop/tzsignalhandler.hpp>

#include "tzcoreapplication_p.hpp"

#include <csignal>
#include <stdexcept>

TzCoreApplication *TzCoreApplication::s_instance = nullptr;

TzCoreApplicationPrivate::TzCoreApplicationPrivate(TzCoreApplication *q)
    : q_ptr(q)
{
}

TzCoreApplication::TzCoreApplication(int /*argc*/, char * /*argv*/[])
    : d_ptr(new TzCoreApplicationPrivate(this))
{
    if (s_instance)
        throw std::runtime_error("TzCoreApplication: only one instance allowed");
    s_instance = this;

    d_ptr->platformIntegration = std::unique_ptr<TzAbstractPlatformIntegration>(createPlatformIntegration());
    d_ptr->eventDispatcher = std::unique_ptr<TzAbstractEventDispatcher>(d_ptr->platformIntegration->createEventDispatcher());
    d_ptr->eventLoop = std::make_unique<TzEventLoop>(d_ptr->eventDispatcher.get());
    d_ptr->sigintHandler = std::unique_ptr<TzSignalHandler>(
        TzSignalHandler::create(d_ptr->eventDispatcher.get(), SIGINT, [this](int) { quit(); }));
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
