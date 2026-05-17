#include <gtest/gtest.h>
#include <loom/tzeventemitter.hpp>
#include <loom/tzeventlistener.hpp>
#include <loom/tzscopedeventlistener.hpp>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <sstream>

using namespace std::string_literals;

struct CerrRedirect {
    std::stringstream buffer;
    std::streambuf* old;
    CerrRedirect() : old(std::cerr.rdbuf(buffer.rdbuf())) {}
    ~CerrRedirect() { std::cerr.rdbuf(old); }
    std::string str() const { return buffer.str(); }
};

TEST(TzEventEmitter, BasicEmit)
{
    TzEventEmitter emitter;
    int counter = 0;
    auto h = emitter.on("inc", [&counter] { ++counter; });

    EXPECT_EQ(counter, 0);
    emitter.emit("inc");
    EXPECT_EQ(counter, 1);
    emitter.emit("inc");
    EXPECT_EQ(counter, 2);
}

TEST(TzEventEmitter, OnceFiresOnlyOnce)
{
    TzEventEmitter emitter;
    int count = 0;
    auto h = emitter.once("fire", [&count] { ++count; });
    EXPECT_EQ(count, 0);
    emitter.emit("fire");
    EXPECT_EQ(count, 1);
    emitter.emit("fire");
    EXPECT_EQ(count, 1);
}

TEST(TzEventEmitter, OnceHandleDisconnectsAfterEmit)
{
    TzEventEmitter emitter;
    bool fired = false;
    auto h = emitter.once("test", [&fired] { fired = true; });
    EXPECT_TRUE(h.isConnected());
    emitter.emit("test");
    EXPECT_TRUE(fired);
    EXPECT_FALSE(h.isConnected());
}

TEST(TzEventEmitter, RemoveListenerByHandle)
{
    TzEventEmitter emitter;
    int value = 0;
    auto h = emitter.on("set", [&value] { value = 42; });
    EXPECT_TRUE(h.isConnected());
    emitter.emit("set");
    EXPECT_EQ(value, 42);

    h.disconnect();
    EXPECT_FALSE(h.isConnected());
    value = 0;
    emitter.emit("set");
    EXPECT_EQ(value, 0);
}

TEST(TzEventEmitter, RemoveListenerViaEmitter)
{
    TzEventEmitter emitter;
    int calls = 0;
    auto h = emitter.on("click", [&calls] { ++calls; });
    emitter.emit("click");
    EXPECT_EQ(calls, 1);

    emitter.removeListener(h);
    EXPECT_FALSE(h.isConnected());
    emitter.emit("click");
    EXPECT_EQ(calls, 1);
}

TEST(TzEventEmitter, RemoveAllListenersSpecificEvent)
{
    TzEventEmitter emitter;
    int a = 0, b = 0;
    emitter.on("e1", [&a] { ++a; });
    emitter.on("e2", [&b] { ++b; });
    emitter.removeAllListeners("e1");

    emitter.emit("e1");
    emitter.emit("e2");
    EXPECT_EQ(a, 0);
    EXPECT_EQ(b, 1);
}

TEST(TzEventEmitter, RemoveAllListenersAllEvents)
{
    TzEventEmitter emitter;
    int a = 0, b = 0;
    emitter.on("e1", [&a] { ++a; });
    emitter.on("e2", [&b] { ++b; });
    emitter.removeAllListeners();

    emitter.emit("e1");
    emitter.emit("e2");
    EXPECT_EQ(a, 0);
    EXPECT_EQ(b, 0);
    EXPECT_TRUE(emitter.eventNames().empty());
}

TEST(TzEventEmitter, ErrorEventThrowsWithoutListener)
{
    TzEventEmitter emitter;
    EXPECT_THROW(emitter.emit("error", std::string("something wrong")), std::runtime_error);
}

TEST(TzEventEmitter, ErrorEventWithListenerDoesNotThrow)
{
    TzEventEmitter emitter;
    bool caught = false;
    emitter.on("error", [&caught](const std::string& msg) {
        caught = true;
        EXPECT_EQ(msg, "oops");
    });
    EXPECT_NO_THROW(emitter.emit("error", std::string("oops")));
    EXPECT_TRUE(caught);
}

TEST(TzEventEmitter, QueryMethods)
{
    TzEventEmitter emitter;
    EXPECT_EQ(emitter.listenerCount("e1"), 0u);
    EXPECT_TRUE(emitter.eventNames().empty());

    auto h1 = emitter.on("e1", []{});
    EXPECT_EQ(emitter.listenerCount("e1"), 1u);
    EXPECT_EQ(emitter.eventNames().size(), 1u);
    EXPECT_EQ(emitter.eventNames().front(), "e1");

    auto h2 = emitter.on("e2", []{});
    EXPECT_EQ(emitter.listenerCount("e2"), 1u);
    EXPECT_EQ(emitter.eventNames().size(), 2u);

    h1.disconnect();
    EXPECT_EQ(emitter.listenerCount("e1"), 0u);
    auto names = emitter.eventNames();
    EXPECT_EQ(std::find(names.begin(), names.end(), "e1"), names.end());
    EXPECT_EQ(names.size(), 1u);
}

TEST(TzEventEmitter, TemplateOverloadsVariousArgs)
{
    TzEventEmitter emitter;

    int result = 0;
    emitter.on("mul2", [&result](int x) { result = x * 2; });
    emitter.emit("mul2", 21);
    EXPECT_EQ(result, 42);

    std::string concat;
    emitter.on("concat", [&concat](int n, const std::string& s, double d) {
        concat = std::to_string(n) + s + std::to_string(d);
    });
    emitter.emit("concat", 3, std::string("pi~"), 3.14);
    EXPECT_EQ(concat, "3pi~3.140000");

    int calls = 0;
    emitter.on("tick", [&calls] { ++calls; });
    emitter.emit("tick");
    EXPECT_EQ(calls, 1);
}

TEST(TzEventEmitter, SelfRemovalDuringEmit)
{
    TzEventEmitter emitter;
    int a = 0, b = 0, c = 0;

    TzEventListener hA = emitter.on("self", [&] { ++a; });
    TzEventListener hB = emitter.on("self", [&] { ++b; hB.disconnect(); });
    TzEventListener hC = emitter.on("self", [&] { ++c; });

    emitter.emit("self");
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 1);
    EXPECT_EQ(c, 1);
    EXPECT_FALSE(hB.isConnected());

    a = b = c = 0;
    emitter.emit("self");
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 0);
    EXPECT_EQ(c, 1);
}

TEST(TzEventEmitter, RemoveOtherDuringEmit)
{
    TzEventEmitter emitter;
    int x = 0, y = 0;
    auto hX = emitter.on("evt", [&] { ++x; });
    auto hY = emitter.on("evt", [&] { ++y; hX.disconnect(); });

    emitter.emit("evt");
    EXPECT_EQ(x, 1);
    EXPECT_EQ(y, 1);

    x = y = 0;
    emitter.emit("evt");
    EXPECT_EQ(x, 0);
    EXPECT_EQ(y, 1);
}

TEST(TzEventEmitter, ListenerOrder)
{
    TzEventEmitter emitter;
    std::vector<int> order;

    emitter.on("seq", [&] { order.push_back(1); });
    emitter.on("seq", [&] { order.push_back(2); });
    emitter.on("seq", [&] { order.push_back(3); });

    emitter.emit("seq");
    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 3);
}

TEST(TzEventEmitter, AddListenerDuringEmit)
{
    TzEventEmitter emitter;
    int count = 0;
    auto h1 = emitter.on("add", [&] {
        ++count;
        emitter.on("add", [&] { ++count; });
    });

    emitter.emit("add");
    EXPECT_EQ(count, 1);
    emitter.emit("add");
    EXPECT_EQ(count, 3);
}

TEST(TzEventEmitter, OnceSelfRemove)
{
    TzEventEmitter emitter;
    int called = 0;
    auto h = emitter.once("test", [&] { ++called; });
    emitter.emit("test");
    EXPECT_EQ(called, 1);
    emitter.emit("test");
    EXPECT_EQ(called, 1);
}

TEST(TzEventEmitter, HandleOutlivesEmitter)
{
    TzEventListener handle;
    {
        TzEventEmitter emitter;
        handle = emitter.on("e", []{});
        EXPECT_TRUE(handle.isConnected());
        emitter.emit("e");
    }
    EXPECT_FALSE(handle.isConnected());
    EXPECT_NO_THROW(handle.disconnect());
}

TEST(TzEventEmitter, MaxListeners)
{
    TzEventEmitter emitter;
    emitter.setMaxListeners(2);

    CerrRedirect cerrRedirect;

    auto h1 = emitter.on("ev", []{});
    auto h2 = emitter.on("ev", []{});
    std::string output = cerrRedirect.str();
    EXPECT_TRUE(output.empty());

    auto h3 = emitter.on("ev", []{});
    output = cerrRedirect.str();
    EXPECT_NE(output.find("MaxListenersExceededWarning"), std::string::npos);
    EXPECT_NE(output.find("event 'ev'"), std::string::npos);

    EXPECT_EQ(emitter.listenerCount("ev"), 3u);
}

TEST(TzEventEmitter, DefaultMaxListeners)
{
    TzEventEmitter::setDefaultMaxListeners(2);
    TzEventEmitter emitter;
    CerrRedirect cerrRedirect;
    auto h1 = emitter.on("ev", []{});
    auto h2 = emitter.on("ev", []{});
    auto h3 = emitter.on("ev", []{});
    std::string output = cerrRedirect.str();
    EXPECT_NE(output.find("MaxListenersExceededWarning"), std::string::npos);
    EXPECT_EQ(emitter.listenerCount("ev"), 3u);
    TzEventEmitter::setDefaultMaxListeners(10);
}

TEST(TzEventEmitter, IdenticalLambdasAreSeparate)
{
    TzEventEmitter emitter;
    int counter = 0;
    auto fn = [&counter] { ++counter; };

    auto h1 = emitter.on("ev", fn);
    auto h2 = emitter.on("ev", fn);
    EXPECT_EQ(emitter.listenerCount("ev"), 2u);
    emitter.emit("ev");
    EXPECT_EQ(counter, 2);

    h1.disconnect();
    EXPECT_EQ(emitter.listenerCount("ev"), 1u);
    emitter.emit("ev");
    EXPECT_EQ(counter, 3);
}

TEST(TzEventEmitter, EmitNoListenersNoExcept)
{
    TzEventEmitter emitter;
    EXPECT_NO_THROW(emitter.emit("some_event", 42));
}

TEST(TzEventEmitter, AddListenerAlias)
{
    TzEventEmitter emitter;
    int a = 0;
    auto h = emitter.on("test", [&a] { a = 99; });
    emitter.emit("test");
    EXPECT_EQ(a, 99);
    h.disconnect();
}

TEST(TzEventEmitter, EventNamesAfterRemoval)
{
    TzEventEmitter emitter;
    auto h1 = emitter.on("a", []{});
    auto h2 = emitter.on("b", []{});
    h1.disconnect();
    auto names = emitter.eventNames();
    EXPECT_EQ(names.size(), 1u);
    EXPECT_EQ(names[0], "b");
}

TEST(TzEventEmitter, ErrorEventMsgInException)
{
    TzEventEmitter emitter;
    try {
        emitter.emit("error", std::string("Boom"));
        FAIL() << "Expected exception";
    } catch (const std::runtime_error& e) {
        EXPECT_EQ(std::string(e.what()), "Boom"s);
    }
}

TEST(TzEventEmitter, ReentrantEmit)
{
    TzEventEmitter emitter;
    int count = 0;
    emitter.on("outer", [&] {
        ++count;
        emitter.emit("inner");
    });
    emitter.on("inner", [&] { ++count; });

    emitter.emit("outer");
    EXPECT_EQ(count, 2);
}

TEST(TzEventEmitter, ListenerAcceptsFewerArguments)
{
    TzEventEmitter emitter;

    int receivedInt = 0;
    emitter.on("data", [&receivedInt](int x) {
        receivedInt = x;
    });

    int receivedInt2 = 0;
    double receivedDouble = 0.0;
    emitter.on("data", [&receivedInt2, &receivedDouble](int x, double d) {
        receivedInt2 = x;
        receivedDouble = d;
    });

    int receivedInt3 = 0;
    double receivedDouble3 = 0.0;
    std::string receivedString;
    emitter.on("data", [&](int x, double d, const std::string& s) {
        receivedInt3 = x;
        receivedDouble3 = d;
        receivedString = s;
    });

    emitter.emit("data", 42, 3.14, std::string("hello"));

    EXPECT_EQ(receivedInt, 42);

    EXPECT_EQ(receivedInt2, 42);
    EXPECT_EQ(receivedDouble, 3.14);

    EXPECT_EQ(receivedInt3, 42);
    EXPECT_EQ(receivedDouble3, 3.14);
    EXPECT_EQ(receivedString, "hello");
}

TEST(TzEventEmitter, WrongArgumentType)
{
    TzEventEmitter emitter;
    bool called = false;
    emitter.on("ev", [&](int x) {
        called = true;
        EXPECT_EQ(x, 42);
    });

    EXPECT_THROW(emitter.emit("ev", std::string("not an int")), std::bad_any_cast);
    EXPECT_FALSE(called);
}

TEST(TzEventEmitter, PrivateSignalPattern)
{
    class MyService {
        struct PrivateSignal {};
    public:
        MyService() {
            emitter.on("secret", [this](int data, PrivateSignal) {
                lastSecret = data;
            });
        }

        void publishSecret(int data) {
            emitter.emit("secret", data, PrivateSignal{});
        }

        int lastSecret = 0;
        TzEventEmitter emitter;
    };

    MyService service;

    int externalCounter = 0;
    service.emitter.on("secret", [&externalCounter](int data) {
        externalCounter += data;
    });

    service.publishSecret(10);
    EXPECT_EQ(service.lastSecret, 10);
    EXPECT_EQ(externalCounter, 10);

    service.publishSecret(7);
    EXPECT_EQ(service.lastSecret, 7);
    EXPECT_EQ(externalCounter, 17);
}

TEST(TzEventEmitter, EventListenerSwap)
{
    TzEventEmitter emitter;
    int a = 0, b = 0;
    auto hA = emitter.on("evA", [&] { ++a; });
    auto hB = emitter.on("evB", [&] { ++b; });

    EXPECT_TRUE(hA.isConnected());
    EXPECT_TRUE(hB.isConnected());

    hA.swap(hB);

    emitter.emit("evA");
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 0);

    emitter.emit("evB");
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 1);

    EXPECT_TRUE(hA.isConnected());
    EXPECT_TRUE(hB.isConnected());
}

TEST(TzEventEmitter, EventListenerSwapDisconnected)
{
    TzEventEmitter emitter;
    auto h1 = emitter.on("ev", [] {});
    TzEventListener h2;
    EXPECT_FALSE(h2.isConnected());

    h1.swap(h2);
    EXPECT_FALSE(h1.isConnected());
    EXPECT_TRUE(h2.isConnected());

    emitter.emit("ev");
    EXPECT_FALSE(h1.isConnected());
    EXPECT_TRUE(h2.isConnected());
}

TEST(TzEventEmitter, EventListenerOperatorBool)
{
    TzEventEmitter emitter;
    auto h = emitter.on("click", [] {});
    EXPECT_TRUE(h);

    h.disconnect();
    EXPECT_FALSE(h);

    TzEventListener empty;
    EXPECT_FALSE(empty);

    if (h) {
        FAIL() << "Should be false";
    }

    auto h2 = emitter.once("once", [] {});
    EXPECT_TRUE(h2);
    emitter.emit("once");
    EXPECT_FALSE(h2);
}

TEST(TzEventEmitter, ScopedEventListenerBasic)
{
    TzEventEmitter emitter;
    int count = 0;

    {
        TzScopedEventListener sc1 = emitter.on("inc", [&] { ++count; });
        EXPECT_TRUE(sc1);
        emitter.emit("inc");
        EXPECT_EQ(count, 1);
    }

    emitter.emit("inc");
    EXPECT_EQ(count, 1);
}

TEST(TzEventEmitter, ScopedEventListenerMove)
{
    TzEventEmitter emitter;
    int count = 0;
    TzScopedEventListener sc1 = emitter.on("ev", [&] { ++count; });

    EXPECT_TRUE(sc1);
    TzScopedEventListener sc2(std::move(sc1));
    EXPECT_FALSE(sc1);
    EXPECT_TRUE(sc2);

    emitter.emit("ev");
    EXPECT_EQ(count, 1);

    TzScopedEventListener sc3;
    sc3 = std::move(sc2);
    EXPECT_FALSE(sc2);
    EXPECT_TRUE(sc3);

    emitter.emit("ev");
    EXPECT_EQ(count, 2);

    sc3 = std::move(sc3);
    EXPECT_TRUE(sc3);
}

TEST(TzEventEmitter, ScopedEventListenerRelease)
{
    TzEventEmitter emitter;
    TzEventListener raw;

    int count = 0;
    {
        TzScopedEventListener sc = emitter.on("ev", [&] { ++count; });
        EXPECT_TRUE(sc);
        raw = sc.release();
        EXPECT_FALSE(sc);
        EXPECT_TRUE(raw.isConnected());

        emitter.emit("ev");
        EXPECT_EQ(count, 1);
    }
    emitter.emit("ev");
    EXPECT_EQ(count, 2);

    raw.disconnect();
    emitter.emit("ev");
    EXPECT_EQ(count, 2);
}

TEST(TzEventEmitter, ScopedEventListenerSwap)
{
    TzEventEmitter emitter;
    int a = 0, b = 0;
    TzScopedEventListener scA = emitter.on("a", [&] { ++a; });
    TzScopedEventListener scB = emitter.on("b", [&] { ++b; });

    scA.swap(scB);

    emitter.emit("a");
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 0);

    emitter.emit("b");
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 1);

    EXPECT_TRUE(scA);
    EXPECT_TRUE(scB);
}

TEST(TzEventEmitter, ScopedEventListenerEmpty)
{
    TzScopedEventListener sc;
    EXPECT_FALSE(sc);
    TzEventListener raw = sc.release();
    EXPECT_FALSE(raw);

    TzScopedEventListener sc2(std::move(sc));
    EXPECT_FALSE(sc2);
}

class InheritedEmitter : public TzEventEmitter
{
public:
    InheritedEmitter()
    {
        m_handle = on("internal", [this](int val) {
            lastInternal = val;
        });
    }

    void triggerInternal(int val) { emit("internal", val); }
    int lastInternal = 0;

private:
    TzEventListener m_handle;
};

TEST(TzEventEmitter, InheritanceBasic)
{
    InheritedEmitter obj;
    int externalCounter = 0;

    auto h = obj.on("internal", [&externalCounter](int val) {
        externalCounter += val;
    });

    obj.triggerInternal(5);
    EXPECT_EQ(obj.lastInternal, 5);
    EXPECT_EQ(externalCounter, 5);

    obj.triggerInternal(3);
    EXPECT_EQ(obj.lastInternal, 3);
    EXPECT_EQ(externalCounter, 8);
}

TEST(TzEventEmitter, InheritanceScopedConnections)
{
    InheritedEmitter obj;
    int called = 0;
    {
        TzScopedEventListener sc = obj.on("internal", [&called](int) { ++called; });
        obj.triggerInternal(1);
        EXPECT_EQ(called, 1);
    }
    obj.triggerInternal(2);
    EXPECT_EQ(called, 1);
}

TEST(TzEventEmitter, InheritanceEmitFromDerived)
{
    class DataSource : public TzEventEmitter
    {
    public:
        void setData(int d)
        {
            data = d;
            emit("data_changed", data);
        }
        int data = 0;
    };

    DataSource ds;
    int received = 0;
    ds.on("data_changed", [&received](int newVal) { received = newVal; });

    ds.setData(42);
    EXPECT_EQ(received, 42);
    EXPECT_EQ(ds.data, 42);
}

TEST(TzEventEmitter, InheritanceSelfDisconnect)
{
    class SelfDisconnecting : public TzEventEmitter
    {
    public:
        SelfDisconnecting()
        {
            handle = on("boom", [this] {
                handle.disconnect();
            });
        }
        TzEventListener handle;
    };

    SelfDisconnecting obj;
    EXPECT_TRUE(obj.handle.isConnected());
    obj.emit("boom");
    EXPECT_FALSE(obj.handle.isConnected());
    EXPECT_NO_THROW(obj.emit("boom"));
}

TEST(TzEventEmitter, WeakPtrInCallback)
{
    TzEventEmitter bus;

    auto owner = std::make_shared<int>(77);
    int called = 0;

    bus.on("event", [weak = std::weak_ptr<int>(owner), &called](int x) {
        if (auto sp = weak.lock()) {
            *sp += x;
            ++called;
        }
    });

    bus.emit("event", 10);
    EXPECT_EQ(*owner, 87);
    EXPECT_EQ(called, 1);

    owner.reset();
    EXPECT_NO_THROW(bus.emit("event", 50));
    EXPECT_EQ(called, 1);
}
