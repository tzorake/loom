#include <loom/tzkeyboardhandler.hpp>
#include <tzkeyboardhandler_p.hpp>
#include <loom/tzabstractconsoleinput.hpp>
#include <loom/tzabstracteventdispatcher.hpp>
#include <loom/tzcoreapplication.hpp>
#include <loom/tzkeyevent.hpp>
#include <loom/tzsocketnotifier.hpp>

#include <algorithm>

TzKeyboardHandlerPrivate::TzKeyboardHandlerPrivate(TzAbstractConsoleInput *consoleInput)
    : consoleInput(consoleInput)
{
}

TzKeyboardHandlerPrivate::~TzKeyboardHandlerPrivate()
{
}

void TzKeyboardHandlerPrivate::processKeyEvent(TzKeyEvent *event)
{
    TzCoreApplication::postEvent(q_ptr, event->clone());
}

void TzKeyboardHandlerPrivate::onInputAvailable()
{
    std::string chunk = consoleInput->read();
    if (chunk.empty())
        return;

    buffer.append(chunk);

    while (!buffer.empty()) {
        unsigned char c = buffer[0];

        // Escape sequence handling
        if (c == 0x1B) {
            if (buffer.size() >= 2 && buffer[1] == 0x1B) {
                buffer.erase(0, 1);
                TzKeyEvent event(TzEvent::KeyPress, Key::Escape);
                processKeyEvent(&event);
                continue;
            }
            if (buffer.size() >= 3 && buffer[1] == '[') {
                auto it = std::find_if(buffer.begin() + 2, buffer.end(),
                                       [](char ch) { return ch >= 'A' && ch <= 'Z'; });
                if (it != buffer.end()) {
                    std::string seq(buffer.begin(), it + 1);
                    buffer.erase(0, seq.size());
                    Key k = Key::Unknown;
                    if (seq == "\x1B[A")
                        k = Key::Up;
                    else if (seq == "\x1B[B")
                        k = Key::Down;
                    else if (seq == "\x1B[C")
                        k = Key::Right;
                    else if (seq == "\x1B[D")
                        k = Key::Left;
                    else if (seq == "\x1B[1~")
                        k = Key::Home;
                    else if (seq == "\x1B[4~")
                        k = Key::End;
                    else if (seq == "\x1B[5~")
                        k = Key::PageUp;
                    else if (seq == "\x1B[6~")
                        k = Key::PageDown;
                    else if (seq == "\x1B[11~")
                        k = Key::F1;
                    else if (seq == "\x1B[12~")
                        k = Key::F2;
                    TzKeyEvent event(TzEvent::KeyPress, k);
                    processKeyEvent(&event);
                    continue;
                } else {
                    break; // incomplete sequence
                }
            }
            // lone ESC (or unrecognised sequence)
            buffer.erase(0, 1);
            TzKeyEvent event(TzEvent::KeyPress, Key::Escape);
            processKeyEvent(&event);
            continue;
        }

        // UTF-8 length detection
        int len = 1;
        if ((c & 0x80) == 0x00)
            len = 1;
        else if ((c & 0xE0) == 0xC0)
            len = 2;
        else if ((c & 0xF0) == 0xE0)
            len = 3;
        else if ((c & 0xF8) == 0xF0)
            len = 4;
        else {
            buffer.erase(0, 1); // invalid
            continue;
        }
        if ((int) buffer.size() < len)
            break;

        std::string seq = buffer.substr(0, len);
        buffer.erase(0, len);

        // Handle control characters (single byte only)
        if (len == 1) {
            switch (c) {
            case 0x01:
            case 0x03:
            case 0x04: {
                TzKeyEvent event(TzEvent::KeyPress, Key::Unknown, KeyModifier::Ctrl);
                processKeyEvent(&event);
            }
                continue;
            case 0x08:
            case 0x7F: {
                TzKeyEvent event(TzEvent::KeyPress, Key::Backspace);
                processKeyEvent(&event);
            }
                continue;
            case '\n':
            case '\r': {
                TzKeyEvent event(TzEvent::KeyPress, Key::Enter);
                processKeyEvent(&event);
            }
                continue;
            case '\t': {
                TzKeyEvent event(TzEvent::KeyPress, Key::Tab);
                processKeyEvent(&event);
            }
                continue;
            default:
                break;
            }
        }
        // Printable text (ASCII or multi-byte UTF-8)
        TzKeyEvent event(TzEvent::KeyPress, Key::Unknown, KeyModifier::None, seq);
        processKeyEvent(&event);
    }
}

TzKeyboardHandler::TzKeyboardHandler(TzAbstractConsoleInput *consoleInput, TzObject *parent)
    : TzObject(*new TzKeyboardHandlerPrivate(consoleInput), parent)
{
}

TzKeyboardHandler::~TzKeyboardHandler()
{
    stop();
}

void TzKeyboardHandler::setCallback(KeyCallback callback)
{
    TZ_D(TzKeyboardHandler);
    d->callback = std::move(callback);
}

bool TzKeyboardHandler::event(TzEvent *event)
{
    TZ_D(TzKeyboardHandler);
    if ((event->type() == TzEvent::KeyPress || event->type() == TzEvent::KeyRelease) && d->callback) {
        d->callback(static_cast<TzKeyEvent *>(event));
        return true;
    }
    return TzObject::event(event);
}

void TzKeyboardHandler::start()
{
    TZ_D(TzKeyboardHandler);
    TzAbstractEventDispatcher *eventDispatcher = tzApp->eventDispatcher();
    if (!eventDispatcher)
        throw std::runtime_error("TzKeyboardHandler::start() without eventDispatcher");

    if (!d->callback)
        throw std::runtime_error("TzKeyboardHandler::start() without callback");

    if (d->active)
        return;

    d->consoleInput->start();
    d->notifier = std::make_unique<TzSocketNotifier>();
    d->notifier->setFd(d->consoleInput->fd());
    d->notifier->setCallback([this](int) { TZ_D(TzKeyboardHandler); d->onInputAvailable(); });
    d->notifier->start();

    d->active = true;
}

void TzKeyboardHandler::stop()
{
    TZ_D(TzKeyboardHandler);
    if (!d->active)
        return;

    d->notifier.reset();
    d->consoleInput->stop();
    d->active = false;
}

TzKeyboardHandler *TzKeyboardHandler::create(TzAbstractConsoleInput *consoleInput, KeyCallback callback, TzObject *parent)
{
    TzKeyboardHandler *h = new TzKeyboardHandler(consoleInput, parent);
    h->setCallback(std::move(callback));
    h->start();
    return h;
}
