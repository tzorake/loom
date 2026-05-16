#include <models/tztreenode.h>
#include <models/tzeventemitter.h>

class TzTreeNodePrivate
{
public:
    TzTreeNodePrivate(const std::string& name, TzTreeNode::Type type);

    static TzTreeNodePrivate* get(TzTreeNode& q);
    static const TzTreeNodePrivate* get(const TzTreeNode& q);

    std::string name;
    TzTreeNode::Type type;
    std::any data;
    std::weak_ptr<TzTreeNode> parent;
    std::vector<TzTreeNodePtr> children;
    TzEventEmitter events;
};

TzTreeNodePrivate::TzTreeNodePrivate(const std::string& name, TzTreeNode::Type type)
    : name(name)
    , type(type)
{
}

TzTreeNodePrivate* TzTreeNodePrivate::get(TzTreeNode& q)
{
    return q.d_ptr.get();
}

const TzTreeNodePrivate* TzTreeNodePrivate::get(const TzTreeNode& q)
{
    return q.d_ptr.get();
}

TzTreeNodePtr TzTreeNode::create(const std::string& name, Type type)
{
    return std::make_shared<TzTreeNode>(name, type, PrivateToken{});
}

TzTreeNode::TzTreeNode(const std::string& name, Type type, PrivateToken)
    : d_ptr(new TzTreeNodePrivate(name, type))
{
}

bool TzTreeNode::setName(const std::string& newName)
{
    TZ_D(TzTreeNode);
    if (d->name == newName)
        return false;
    d->name = newName;
    d->events.emit("nameChanged", d->name);
    return true;
}

const std::string& TzTreeNode::name() const
{
    TZ_D(const TzTreeNode);
    return d->name;
}

TzTreeNode::Type TzTreeNode::type() const
{
    TZ_D(const TzTreeNode);
    return d->type;
}

bool TzTreeNode::isFolder() const
{
    TZ_D(const TzTreeNode);
    return d->type == TzTreeNode::Type::Folder;
}

bool TzTreeNode::isItem() const
{
    TZ_D(const TzTreeNode);
    return d->type == TzTreeNode::Type::Item;
}

TzTreeNodePtr TzTreeNode::parent() const
{
    TZ_D(const TzTreeNode);
    return d->parent.lock();
}

int TzTreeNode::childCount() const
{
    TZ_D(const TzTreeNode);
    return static_cast<int>(d->children.size());
}

const std::vector<TzTreeNodePtr>& TzTreeNode::children() const
{
    TZ_D(const TzTreeNode);
    return d->children;
}

TzTreeNodePtr TzTreeNode::childAt(int index) const
{
    TZ_D(const TzTreeNode);
    if (index < 0 || static_cast<std::size_t>(index) >= d->children.size())
        return nullptr;
    return d->children[static_cast<std::size_t>(index)];
}

int TzTreeNode::indexOf(const TzTreeNodePtr& child) const
{
    TZ_D(const TzTreeNode);
    auto it = std::ranges::find(d->children, child);
    return it != d->children.end() ? static_cast<int>(std::distance(d->children.begin(), it)) : -1;
}

bool TzTreeNode::addChild(const TzTreeNodePtr& child)
{
    TZ_D(TzTreeNode);
    if (!child)
        return false;
    d->children.push_back(child);
    TzTreeNodePrivate::get(*child)->parent = weak_from_this();
    d->events.emit("childAdded", child, static_cast<int>(d->children.size() - 1));
    return true;
}

bool TzTreeNode::insertChild(int index, const TzTreeNodePtr& child)
{
    TZ_D(TzTreeNode);
    if (!child || index < 0 || index > static_cast<int>(d->children.size()))
        return false;
    d->children.insert(d->children.begin() + index, child);
    TzTreeNodePrivate::get(*child)->parent = weak_from_this();
    d->events.emit("childAdded", child, index);
    return true;
}

bool TzTreeNode::removeChild(const TzTreeNodePtr& child)
{
    TZ_D(TzTreeNode);
    auto it = std::ranges::find(d->children, child);
    if (it == d->children.end())
        return false;
    int index = static_cast<int>(std::distance(d->children.begin(), it));
    d->events.emit("childAboutToBeRemoved", child, index);
    d->children.erase(it);
    TzTreeNodePrivate::get(*child)->parent.reset();
    return true;
}

void TzTreeNode::setData(const std::any& value)
{
    TZ_D(TzTreeNode);
    d->data = value;
    d->events.emit("dataChanged");
}

const std::any& TzTreeNode::data() const
{
    TZ_D(const TzTreeNode);
    return d->data;
}

TzEventEmitter& TzTreeNode::events()
{
    TZ_D(TzTreeNode);
    return d->events;
}

const TzEventEmitter& TzTreeNode::events() const
{
    TZ_D(const TzTreeNode);
    return d->events;
}

TzTreeNodePtr TzTreeNode::createFolder(const std::string& name)
{
    return TzTreeNode::create(name, TzTreeNode::Folder);
}

TzTreeNodePtr TzTreeNode::createItem(const std::string& name)
{
    return TzTreeNode::create(name, TzTreeNode::Item);
}
