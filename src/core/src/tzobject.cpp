#include <loom/tzobject.hpp>
#include <tzobject_p.hpp>
#include <loom/tzglobal.hpp>
#include <loom/tzcoreapplication.hpp>
#include <loom/tzdeferreddeleteevent.hpp>

TzObjectPrivate::TzObjectPrivate()
{
}

TzObjectPrivate::~TzObjectPrivate()
{
}

void TzObjectPrivate::unlinkFromParent()
{
    if (!parent)
        return;

    TzObjectPrivate *pd = parent->d_ptr.get();
    if (previousSibling)
        previousSibling->d_ptr->nextSibling = nextSibling;
    else
        pd->firstChild = nextSibling;

    if (nextSibling)
        nextSibling->d_ptr->previousSibling = previousSibling;

    parent = nullptr;
    previousSibling = nullptr;
    nextSibling = nullptr;
}

void TzObjectPrivate::appendToParent(TzObject *newParent)
{
    TzObjectPrivate *pd = newParent->d_ptr.get();
    parent = newParent;

    if (!pd->firstChild) {
        pd->firstChild = q_ptr;
        return;
    }

    TzObject *last = pd->firstChild;
    while (last->d_ptr->nextSibling)
        last = last->d_ptr->nextSibling;

    last->d_ptr->nextSibling = q_ptr;
    previousSibling = last;
}

TzObject::TzObject(TzObject *parent)
    : TzObject(*new TzObjectPrivate, parent)
{}

TzObject::TzObject(TzObjectPrivate &dd, TzObject *parent)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
    if (parent)
        d_ptr->appendToParent(parent);
}

TzObject::~TzObject()
{
    if (TzCoreApplication *app = tzApp)
        tzApp->removePostedEvents(this);

    d_ptr->unlinkFromParent();

    TzObject *child = d_ptr->firstChild;
    while (child) {
        TzObject *next = child->d_ptr->nextSibling;
        child->d_ptr->parent = nullptr;
        child->d_ptr->previousSibling = nullptr;
        child->d_ptr->nextSibling = nullptr;
        delete child;
        child = next;
    }
    d_ptr->firstChild = nullptr;
}

void TzObject::setParent(TzObject *parent)
{
    if (d_ptr->parent == parent)
        return;
    d_ptr->unlinkFromParent();
    if (parent)
        d_ptr->appendToParent(parent);
}

TzObject *TzObject::parent() const
{
    return d_ptr->parent;
}

TzObject *TzObject::firstChild() const
{
    return d_ptr->firstChild;
}

TzObject *TzObject::nextSibling() const
{
    return d_ptr->nextSibling;
}

TzObject *TzObject::previousSibling() const
{
    return d_ptr->previousSibling;
}

std::vector<TzObject *> TzObject::children() const
{
    std::vector<TzObject *> result;
    for (TzObject *c = d_ptr->firstChild; c; c = c->d_ptr->nextSibling)
        result.push_back(c);
    return result;
}

bool TzObject::event(TzEvent *event)
{
    TZ_UNUSED(event);
    return false;
}

void TzObject::deleteLater()
{
    if (d_ptr->pendingDelete)
        return;
    d_ptr->pendingDelete = true;
    TzCoreApplication::postEvent(this, new TzDeferredDeleteEvent());
}
