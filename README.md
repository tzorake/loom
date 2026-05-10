# Loom

A lightweight C++ application framework inspired by Qt. Loom provides an event loop, platform abstraction, signal handling, timers, and an anchor-based widget system.

## Features

- **Event loop** with timer and file descriptor (socket notifier) support
- **Platform abstraction** — clean separation between portable and platform-specific code
- **Signal handler** — process signal integration into the event loop
- **Painter** — simple pixel-buffer renderer with text and rectangle primitives
- **Anchor-based layout** — QML-style anchors for positioning widgets relative to each other
- **Window management** — create and show native windows, nest child windows
- **Qt-style pimpl** — stable ABI-friendly private implementation pattern throughout

## Requirements

- CMake 3.25+
- C++23 compiler (Apple Clang 15+ / GCC 13+ / MSVC 19.38+)
- macOS (Linux/Windows platform backends not yet implemented)

## Building

```sh
cmake -S . -B build
cmake --build build --parallel
```

To skip building examples:

```sh
cmake -S . -B build -DLOOM_BUILD_EXAMPLES=OFF
cmake --build build --parallel
```

## CMake targets

| Target | Description |
|---|---|
| `loom` | Umbrella target — links core + widgets. Use this if you want everything. |
| `loom-core` | Core only: event loop, timers, signals, painter, platform abstraction |
| `loom-widgets` | Widget system (automatically links `loom-core`) |

```cmake
target_link_libraries(myapp PRIVATE loom)
```

## Project structure

```
src/
  core/               # loom-core
    include/loom/     # public headers
    src/              # sources and private headers
    src/platform/     # platform-specific backends
  widgets/            # loom-widgets
    include/loom/     # public headers
    src/              # sources and private headers
examples/
```

## Examples

### Console app — event loop, keyboard input, timers

```cpp
#include <loom/TzCoreApplication>
#include <loom/TzKeyboardHandler>
#include <loom/TzTimer>
#include <loom/TzScopedPointer>
#include <print>

int main(int argc, char *argv[])
{
    TzCoreApplication app(argc, argv);

    auto consoleInput = tz::as_scoped_ptr(
        app.platformIntegration()->createConsoleInput());

    auto keyboard = tz::as_scoped_ptr(TzKeyboardHandler::create(
        app.eventDispatcher(), consoleInput.get(),
        [&](TzKeyEvent *event) {
            if (event->utf8() == "q") app.quit();
            else std::println("Key: {}", event->utf8());
        }));

    auto tick = tz::as_scoped_ptr(
        TzTimer::repeat(app.eventDispatcher(), std::chrono::seconds(1),
            [] { std::println("Tick"); }));

    return app.exec();
}
```

### Widget app — windows, anchor layout, custom widgets

```cpp
#include <loom/TzGuiApplication>
#include <loom/TzWindow>
#include <loom/TzWidget>
#include <loom/TzAnchors>
#include <loom/TzPainter>

class MyWidget : public TzWidget
{
public:
    explicit MyWidget(TzWidget *parent = nullptr) : TzWidget(parent)
    {
        setImplicitSize(200.0, 100.0);
    }

protected:
    void paint(TzPainter *p) override
    {
        p->fillRect(0.0, 0.0, width(), height(), 0xFF313244);
        p->drawText(8.0, 8.0, "Hello, Loom!", 0xFFCDD6F4);
    }
};

int main(int argc, char *argv[])
{
    TzGuiApplication app(argc, argv);

    TzWindow window(800, 600);
    window.setTitle("My App");
    window.setOnClose([&] { app.quit(); });

    auto *root = new MyWidget;
    window.setRootWidget(root);
    window.show();

    return app.exec();
}
```

### Nested windows

```cpp
TzWindow parent(640, 480);
parent.setTitle("Parent");
parent.setOnClose([&] { app.quit(); });

// Child window lifetime is tied to the parent
TzWindow *child = new TzWindow(320, 240, &parent);
child->setTitle("Child");
child->show();

parent.show();
```
