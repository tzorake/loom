#include <loom/tzabstractconsoleinput.hpp>
#include <loom/tzabstracteventdispatcher.hpp>
#include <loom/tzcoreapplication.hpp>
#include <loom/tzkeyboardhandler.hpp>
#include <loom/tzkeyevent.hpp>
#include <loom/tzsocketnotifier.hpp>

#include "tzkeyboardhandler_p.hpp"

#include <algorithm>

TzKeyboardHandlerPrivate::TzKeyboardHandlerPrivate(TzAbstractEventDispatcher *eventDispatcher,
                                                   TzAbstractConsoleInput *consoleInput)
    : eventDispatcher(eventDispatcher)
    , consoleInput(consoleInput)
{}

TzKeyboardHandlerPrivate::~TzKeyboardHandlerPrivate() {}

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

TzKeyboardHandler::TzKeyboardHandler(TzAbstractEventDispatcher *eventDispatcher,
                                     TzAbstractConsoleInput *consoleInput, TzObject *parent)
    : TzObject(parent)
    , d_ptr(new TzKeyboardHandlerPrivate(eventDispatcher, consoleInput))
{
    d_ptr->q_ptr = this;
}

TzKeyboardHandler::~TzKeyboardHandler()
{
    stop();
}

void TzKeyboardHandler::setCallback(KeyCallback callback)
{
    d_ptr->callback = std::move(callback);
}

bool TzKeyboardHandler::event(TzEvent *e)
{
    if ((e->type() == TzEvent::KeyPress || e->type() == TzEvent::KeyRelease) && d_ptr->callback) {
        d_ptr->callback(static_cast<TzKeyEvent *>(e));
        return true;
    }
    return TzObject::event(e);
}

void TzKeyboardHandler::start()
{
    if (!d_ptr->eventDispatcher)
        throw std::runtime_error("KeyboardHandler::start() without eventDispatcher");

    if (!d_ptr->callback)
        throw std::runtime_error("KeyboardHandler::start() without callback");

    if (d_ptr->active)
        return;

    d_ptr->consoleInput->start();
    d_ptr->notifier = std::make_unique<TzSocketNotifier>(d_ptr->eventDispatcher);
    d_ptr->notifier->setFd(d_ptr->consoleInput->fd());
    d_ptr->notifier->setCallback([this](int) { d_ptr->onInputAvailable(); });
    d_ptr->notifier->start();

    d_ptr->active = true;
}

void TzKeyboardHandler::stop()
{
    if (!d_ptr->active)
        return;

    d_ptr->notifier.reset();
    d_ptr->consoleInput->stop();
    d_ptr->active = false;
}

TzKeyboardHandler *TzKeyboardHandler::create(TzAbstractEventDispatcher *eventDispatcher,
                                             TzAbstractConsoleInput *consoleInput,
                                             KeyCallback callback, TzObject *parent)
{
    TzKeyboardHandler *h = new TzKeyboardHandler(eventDispatcher, consoleInput, parent);
    h->setCallback(std::move(callback));
    h->start();
    return h;
}
