#include <loom/TzCloseEvent>
#include <loom/TzGuiApplication>
#include <loom/TzMouseEvent>
#include <loom/TzPainter>
#include <loom/TzPoint>
#include <loom/TzRect>
#include <loom/TzTimer>
#include <loom/TzWidget>
#include <loom/TzWindow>
#include <loom/tzfixed.hpp>
#include <cstdint>
#include <deque>

static constexpr int FRAC = 32;
static TzFixed tzFixed(double v) { return TzFixed(v, FRAC); }

static const TzFixed G = tzFixed(980.0);
static const TzFixed L1 = tzFixed(150.0);
static const TzFixed L2 = tzFixed(130.0);
static const TzFixed M1 = tzFixed(2.0);
static const TzFixed M2 = tzFixed(1.0);
static const TzFixed DT = tzFixed(0.016);
static const TzFixed DAMP = tzFixed(0.9998);
static const TzFixed TWO = tzFixed(2.0);
static const TzFixed NEG1 = tzFixed(-1.0);
static const TzFixed HALF = tzFixed(0.5);
static const TzFixed P30 = tzFixed(0.30);
static const TzFixed GRAB_R2 = tzFixed((14.0 * 2.5) * (14.0 * 2.5));
static constexpr int TRAIL = 350;

struct BobPosition {
    TzFixed px{FRAC}, py{FRAC}, b1x{FRAC}, b1y{FRAC}, b2x{FRAC}, b2y{FRAC};
};

class PendulumWidget : public TzWidget
{
public:
    explicit PendulumWidget(TzWidget *parent = nullptr)
        : TzWidget(parent)
        , m_theta1(tzFixed(1.2))
        , m_theta2(tzFixed(0.4))
        , m_omega1(tzFixed(0.0))
        , m_omega2(tzFixed(0.0))
    {}

    void step()
    {
        if (!m_dragging)
            integrate();
        update();
    }

protected:
    void paint(TzPainter *p) override
    {
        const double W = width(), H = height();
        if (W < 1.0 || H < 1.0) return;

        p->fillRect(0, 0, W, H, 0xFF0D0D1A);

        const BobPosition pos = bobPositions();

        // Trail
        const int n = static_cast<int>(m_trail.size());
        for (int i = 1; i < n; ++i) {
            const auto alpha = static_cast<uint8_t>(220 * i / n);
            const uint32_t col  = (static_cast<uint32_t>(alpha) << 24) | 0x00FF6B6B;
            p->drawLine(m_trail[i - 1], m_trail[i], col, 1.5);
        }

        // Pivot
        p->drawCircle({pos.px.toDouble(), pos.py.toDouble()}, 5.0, 0xFFEEEEEE);

        // Rods
        p->drawLine({pos.px.toDouble(),  pos.py.toDouble()},
                    {pos.b1x.toDouble(), pos.b1y.toDouble()}, 0xFFAAAAAA, 2.5);
        p->drawLine({pos.b1x.toDouble(), pos.b1y.toDouble()},
                    {pos.b2x.toDouble(), pos.b2y.toDouble()}, 0xFFAAAAAA, 2.5);

        // Bob 1 (blue), Bob 2 (coral)
        p->drawCircle({pos.b1x.toDouble(), pos.b1y.toDouble()}, 14.0, 0xFF4FC3F7);
        p->drawCircle({pos.b2x.toDouble(), pos.b2y.toDouble()}, 14.0, 0xFFFF6B6B);
    }

    bool event(TzEvent *e) override
    {
        if (e->type() == TzEvent::MouseButtonPress) {
            auto *me = static_cast<TzMouseEvent *>(e);
            if (me->button() == MouseButton::Left) {
                if (tryGrab(me->x(), me->y())) {
                    m_trail.clear();
                    return true;
                }
            }
        }
        if (e->type() == TzEvent::MouseMove) {
            auto *me = static_cast<TzMouseEvent *>(e);
            if (m_dragging) {
                applyDrag(me->x(), me->y());
                update();
                return true;
            }
        }
        if (e->type() == TzEvent::MouseButtonRelease) {
            m_dragging = 0;
            m_omega1 = tzFixed(0.0);
            m_omega2 = tzFixed(0.0);
            return true;
        }
        return TzWidget::event(e);
    }

    void geometryChanged(const TzRect &, const TzRect &) override { update(); }

private:
    BobPosition bobPositions() const
    {
        BobPosition pos;
        pos.px  = tzFixed(width())  * HALF;
        pos.py  = tzFixed(height()) * P30;
        pos.b1x = pos.px + L1 * std::sin(m_theta1);
        pos.b1y = pos.py + L1 * std::cos(m_theta1);
        pos.b2x = pos.b1x + L2 * std::sin(m_theta2);
        pos.b2y = pos.b1y + L2 * std::cos(m_theta2);
        return pos;
    }

    void integrate()
    {
        const TzFixed delta   = m_theta1 - m_theta2;
        const TzFixed two_del = TWO * delta;

        const TzFixed sin_t1 = std::sin(m_theta1);
        const TzFixed cos_t1 = std::cos(m_theta1);
        const TzFixed sin_d = std::sin(delta);
        const TzFixed cos_d = std::cos(delta);
        const TzFixed cos_2d = std::cos(two_del);
        const TzFixed sin_t12 = std::sin(m_theta1 - TWO * m_theta2);

        const TzFixed o1sq = m_omega1 * m_omega1;
        const TzFixed o2sq = m_omega2 * m_omega2;

        // D = 2m₁ + m₂ − m₂ cos(2Δ)
        const TzFixed D = (TWO * M1 + M2) - M2 * cos_2d;

        // α₁
        const TzFixed a1 =
            (NEG1 * G * (TWO * M1 + M2) * sin_t1
             - M2 * G * sin_t12
             - TWO * sin_d * M2 * (o2sq * L2 + o1sq * L1 * cos_d))
            / (L1 * D);

        // α₂
        const TzFixed a2 =
            (TWO * sin_d *
             (o1sq * L1 * (M1 + M2)
              + G * (M1 + M2) * cos_t1
              + o2sq * L2 * M2 * cos_d))
            / (L2 * D);

        m_omega1 += a1 * DT;
        m_omega2 += a2 * DT;
        m_omega1 *= DAMP;
        m_omega2 *= DAMP;
        m_theta1 += m_omega1 * DT;
        m_theta2 += m_omega2 * DT;

        const BobPosition pos = bobPositions();
        m_trail.push_back({pos.b2x.toDouble(), pos.b2y.toDouble()});
        if (static_cast<int>(m_trail.size()) > TRAIL)
            m_trail.pop_front();
    }

    bool tryGrab(double mx, double my)
    {
        const TzFixed mx_f = tzFixed(mx);
        const TzFixed my_f = tzFixed(my);

        const BobPosition pos = bobPositions();

        auto sqDist = [](const TzFixed &ax, const TzFixed &ay,
                         const TzFixed &bx, const TzFixed &by) {
            const TzFixed dx = ax - bx, dy = ay - by;
            return dx * dx + dy * dy;
        };

        if (sqDist(mx_f, my_f, pos.b2x, pos.b2y) < GRAB_R2) { m_dragging = 2; return true; }
        if (sqDist(mx_f, my_f, pos.b1x, pos.b1y) < GRAB_R2) { m_dragging = 1; return true; }
        return false;
    }

    void applyDrag(double mx, double my)
    {
        const TzFixed mx_f = tzFixed(mx);
        const TzFixed my_f = tzFixed(my);

        const BobPosition pos = bobPositions();

        if (m_dragging == 1) {
            m_theta1 = std::atan2(mx_f - pos.px, my_f - pos.py);
            m_omega1 = tzFixed(0.0);
        } else if (m_dragging == 2) {
            m_theta2 = std::atan2(mx_f - pos.b1x, my_f - pos.b1y);
            m_omega2 = tzFixed(0.0);
        }
    }

    TzFixed m_theta1, m_theta2; // angles from downward vertical
    TzFixed m_omega1, m_omega2; // angular velocities (rad/s)

    int m_dragging = 0; // 0 = none, 1 = bob1, 2 = bob2

    std::deque<TzPoint> m_trail; // trail stores TzPoint (double pixels) — conversion
                                 // happens once per frame at push_back
};

class AppWindow : public TzWindow
{
public:
    using TzWindow::TzWindow;

protected:
    void closeEvent(TzCloseEvent *e) override
    {
        TzWindow::closeEvent(e);
        TzGuiApplication::instance()->quit();
    }
};

int loom_main(int argc, char *argv[])
{
    auto *app = new TzGuiApplication(argc, argv);
    auto *window = new AppWindow(800, 600);
    window->setTitle("Double Pendulum");

    auto *pendulum = new PendulumWidget();
    window->setRootWidget(pendulum);
    window->show();

    TzTimer::repeat(std::chrono::milliseconds(16), [pendulum] { pendulum->step(); });

    return app->exec();
}
