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

### Web (WebAssembly)

**1. Install the WASI SDK**

Download from https://github.com/WebAssembly/wasi-sdk/releases and expose it:

```sh
export WASI_SDK_PATH=/path/to/wasi-sdk-25.0   # add to ~/.zshrc or ~/.bashrc
```

The toolchain also checks `/opt/wasi-sdk` and `~/wasi-sdk` automatically.

**2. Configure and build**

```sh
cmake -B build-wasm \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/wasm32.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DLOOM_BUILD_TESTS=OFF

cmake --build build-wasm --target window
```

**3. Serve and open**

Each web example is self-contained in its own output directory:

```sh
python3 -m http.server 8080 --directory build-wasm/src/widgets/examples/window
# open http://localhost:8080
```

Replace `window` with any other example name (`widget`, `nested_windows`, …).

### Linking

```cmake
target_link_libraries(myapp PRIVATE loom-core)       # core only
target_link_libraries(myapp PRIVATE loom-widgets)    # widgets (pulls in core)
target_link_libraries(myapp PRIVATE loom-models)     # models (pulls in core)
```

### Writing cross-platform applications

Use `loom_add_executable()` instead of `add_executable()` to build for both
native and web from the same source file and the same CMake call:

```cmake
include(cmake/LoomHelpers.cmake)
loom_add_executable(my_app my_app.cpp)
```

Define `loom_main` instead of `main`. Heap-allocate application objects so
they survive `exec()` returning on web (where the event loop is driven by
`requestAnimationFrame` rather than blocking):

```cpp
#include <loom/TzGuiApplication>
#include <loom/TzWindow>

class MyWindow : public TzWindow { /* ... */ };

int loom_main(int argc, char *argv[])
{
    auto *app    = new TzGuiApplication(argc, argv);
    auto *window = new MyWindow();
    window->setTitle("My App");
    window->show();
    return app->exec();
}
```
