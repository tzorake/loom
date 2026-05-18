#include <loom/tzconnect.hpp>
#include <loom/tzscopedeventlistener.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Fixture — sender inherits TzEventEmitter; signals are plain methods
// ---------------------------------------------------------------------------

class Button : public TzEventEmitter
{
public:
    void clicked(bool checked);
    void resized(int w, int h);
    void triggered();

    TZ_DECLARE_SIGNAL(Button::clicked);
    TZ_DECLARE_SIGNAL(Button::resized);
    TZ_DECLARE_SIGNAL(Button::triggered);
};

void Button::clicked(bool checked)  { TZ_EMIT(Button::clicked, checked); }
void Button::resized(int w, int h)  { TZ_EMIT(Button::resized, w, h); }
void Button::triggered()            { TZ_EMIT(Button::triggered); }

struct Handler
{
    int clickCount = 0;
    bool lastChecked = false;
    int lastW = 0, lastH = 0;
    int triggerCount = 0;

    void onClicked(bool checked) { ++clickCount; lastChecked = checked; }
    void onClicked_noarg()       { ++clickCount; }
    void onResized(int w, int h) { lastW = w; lastH = h; }
    void onResizedWOnly(int w)   { lastW = w; }
    void onTriggered()           { ++triggerCount; }
    void onConst() const         {}
};

// ---------------------------------------------------------------------------
// Basic emission
// ---------------------------------------------------------------------------

TEST(TzSignal, LambdaSlot)
{
    Button btn;
    int count = 0;
    auto c = connect(&btn, &Button::clicked, [&](bool) { ++count; });

    btn.clicked(false);
    btn.clicked(true);
    EXPECT_EQ(count, 2);
}

TEST(TzSignal, NoArgSignal)
{
    Button btn;
    int count = 0;
    auto c = connect(&btn, &Button::triggered, [&] { ++count; });

    btn.triggered();
    btn.triggered();
    EXPECT_EQ(count, 2);
}

TEST(TzSignal, MultipleArgsSignal)
{
    Button btn;
    int w = 0, h = 0;
    auto c = connect(&btn, &Button::resized, [&](int rw, int rh) { w = rw; h = rh; });

    btn.resized(800, 600);
    EXPECT_EQ(w, 800);
    EXPECT_EQ(h, 600);
}

TEST(TzSignal, MultipleSlotsFire)
{
    Button btn;
    int a = 0, b = 0, c = 0;
    auto c1 = connect(&btn, &Button::clicked, [&](bool) { ++a; });
    auto c2 = connect(&btn, &Button::clicked, [&](bool) { ++b; });
    auto c3 = connect(&btn, &Button::clicked, [&](bool) { ++c; });

    btn.clicked(false);
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 1);
    EXPECT_EQ(c, 1);
}

TEST(TzSignal, SlotsFireInOrder)
{
    Button btn;
    std::vector<int> order;
    auto c1 = connect(&btn, &Button::clicked, [&](bool) { order.push_back(1); });
    auto c2 = connect(&btn, &Button::clicked, [&](bool) { order.push_back(2); });
    auto c3 = connect(&btn, &Button::clicked, [&](bool) { order.push_back(3); });

    btn.clicked(false);
    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 3);
}

// ---------------------------------------------------------------------------
// Member function slots
// ---------------------------------------------------------------------------

TEST(TzSignal, NonConstMemberSlot)
{
    Button btn;
    Handler h;
    auto c = connect(&btn, &Button::clicked, &h, &Handler::onClicked);

    btn.clicked(true);
    EXPECT_EQ(h.clickCount, 1);
    EXPECT_TRUE(h.lastChecked);
}

TEST(TzSignal, ConstMemberSlot)
{
    Button btn;
    Handler h;
    auto c = connect(&btn, &Button::triggered, &h, &Handler::onConst);
    EXPECT_NO_THROW(btn.triggered());
}

// ---------------------------------------------------------------------------
// Slot takes fewer arguments than the signal (handled by TzEventEmitter)
// ---------------------------------------------------------------------------

TEST(TzSignal, MemberSlot_FewerArgs)
{
    Button btn;
    Handler h;
    auto c = connect(&btn, &Button::resized, &h, &Handler::onResizedWOnly);

    btn.resized(1920, 1080);
    EXPECT_EQ(h.lastW, 1920);
}

TEST(TzSignal, MemberSlot_NoArgs)
{
    Button btn;
    Handler h;
    auto c = connect(&btn, &Button::clicked, &h, &Handler::onClicked_noarg);

    btn.clicked(true);
    EXPECT_EQ(h.clickCount, 1);
}

TEST(TzSignal, LambdaSlot_FewerArgs)
{
    Button btn;
    int w = 0;
    auto c = connect(&btn, &Button::resized, [&](int rw) { w = rw; });

    btn.resized(640, 480);
    EXPECT_EQ(w, 640);
}

TEST(TzSignal, LambdaSlot_NoArgs)
{
    Button btn;
    int count = 0;
    auto c = connect(&btn, &Button::clicked, [&] { ++count; });

    btn.clicked(false);
    EXPECT_EQ(count, 1);
}

// ---------------------------------------------------------------------------
// shared_ptr receiver — tracked lifetime
// ---------------------------------------------------------------------------

TEST(TzSignal, SharedPtrReceiver_FiresWhileAlive)
{
    Button btn;
    auto h = std::make_shared<Handler>();
    auto c = connect(&btn, &Button::clicked, h, &Handler::onClicked);

    btn.clicked(true);
    EXPECT_EQ(h->clickCount, 1);
}

TEST(TzSignal, SharedPtrReceiver_SilentAfterDeath)
{
    Button btn;
    {
        auto h = std::make_shared<Handler>();
        auto c = connect(&btn, &Button::clicked, h, &Handler::onClicked);
        btn.clicked(false);
        EXPECT_EQ(h->clickCount, 1);
    }
    EXPECT_NO_THROW(btn.clicked(false));
}

// ---------------------------------------------------------------------------
// TzEventListener handle — returned directly from connect()
// ---------------------------------------------------------------------------

TEST(TzSignal, ListenerDisconnect)
{
    Button btn;
    int count = 0;
    TzEventListener c = connect(&btn, &Button::clicked, [&](bool) { ++count; });

    btn.clicked(false);
    EXPECT_EQ(count, 1);

    c.disconnect();
    EXPECT_FALSE(c.isConnected());

    btn.clicked(false);
    EXPECT_EQ(count, 1);
}

TEST(TzSignal, ScopedListenerDisconnectsOnDestruction)
{
    Button btn;
    int count = 0;
    {
        TzScopedEventListener sc = connect(&btn, &Button::clicked,
                                           [&](bool) { ++count; });
        btn.clicked(false);
        EXPECT_EQ(count, 1);
    }
    btn.clicked(false);
    EXPECT_EQ(count, 1);
}

// ---------------------------------------------------------------------------
// Reentrancy
// ---------------------------------------------------------------------------

TEST(TzSignal, SelfDisconnectDuringEmit)
{
    Button btn;
    int count = 0;
    TzEventListener c;
    c = connect(&btn, &Button::clicked, [&](bool) {
        ++count;
        c.disconnect();
    });

    btn.clicked(false);
    EXPECT_EQ(count, 1);
    btn.clicked(false);
    EXPECT_EQ(count, 1);
}

TEST(TzSignal, ReentrantEmit)
{
    Button btn;
    int outer = 0, inner = 0;
    auto ci = connect(&btn, &Button::triggered, [&] { ++inner; });
    auto co = connect(&btn, &Button::clicked, [&](bool) {
        ++outer;
        btn.triggered();
    });

    btn.clicked(false);
    EXPECT_EQ(outer, 1);
    EXPECT_EQ(inner, 1);
}

// ---------------------------------------------------------------------------
// Argument forwarding
// ---------------------------------------------------------------------------

TEST(TzSignal, BoolArgForwarded)
{
    Button btn;
    bool got = false;
    auto c = connect(&btn, &Button::clicked, [&](bool v) { got = v; });

    btn.clicked(true);
    EXPECT_TRUE(got);
    btn.clicked(false);
    EXPECT_FALSE(got);
}

TEST(TzSignal, MultiArgForwarded)
{
    Button btn;
    int rw = 0, rh = 0;
    auto c = connect(&btn, &Button::resized, [&](int w, int h) { rw = w; rh = h; });

    btn.resized(1920, 1080);
    EXPECT_EQ(rw, 1920);
    EXPECT_EQ(rh, 1080);
}
