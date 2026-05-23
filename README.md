# Loom

A lightweight C++ application framework inspired by Qt. Provides an event loop, platform abstraction, a pixel-buffer painter, an anchor-based widget system, and a model/view data layer.

## Modules

### loom-core

Event loop, timers, socket notifiers, signal handling, keyboard/mouse input, painter, and platform abstraction.

### loom-models

Qt-style item models and selection.

### loom-widgets

Retained-mode widget toolkit built on top of loom-core.

## Requirements

- CMake 3.25+
- C++23 compiler — Apple Clang 15+, GCC 13+, MSVC 19.38+
- Platform: macOS, Windows, Linux (Wayland or X11)

## Building

```sh
cmake -S . -B build
cmake --build build --parallel
```

### Options

| Option | Default | Description |
|---|---|---|
| `LOOM_BUILD_EXAMPLES` | `ON` | Build example programs |
| `LOOM_BUILD_TESTS` | `ON` | Build unit tests |
| `LOOM_LINUX_BACKEND` | `auto` | Linux display backend: `auto`, `wayland`, `x11` |

### Linking

```cmake
target_link_libraries(myapp PRIVATE loom-core)       # core only
target_link_libraries(myapp PRIVATE loom-widgets)    # widgets (pulls in core)
target_link_libraries(myapp PRIVATE loom-models)     # models (pulls in core)
```
