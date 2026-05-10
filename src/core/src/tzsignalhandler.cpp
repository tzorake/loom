#include <loom/tzsignalhandler.hpp>
#include <loom/tzsocketnotifier.hpp>

#include "tzsignalhandler_p.hpp"
#include "tzsignalpipe.hpp"

#include <csignal>
#include <stdexcept>
#include <unordered_map>

static std::unordered_map<int, int> gSignalWriteFds; // signo -> write end of pipe

static void pipeSignalHandler(int signo)
{
    auto it = gSignalWriteFds.find(signo);
    if (it == gSignalWriteFds.end())
        return;
    const char byte = static_cast<char>(signo);
    (void)TzSignalPipe::write(it->second, &byte, 1);
}

TzSignalHandlerPrivate::TzSignalHandlerPrivate(TzAbstractEventDispatcher *eventDispatcher)
    : eventDispatcher(eventDispatcher)
{
}

TzSignalHandler::TzSignalHandler(TzAbstractEventDispatcher *eventDispatcher)
    : d_ptr(new TzSignalHandlerPrivate(eventDispatcher))
{
}

TzSignalHandler::~TzSignalHandler()
{
    stop();
}

void TzSignalHandler::setSignal(int signo)
{
    d_ptr->signo = signo;
}

void TzSignalHandler::setCallback(SignalCallback callback)
{
    d_ptr->callback = std::move(callback);
}

void TzSignalHandler::start()
{
    if (d_ptr->active)
        return;

    if (d_ptr->signo < 0)
        throw std::runtime_error("TzSignalHandler::start() — no signal set");
    if (!d_ptr->callback)
        throw std::runtime_error("TzSignalHandler::start() — no callback set");

    if (TzSignalPipe::create(d_ptr->pipeFds) == -1)
        throw std::runtime_error("TzSignalHandler::start() — pipe() failed");

    TzSignalPipe::makeNonBlocking(d_ptr->pipeFds[1]);

    gSignalWriteFds[d_ptr->signo] = d_ptr->pipeFds[1];
    signal(d_ptr->signo, pipeSignalHandler);

    d_ptr->notifier = std::make_unique<TzSocketNotifier>(d_ptr->eventDispatcher);
    d_ptr->notifier->setFd(d_ptr->pipeFds[0]);
    d_ptr->notifier->setCallback([this](int) {
        char byte{};
        (void)TzSignalPipe::read(d_ptr->pipeFds[0], &byte, 1);
        if (d_ptr->callback)
            d_ptr->callback(static_cast<int>(byte));
    });
    d_ptr->notifier->start();

    d_ptr->active = true;
}

void TzSignalHandler::stop()
{
    if (!d_ptr->active)
        return;

    d_ptr->notifier.reset();
    signal(d_ptr->signo, SIG_DFL);
    gSignalWriteFds.erase(d_ptr->signo);

    TzSignalPipe::close(d_ptr->pipeFds[0]);
    TzSignalPipe::close(d_ptr->pipeFds[1]);
    d_ptr->pipeFds[0] = d_ptr->pipeFds[1] = -1;

    d_ptr->active = false;
}

TzSignalHandler *TzSignalHandler::create(TzAbstractEventDispatcher *eventDispatcher, int signo, SignalCallback callback)
{
    TzSignalHandler *h = new TzSignalHandler(eventDispatcher);
    h->setSignal(signo);
    h->setCallback(std::move(callback));
    h->start();
    return h;
}
