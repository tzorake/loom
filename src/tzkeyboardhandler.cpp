#include <event-loop/tzkeyboardhandler.hpp>
#include <event-loop/tzabstracteventdispatcher.hpp>
#include <event-loop/tzabstractconsoleinput.hpp>
#include <event-loop/tzsocketnotifier.hpp>
#include <event-loop/tzkeyevent.hpp>

#include "tzkeyboardhandler_p.hpp"

#include <algorithm>

TzKeyboardHandlerPrivate::TzKeyboardHandlerPrivate(TzAbstractEventDispatcher *dispatcher, TzAbstractConsoleInput *consoleInput)
    : dispatcher(dispatcher)
    , consoleInput(consoleInput)
{
}

TzKeyboardHandlerPrivate::~TzKeyboardHandlerPrivate()
{
}

void TzKeyboardHandlerPrivate::processKeyEvent(TzKeyEvent *event)
{
    if (callback) callback(event);
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
                TzKeyEvent event{ Key::Escape, KeyModifier::None, "" };
                processKeyEvent(&event);
                continue;
            }
            if (buffer.size() >= 3 && buffer[1] == '[') {
                auto it = std::find_if(buffer.begin() + 2, buffer.end(),
                    [](char ch) { return ch >= 'A' && ch <= 'Z'; });
                if (it != buffer.end()) {
                    std::string seq(buffer.begin(), it + 1);
                    buffer.erase(0, seq.size());
                    if (seq == "\x1B[A") {
                        TzKeyEvent event{ Key::Up, KeyModifier::None, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[B") {
                        TzKeyEvent event{ Key::Down, KeyModifier::None, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[C") {
                        TzKeyEvent event{ Key::Right, KeyModifier::None, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[D") {
                        TzKeyEvent event{ Key::Left, KeyModifier::None, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[1~") {
                        TzKeyEvent event{ Key::Home, KeyModifier::None, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[4~") {
                        TzKeyEvent event{ Key::End, KeyModifier::None, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[5~") {
                        TzKeyEvent event{ Key::PageUp, KeyModifier::None, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[6~") {
                        TzKeyEvent event{ Key::PageDown, KeyModifier::None, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[11~") {
                        TzKeyEvent event{ Key::F1, KeyModifier::None, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[12~") {
                        TzKeyEvent event{ Key::F2, KeyModifier::None, "" };
                        processKeyEvent(&event);
                    } else {
                        TzKeyEvent event{ Key::Unknown, KeyModifier::None, "" };
                        processKeyEvent(&event);
                    } continue;
                } else {
                    break; // incomplete sequence
                }
            }
            // lone ESC (or unrecognised sequence)
            buffer.erase(0, 1);
            TzKeyEvent event{ Key::Escape, KeyModifier::None, "" };
            processKeyEvent(&event);
            continue;
        }

        // UTF-8 length detection
        int len = 1;
        if ((c & 0x80) == 0x00)      len = 1;
        else if ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;
        else {
            buffer.erase(0, 1); // invalid
            continue;
        }
        if ((int)buffer.size() < len) break;

        std::string seq = buffer.substr(0, len);
        buffer.erase(0, len);

        // Handle control characters (single byte only)
        if (len == 1) {
            switch (c) {
                case 0x01:
                case 0x03:
                case 0x04: {
                    TzKeyEvent event{ Key::Unknown, KeyModifier::Ctrl, "" };
                    processKeyEvent(&event);
                } continue;
                case 0x08:
                case 0x7F: {
                    TzKeyEvent event{ Key::Backspace, KeyModifier::None, "" };
                    processKeyEvent(&event);
                } continue;
                case '\n':
                case '\r': {
                    TzKeyEvent event{ Key::Enter, KeyModifier::None, "" };
                    processKeyEvent(&event);
                } continue;
                case '\t': {
                    TzKeyEvent event{ Key::Tab, KeyModifier::None, "" };
                    processKeyEvent(&event);
                } continue;
                default: break;
            }
        }
        // Printable text (ASCII or multi-byte UTF-8)
        TzKeyEvent event{ Key::Unknown, KeyModifier::None, seq };
        processKeyEvent(&event);
    }
}

TzKeyboardHandler::TzKeyboardHandler(TzAbstractEventDispatcher *dispatcher, TzAbstractConsoleInput *consoleInput)
    : d_ptr(new TzKeyboardHandlerPrivate(dispatcher, consoleInput))
{
}

TzKeyboardHandler::~TzKeyboardHandler()
{
    stop();
}

void TzKeyboardHandler::setCallback(KeyCallback callback)
{
    d_ptr->callback = std::move(callback);
}

void TzKeyboardHandler::start()
{
    if (!d_ptr->dispatcher)
        throw std::runtime_error("KeyboardHandler::start() without dispatcher");

    if (!d_ptr->callback)
        throw std::runtime_error("KeyboardHandler::start() without callback");

    if (d_ptr->active)
        return;

    d_ptr->consoleInput->start();
    d_ptr->notifier = std::make_unique<TzSocketNotifier>(d_ptr->dispatcher);
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

TzKeyboardHandler *TzKeyboardHandler::create(TzAbstractEventDispatcher *dispatcher, TzAbstractConsoleInput *consoleInput, KeyCallback callback)
{
    TzKeyboardHandler *h = new TzKeyboardHandler(dispatcher, consoleInput);
    h->setCallback(std::move(callback));
    h->start();
    return h;
}
