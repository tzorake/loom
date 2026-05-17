#include <loom/TzAbstractWindow>
#include <loom/TzGuiApplication>
#include <loom/TzKeyEvent>
#include <loom/TzMouseEvent>
#include <loom/TzScopedPointer>
#include <loom/TzTimer>

#include <cmath>
#include <cstdint>
#include <print>
#include <vector>

static constexpr int kFps = 60;

int main(int argc, char *argv[])
{
    TzGuiApplication app(argc, argv);

    int renderWidth = 800;
    int renderHeight = 600;

    auto window = tz::as_scoped_ptr(app.createWindow(renderWidth, renderHeight));
    window->setTitle("event-loop window");
    window->setCloseCallback([&]() { app.quit(); });
    window->setKeyCallback([&](TzKeyEvent *event) {
        if (event->key() == Key::Escape)
            app.quit();
        else if (!event->utf8().empty())
            std::println("Key: {}", event->utf8());
    });
    window->setMouseCallback([](TzMouseEvent *event) {
        switch (event->type()) {
        case TzEvent::MouseButtonPress:
            std::println("Mouse press   ({:.0f}, {:.0f}) button={}", event->x(), event->y(),
                         (int) event->button());
            break;
        case TzEvent::MouseButtonRelease:
            std::println("Mouse release ({:.0f}, {:.0f}) button={}", event->x(), event->y(),
                         (int) event->button());
            break;
        case TzEvent::MouseScroll:
            std::println("Scroll dx={:.1f} dy={:.1f}", event->scrollDx(), event->scrollDy());
            break;
        default:
            break;
        }
    });

    std::size_t frame = 0;
    std::vector<uint32_t> pixels;

    window->setResizeCallback([&](int w, int h) {
        renderWidth = w;
        renderHeight = h;
        std::println("Resized: {}x{}", w, h);
    });

    window->show();

    auto ticker = tz::as_scoped_ptr(
        TzTimer::repeat(app.eventDispatcher(), std::chrono::milliseconds(1000 / kFps), [&]() {
            const int w = renderWidth;
            const int h = renderHeight;
            pixels.resize(static_cast<size_t>(w) * static_cast<size_t>(h));

            const double t = static_cast<double>(frame++) / kFps;

            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    const double r = std::cos(static_cast<double>(x) / w + t) * 0.5 + 0.5;
                    const double g = std::sin(static_cast<double>(y) / h + t) * 0.5 + 0.5;
                    const double b = std::cos(static_cast<double>(x + y) / (w + h) + t) * 0.5 + 0.5;
                    pixels[static_cast<size_t>(y) * static_cast<size_t>(w) + static_cast<size_t>(x)]
                        = (0xFFu << 24) | (static_cast<uint32_t>(r * 255.0) << 16)
                          | (static_cast<uint32_t>(g * 255.0) << 8)
                          | static_cast<uint32_t>(b * 255.0);
                }
            }

            window->render(pixels, w, h);
        }));

    return app.exec();
}
