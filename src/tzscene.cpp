#include <event-loop/tzscene.hpp>
#include <event-loop/tzwidget.hpp>
#include <event-loop/tzabstractwindow.hpp>
#include <event-loop/tzkeyevent.hpp>
#include <event-loop/tzmouseevent.hpp>
#include <event-loop/tzfocusevent.hpp>
#include <event-loop/tzcoreapplication.hpp>
#include <event-loop/tzpainter.hpp>

#include "tzwidget_p.hpp"

TzScene::TzScene(TzAbstractWindow *window)
    : m_window(window)
{
    window->setResizeCallback([this](int w, int h) { onResize(w, h); });
    window->setKeyCallback   ([this](TzKeyEvent *e) { onKey(e); });
    window->setMouseCallback ([this](TzMouseEvent *e) { onMouse(e); });
    // Close is intentionally left to the user (they set the close callback separately).
}

TzScene::~TzScene()
{
    if (m_root)
        unregisterSubtree(m_root);
}

// ── Root ──────────────────────────────────────────────────────────────────

TzWidget *TzScene::root() const { return m_root; }

void TzScene::setRoot(TzWidget *root)
{
    if (m_root)
        unregisterSubtree(m_root);

    m_root = root;

    if (m_root) {
        registerSubtree(m_root);
        // Set implicit size to current window dimensions so the root fills the window
        m_root->setImplicitSize((double)m_width, (double)m_height);
        m_root->resetWidth();
        m_root->resetHeight();
        m_root->setGeometry({ 0.0, 0.0, (double)m_width, (double)m_height });
        doLayout();
        markPaintDirty();
    }
}

// ── Layout ────────────────────────────────────────────────────────────────

void TzScene::doLayout()
{
    if (!m_root) return;

    // Root always fills the window
    m_root->setImplicitSize((double)m_width, (double)m_height);
    m_root->setGeometry({ 0.0, 0.0, (double)m_width, (double)m_height });

    // Resolve children with up to 3 passes to handle sibling-to-sibling anchors
    layoutWidget(m_root, 3);

    markPaintDirty();
}

void TzScene::layoutWidget(TzWidget *w, int passesLeft)
{
    // Resolve immediate children (multiple passes for sibling→sibling anchors)
    for (int pass = 0; pass < passesLeft; ++pass) {
        bool changed = false;
        TzObject *child = w->firstChild();
        while (child) {
            if (TzWidget *cw = dynamic_cast<TzWidget *>(child))
                if (cw->resolveAnchors()) changed = true;
            child = child->nextSibling();
        }
        if (!changed) break;
    }

    // Recurse
    TzObject *child = w->firstChild();
    while (child) {
        if (TzWidget *cw = dynamic_cast<TzWidget *>(child))
            layoutWidget(cw, passesLeft);
        child = child->nextSibling();
    }
}

// ── Painting ──────────────────────────────────────────────────────────────

void TzScene::doPaint()
{
    if (!m_root || m_width <= 0 || m_height <= 0) return;
    if (m_layoutDirty) { doLayout(); m_layoutDirty = false; }

    m_pixels.assign((size_t)(m_width * m_height), 0xFF000000u);

    TzRect windowRect = { 0.0, 0.0, (double)m_width, (double)m_height };
    paintWidget(m_root, 0.0, 0.0, windowRect);

    m_window->render(m_pixels, m_width, m_height);
    m_paintDirty = false;
}

void TzScene::paintWidget(TzWidget *w, double parentAbsX, double parentAbsY,
                           const TzRect &parentClip)
{
    if (!w->isVisible()) return;

    TzRect geom   = w->geometry();
    double absX   = parentAbsX + geom.x;
    double absY   = parentAbsY + geom.y;
    TzRect myClip = { absX, absY, geom.width, geom.height };
    TzRect clip   = intersected(myClip, parentClip);

    if (clip.isEmpty()) return;

    TzPainter painter(m_pixels.data(), m_width, m_height, clip, { absX, absY });
    w->paint(&painter);

    TzObject *child = w->firstChild();
    while (child) {
        if (TzWidget *cw = dynamic_cast<TzWidget *>(child))
            paintWidget(cw, absX, absY, clip);
        child = child->nextSibling();
    }
}

void TzScene::markPaintDirty()  { m_paintDirty  = true; }
bool TzScene::isPaintDirty()    const { return m_paintDirty; }
void TzScene::markLayoutDirty() { m_layoutDirty = true; m_paintDirty = true; }

// ── Focus ──────────────────────────────────────────────────────────────────

TzWidget *TzScene::focusedWidget() const { return m_focusedWidget; }

void TzScene::setFocusedWidget(TzWidget *widget)
{
    if (m_focusedWidget == widget) return;

    if (m_focusedWidget) {
        m_focusedWidget->d_ptr->focused = false;
        TzFocusEvent out(TzEvent::FocusOut);
        TzCoreApplication::sendEvent(m_focusedWidget, &out);
    }

    m_focusedWidget = widget;

    if (m_focusedWidget) {
        m_focusedWidget->d_ptr->focused = true;
        TzFocusEvent in(TzEvent::FocusIn);
        TzCoreApplication::sendEvent(m_focusedWidget, &in);
    }
}

// ── Hit-testing ───────────────────────────────────────────────────────────

TzWidget *TzScene::widgetAt(double x, double y) const
{
    if (!m_root) return nullptr;
    return widgetAtHelper(m_root, x, y, 0.0, 0.0);
}

TzWidget *TzScene::widgetAtHelper(TzWidget *w,
                                   double x, double y,
                                   double parentAbsX, double parentAbsY) const
{
    if (!w->isVisible()) return nullptr;

    TzRect geom = w->geometry();
    double absX = parentAbsX + geom.x;
    double absY = parentAbsY + geom.y;
    TzRect abs  = { absX, absY, geom.width, geom.height };

    if (!abs.contains(x, y)) return nullptr;

    // Check children in reverse order so topmost (last-painted) wins
    std::vector<TzObject *> children;
    TzObject *child = w->firstChild();
    while (child) { children.push_back(child); child = child->nextSibling(); }

    for (int i = (int)children.size() - 1; i >= 0; --i) {
        TzWidget *cw = dynamic_cast<TzWidget *>(children[i]);
        if (!cw) continue;
        TzWidget *hit = widgetAtHelper(cw, x, y, absX, absY);
        if (hit) return hit;
    }
    return w;
}

// ── Window size ───────────────────────────────────────────────────────────

int TzScene::width()  const { return m_width;  }
int TzScene::height() const { return m_height; }

// ── Window event handlers ─────────────────────────────────────────────────

void TzScene::onResize(int w, int h)
{
    m_width  = w;
    m_height = h;
    doLayout();
    doPaint();
}

void TzScene::onKey(TzKeyEvent *event)
{
    if (m_focusedWidget)
        TzCoreApplication::sendEvent(m_focusedWidget, event);
}

void TzScene::onMouse(TzMouseEvent *event)
{
    TzWidget *target = widgetAt(event->x(), event->y());
    if (!target) return;

    // On press, transfer focus to the clicked widget
    if (event->type() == TzEvent::MouseButtonPress && target != m_focusedWidget)
        setFocusedWidget(target);

    TzCoreApplication::sendEvent(target, event);
}

// ── Scene registration helpers ────────────────────────────────────────────

void TzScene::registerSubtree(TzWidget *w)
{
    w->d_ptr->scene = this;
    TzObject *child = w->firstChild();
    while (child) {
        if (TzWidget *cw = dynamic_cast<TzWidget *>(child))
            registerSubtree(cw);
        child = child->nextSibling();
    }
}

void TzScene::unregisterSubtree(TzWidget *w)
{
    w->d_ptr->scene = nullptr;
    if (m_focusedWidget == w) m_focusedWidget = nullptr;
    TzObject *child = w->firstChild();
    while (child) {
        if (TzWidget *cw = dynamic_cast<TzWidget *>(child))
            unregisterSubtree(cw);
        child = child->nextSibling();
    }
}
