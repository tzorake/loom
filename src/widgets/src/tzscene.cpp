#include <loom/tzcloseevent.hpp>
#include <loom/tzcoreapplication.hpp>
#include <loom/tzfocusevent.hpp>
#include <loom/tzkeyevent.hpp>
#include <loom/tzmouseevent.hpp>
#include <loom/tzresizeevent.hpp>
#include <loom/tzpainter.hpp>
#include <loom/tzpaintevent.hpp>
#include <loom/tzplatformsurface.hpp>
#include <loom/tzpoint.hpp>
#include <loom/tzrect.hpp>
#include <loom/tzscene.hpp>
#include <loom/tzsurface.hpp>
#include <loom/tzwidget.hpp>

#include "tzscene_p.hpp"
#include "tzwidget_p.hpp"

void TzScenePrivate::layoutWidget(TzWidget *w, int passesLeft)
{
    for (int pass = 0; pass < passesLeft; ++pass) {
        bool changed = false;
        TzObject *child = w->firstChild();
        while (child) {
            if (TzWidget *cw = dynamic_cast<TzWidget *>(child))
                if (static_cast<TzWidgetPrivate *>(cw->d_ptr.get())->resolveAnchors())
                    changed = true;
            child = child->nextSibling();
        }
        if (!changed)
            break;
    }

    TzObject *child = w->firstChild();
    while (child) {
        if (TzWidget *cw = dynamic_cast<TzWidget *>(child))
            layoutWidget(cw, passesLeft);
        child = child->nextSibling();
    }
}

void TzScenePrivate::paintWidget(TzWidget *w, double parentAbsX, double parentAbsY,
                                 const TzRect &parentClip)
{
    if (!w->isVisible())
        return;

    TzRect geom = w->geometry();
    double absX = parentAbsX + geom.x;
    double absY = parentAbsY + geom.y;
    TzRect myClip = {absX, absY, geom.width, geom.height};
    TzRect clip = myClip.intersected(parentClip);

    if (clip.isEmpty())
        return;

    TzPoint offset{absX, absY};
    TzPainter painter(pixels.data(), width, height, clip, offset);
    TzPaintEvent pe(&painter);
    TzCoreApplication::sendEvent(w, &pe);

    TzObject *child = w->firstChild();
    while (child) {
        if (TzWidget *cw = dynamic_cast<TzWidget *>(child))
            paintWidget(cw, absX, absY, clip);
        child = child->nextSibling();
    }
}

void TzScenePrivate::registerSubtree(TzWidget *w)
{
    TZ_Q(TzScene);
    static_cast<TzWidgetPrivate *>(w->d_ptr.get())->scene = q;
    TzObject *child = w->firstChild();
    while (child) {
        if (TzWidget *cw = dynamic_cast<TzWidget *>(child))
            registerSubtree(cw);
        child = child->nextSibling();
    }
}

void TzScenePrivate::unregisterSubtree(TzWidget *w)
{
    static_cast<TzWidgetPrivate *>(w->d_ptr.get())->scene = nullptr;
    if (focusedWidget == w)
        focusedWidget = nullptr;
    TzObject *child = w->firstChild();
    while (child) {
        if (TzWidget *cw = dynamic_cast<TzWidget *>(child))
            unregisterSubtree(cw);
        child = child->nextSibling();
    }
}

TzWidget *TzScenePrivate::widgetAtHelper(TzWidget *w, double x, double y, double parentAbsX,
                                         double parentAbsY) const
{
    if (!w->isVisible())
        return nullptr;

    TzRect geom = w->geometry();
    double absX = parentAbsX + geom.x;
    double absY = parentAbsY + geom.y;
    TzRect abs = {absX, absY, geom.width, geom.height};

    if (!abs.contains(x, y))
        return nullptr;

    std::vector<TzObject *> children;
    TzObject *child = w->firstChild();
    while (child) {
        children.push_back(child);
        child = child->nextSibling();
    }

    for (int i = (int) children.size() - 1; i >= 0; --i) {
        TzWidget *cw = dynamic_cast<TzWidget *>(children[i]);
        if (!cw)
            continue;
        TzWidget *hit = widgetAtHelper(cw, x, y, absX, absY);
        if (hit)
            return hit;
    }
    return w;
}

TzScene::TzScene(TzSurface *surface)
    : d_ptr(new TzScenePrivate)
{
    TZ_D(TzScene);
    d->q_ptr = this;
    d->platformSurface = surface->surfaceHandle();
}

void TzScene::dispatchCloseEvent(TzCloseEvent *e)
{
    TZ_D(TzScene);
    if (d->root)
        TzCoreApplication::sendEvent(d->root, e);
}

void TzScene::dispatchKeyEvent(TzKeyEvent *e)
{
    TZ_D(TzScene);
    if (d->focusedWidget)
        TzCoreApplication::sendEvent(d->focusedWidget, e);
}

void TzScene::dispatchMouseEvent(TzMouseEvent *e)
{
    TzWidget *target = widgetAt(e->x(), e->y());
    if (!target)
        return;
    if (e->type() == TzEvent::MouseButtonPress && target != focusedWidget())
        setFocusedWidget(target);
    TzPoint local = target->mapFromGlobal({e->x(), e->y()});
    TzMouseEvent localEvent(e->type(), e->button(), local.x, local.y, e->modifiers(),
                            e->scrollDx(), e->scrollDy());
    TzCoreApplication::sendEvent(target, &localEvent);
}

void TzScene::dispatchResizeEvent(TzResizeEvent *e)
{
    TZ_D(TzScene);
    d->width = e->width();
    d->height = e->height();
    doLayout();
    doPaint();
}

TzScene::~TzScene()
{
    TZ_D(TzScene);
    if (d->root)
        d->unregisterSubtree(d->root);
}

TzWidget *TzScene::root() const
{
    TZ_D(const TzScene);
    return d->root;
}

void TzScene::setRoot(TzWidget *root)
{
    TZ_D(TzScene);
    if (d->root)
        d->unregisterSubtree(d->root);

    d->root = root;

    if (d->root) {
        d->registerSubtree(d->root);
        d->root->setImplicitSize((double) d->width, (double) d->height);

        TzWidgetPrivate *rd = static_cast<TzWidgetPrivate *>(d->root->d_ptr.get());
        rd->resetWidth();
        rd->resetHeight();

        d->root->setGeometry({0.0, 0.0, (double) d->width, (double) d->height});
        doLayout();
        markPaintDirty();
    }
}

void TzScene::doLayout()
{
    TZ_D(TzScene);
    if (!d->root)
        return;

    d->root->setImplicitSize((double) d->width, (double) d->height);
    d->root->setGeometry({0.0, 0.0, (double) d->width, (double) d->height});

    d->layoutWidget(d->root, 3);

    markPaintDirty();
}

void TzScene::doPaint()
{
    TZ_D(TzScene);
    if (!d->root || d->width <= 0 || d->height <= 0)
        return;
    if (d->layoutDirty) {
        doLayout();
        d->layoutDirty = false;
    }

    d->pixels.assign((size_t) (d->width * d->height), 0xFF000000u);

    TzRect windowRect = {0.0, 0.0, (double) d->width, (double) d->height};
    d->paintWidget(d->root, 0.0, 0.0, windowRect);

    d->platformSurface->render(d->pixels, d->width, d->height);
    d->paintDirty = false;
}

void TzScene::markPaintDirty()
{
    TZ_D(TzScene);
    d->paintDirty = true;
}

bool TzScene::isPaintDirty() const
{
    TZ_D(const TzScene);
    return d->paintDirty;
}

void TzScene::markLayoutDirty()
{
    TZ_D(TzScene);
    d->layoutDirty = true;
    d->paintDirty = true;
}

TzWidget *TzScene::focusedWidget() const
{
    TZ_D(const TzScene);
    return d->focusedWidget;
}

void TzScene::setFocusedWidget(TzWidget *widget)
{
    TZ_D(TzScene);
    if (d->focusedWidget == widget)
        return;

    if (d->focusedWidget) {
        static_cast<TzWidgetPrivate *>(d->focusedWidget->d_ptr.get())->focused = false;
        TzFocusEvent out(TzEvent::FocusOut);
        TzCoreApplication::sendEvent(d->focusedWidget, &out);
    }

    d->focusedWidget = widget;

    if (d->focusedWidget) {
        static_cast<TzWidgetPrivate *>(d->focusedWidget->d_ptr.get())->focused = true;
        TzFocusEvent in(TzEvent::FocusIn);
        TzCoreApplication::sendEvent(d->focusedWidget, &in);
    }
}

TzWidget *TzScene::widgetAt(double x, double y) const
{
    TZ_D(const TzScene);
    if (!d->root)
        return nullptr;
    return d->widgetAtHelper(d->root, x, y, 0.0, 0.0);
}

int TzScene::width() const
{
    TZ_D(const TzScene);
    return d->width;
}

int TzScene::height() const
{
    TZ_D(const TzScene);
    return d->height;
}
