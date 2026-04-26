#include <event-loop/tzkeyboardhandler.hpp>
#include <event-loop/tzabstracteventdispatcher.hpp>
#include <event-loop/tzsocketnotifier.hpp>
#include <event-loop/tzkeyevent.hpp>

#include "tzkeyboardhandler_p.hpp"
#include "platform/posix/tzposixkeyboardhandler_p.hpp"

#include <unistd.h>
#include <algorithm>

TzKeyboardHandlerPrivate::TzKeyboardHandlerPrivate(TzAbstractEventDispatcher *dispatcher)
    : dispatcher(dispatcher)
{
}

TzKeyboardHandlerPrivate::~TzKeyboardHandlerPrivate()
{
}

void TzKeyboardHandlerPrivate::processKeyEvent(TzKeyEvent *event)
{
    if (callback) callback(event);
}

TzPosixKeyboardHandlerPrivate::TzPosixKeyboardHandlerPrivate(TzAbstractEventDispatcher *dispatcher)
    : TzKeyboardHandlerPrivate(dispatcher)
{
}

void TzPosixKeyboardHandlerPrivate::start()
{
    if (tcgetattr(STDIN_FILENO, &this->origTermios) == -1)
        throw std::runtime_error("tcgetattr failed");

    this->rawTermios = this->origTermios;
    cfmakeraw(&this->rawTermios);
    this->rawTermios.c_lflag &= ~(ECHO | ICANON);
    this->rawTermios.c_oflag |= OPOST;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &this->rawTermios) == -1)
        throw std::runtime_error("tcsetattr failed");
    
    this->rawActive = true;
}

void TzPosixKeyboardHandlerPrivate::stop()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &this->origTermios);
}

void TzPosixKeyboardHandlerPrivate::onInputAvailable()
{
    char buf[16];
    ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
    if (n <= 0)
        return;

    buffer.append(buf, n);

    while (!buffer.empty()) {
        unsigned char c = buffer[0];

        // Escape sequence handling
        if (c == 0x1B) {
            if (buffer.size() >= 2 && buffer[1] == 0x1B) {
                buffer.erase(0, 1);
                TzKeyEvent event{ Key::Escape, 0, "" };
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
                        TzKeyEvent event{ Key::Up, 0, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[B") {
                        TzKeyEvent event{ Key::Down, 0, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[C") {
                        TzKeyEvent event{ Key::Right, 0, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[D") {
                        TzKeyEvent event{ Key::Left, 0, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[1~") {
                        TzKeyEvent event{ Key::Home, 0, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[4~") {
                        TzKeyEvent event{ Key::End, 0, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[5~") {
                        TzKeyEvent event{ Key::PageUp, 0, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[6~") {
                        TzKeyEvent event{ Key::PageDown, 0, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[11~") {
                        TzKeyEvent event{ Key::F1, 0, "" };
                        processKeyEvent(&event);
                    } else if (seq == "\x1B[12~") {
                        TzKeyEvent event{ Key::F2, 0, "" };
                        processKeyEvent(&event);
                    } else {
                        TzKeyEvent event{ Key::Unknown, 0, "" };
                        processKeyEvent(&event);
                    } continue;
                } else {
                    break; // incomplete sequence
                }
            }
            // lone ESC (or unrecognised sequence)
            buffer.erase(0, 1);
            TzKeyEvent event{ Key::Escape, 0, "" };
            processKeyEvent(&event);
            continue;
        }

        // UTF‑8 length detection
        int len = 1;
        if ((c & 0x80) == 0x00)      len = 1;
        else if ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;
        else {
            buffer.erase(0, 1); // invalid
            continue;
        }
        if (buffer.size() < len) break;

        std::string seq = buffer.substr(0, len);
        buffer.erase(0, len);

        // Handle control characters (single byte only)
        if (len == 1) {
            switch (c) {
                case 0x01: {
                    TzKeyEvent event{ Key::Unknown, (int)KeyModifier::Ctrl, "" };
                    processKeyEvent(&event);
                } continue;
                case 0x03: {
                    TzKeyEvent event{ Key::Unknown, (int)KeyModifier::Ctrl, "" };
                    processKeyEvent(&event);
                } continue;
                case 0x04: {
                    TzKeyEvent event{ Key::Unknown, (int)KeyModifier::Ctrl, "" };
                    processKeyEvent(&event);
                } continue;
                case 0x08:
                case 0x7F: {
                    TzKeyEvent event{ Key::Backspace, 0, "" };
                    processKeyEvent(&event);
                } continue;
                case '\n':
                case '\r': {
                    TzKeyEvent event{ Key::Enter, 0, "" };
                    processKeyEvent(&event);
                } continue;
                case '\t': {
                    TzKeyEvent event{ Key::Tab, 0, "" };
                    processKeyEvent(&event);
                } continue;
                default: break;
            }
        }
        // Printable text (ASCII or multi‑byte UTF‑8)
        TzKeyEvent event{ Key::Unknown, 0, seq };
        processKeyEvent(&event);
    }
}

TzKeyboardHandler::TzKeyboardHandler(TzAbstractEventDispatcher *dispatcher)
#ifdef _WIN32
    : d_ptr(new TzWindowsKeyboardHandlerPrivate(dispatcher))
#else
    : d_ptr(new TzPosixKeyboardHandlerPrivate(dispatcher))
#endif
{
}

TzKeyboardHandler::~TzKeyboardHandler()
{
    stop();
}

void TzKeyboardHandler::setCallback(KeyCallback callback)
{
    d_ptr->callback = callback;
}

void TzKeyboardHandler::start()
{
    if (!d_ptr->dispatcher)
        throw std::runtime_error("KeyboardHandler::start() without dispatcher");

    if (!d_ptr->callback)
        throw std::runtime_error("KeyboardHandler::start() without callback");
    
    if (d_ptr->active)
        return;

    d_ptr->start();
    d_ptr->notifier = std::make_unique<TzSocketNotifier>(d_ptr->dispatcher);
    d_ptr->notifier->setFd(STDIN_FILENO);
    d_ptr->notifier->setCallback([this](int) { d_ptr->onInputAvailable(); });
    d_ptr->notifier->start();

    d_ptr->active = true;
}

void TzKeyboardHandler::stop()
{
    if (!d_ptr->active)
        return;

    d_ptr->notifier.reset();

    if (d_ptr->rawActive) {
        d_ptr->stop();
        d_ptr->rawActive = false;
    }
    d_ptr->active = false;
}
