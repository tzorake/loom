#include <event-loop/TzGuiApplication>
#include <event-loop/TzAbstractWindow>
#include <event-loop/TzKeyEvent>
#include <event-loop/TzMouseEvent>
#include <event-loop/TzTimer>
#include <event-loop/TzScopedPointer>

#include <cmath>
#include <cstdint>
#include <print>
#include <vector>

static constexpr int kFps = 60;

static std::vector<uint32_t> generateFrame(std::size_t frameId, int width, int height)
{
    double t = frameId / (double)kFps;
    std::vector<uint32_t> pixels(width * height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t r = (uint8_t)((std::cos((double)x / width  + t) * 0.5 + 0.5) * 255);
            uint8_t g = (uint8_t)((std::sin((double)y / height + t) * 0.5 + 0.5) * 255);
            uint8_t b = (uint8_t)((std::cos((double)(x + y) / (width + height) + t) * 0.5 + 0.5) * 255);
            pixels[y * width + x] = (0xFFu << 24) | (r << 16) | (g << 8) | b;
        }
    }
    return pixels;
}

int main(int argc, char *argv[])
{
    TzGuiApplication app(argc, argv);

    int renderWidth  = 800;
    int renderHeight = 600;

    auto window = tz::as_scoped_ptr(app.createWindow(renderWidth, renderHeight));
    window->setTitle("event-loop window");
    window->setCloseCallback([&]() { app.quit(); });
    window->setResizeCallback([&](int w, int h) {
        renderWidth  = w;
        renderHeight = h;
        std::println("Resized: {}x{}", w, h);
    });
    window->setKeyCallback([&](TzKeyEvent *event) {
        if (event->key == Key::Escape) app.quit();
        else if (!event->utf8.empty()) std::println("Key: {}", event->utf8);
    });
    window->setMouseCallback([](TzMouseEvent *event) {
        switch (event->type) {
            case MouseEventType::ButtonPress:
                std::println("Mouse press   ({:.0f}, {:.0f}) button={}",
                    event->x, event->y, (int)event->button);
                break;
            case MouseEventType::ButtonRelease:
                std::println("Mouse release ({:.0f}, {:.0f}) button={}",
                    event->x, event->y, (int)event->button);
                break;
            case MouseEventType::Scroll:
                std::println("Scroll dx={:.1f} dy={:.1f}", event->scrollDx, event->scrollDy);
                break;
            default: break;
        }
    });
    window->show();

    std::size_t frame = 0;
    auto ticker = tz::as_scoped_ptr(
        TzTimer::repeat(app.eventDispatcher(), std::chrono::milliseconds(1000 / kFps),
            [&]() {
                window->render(generateFrame(frame++, renderWidth, renderHeight), renderWidth, renderHeight); 
            }));

    return app.exec();
}
