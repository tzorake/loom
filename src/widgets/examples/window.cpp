#include <loom/TzCloseEvent>
#include <loom/TzGuiApplication>
#include <loom/TzKeyEvent>
#include <loom/TzMouseEvent>
#include <loom/TzPaintEvent>
#include <loom/TzPainter>
#include <loom/TzResizeEvent>
#include <loom/TzTimer>
#include <loom/TzWindow>
#include <loom/TzLogging>
#include <cmath>
#include <cstdint>

static constexpr int kFps = 60;

class ExampleWindow : public TzWindow
{
public:
    ExampleWindow()
        : TzWindow(800, 600)
    {}

protected:
    void paintEvent(TzPaintEvent *event) override
    {
        const int w = width();
        const int h = height();
        TzPainter *p = event->painter();

        const double t = static_cast<double>(m_frame++) / kFps;

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                const double r = std::cos(static_cast<double>(x) / w + t) * 0.5 + 0.5;
                const double g = std::sin(static_cast<double>(y) / h + t) * 0.5 + 0.5;
                const double b = std::cos(static_cast<double>(x + y) / (w + h) + t) * 0.5 + 0.5;
                p->fillRect(static_cast<double>(x), static_cast<double>(y), 1.0, 1.0,
                            (0xFFu << 24)
                            | (static_cast<uint32_t>(r * 255.0) << 16)
                            | (static_cast<uint32_t>(g * 255.0) <<  8)
                            |  static_cast<uint32_t>(b * 255.0));
            }
        }
        update();
    }

    void closeEvent(TzCloseEvent *event) override
    {
        TzWindow::closeEvent(event);
        tzGuiApp->quit();
    }

    void keyEvent(TzKeyEvent *event) override
    {
        if (event->key() == Key::Escape) {
            tzGuiApp->quit();
        } else if (!event->utf8().empty()) {
            tzInfo("Key: {}", event->utf8());
        }
    }

    void mouseEvent(TzMouseEvent *event) override
    {
        switch (event->type()) {
        case TzEvent::MouseButtonPress:
            tzInfo("Mouse press   ({:.0f}, {:.0f}) button={}", event->x(), event->y(),
                         (int) event->button());
            break;
        case TzEvent::MouseButtonRelease:
            tzInfo("Mouse release ({:.0f}, {:.0f}) button={}", event->x(), event->y(),
                         (int) event->button());
            break;
        case TzEvent::MouseScroll:
            tzInfo("Scroll dx={:.1f} dy={:.1f}", event->scrollDx(), event->scrollDy());
            break;
        default:
            break;
        }
    }

    void resizeEvent(TzResizeEvent *event) override
    {
        TzWindow::resizeEvent(event);
        tzInfo("Resized: {}x{}", event->width(), event->height());
    }

private:
    std::size_t m_frame{0};
};

// ── Entry point ───────────────────────────────────────────────────────────────
//
// loom_add_executable() generates the platform-specific entry point that
// calls loom_main():
//   • Native: main()      — exec() blocks; stack or heap allocation both work.
//   • Web:    loom_init() — exec() is non-blocking; objects MUST be
//                           heap-allocated to survive loom_main() returning.
//
// This example uses heap allocation so the same source compiles and runs
// correctly on all platforms.

int loom_main(int argc, char *argv[])
{
    // Heap-allocate for web (WASM) compat.  On native these leak at process
    // exit, which is harmless.
    auto *app    = new TzGuiApplication(argc, argv);
    auto *window = new ExampleWindow();
    window->setTitle("loom window");
    window->show();
    return app->exec();
}
