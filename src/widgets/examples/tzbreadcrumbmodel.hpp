#pragma once
#include "tztreenode.hpp"
#include <loom/tzeventemitter.hpp>
#include <vector>
#include <algorithm>
#include <ranges>

class TzBreadcrumbModel
{
public:
    explicit TzBreadcrumbModel(const TzTreeNodePtr &root);

    TzTreeNodePtr root() const;
    TzTreeNodePtr currentNode() const;

    std::vector<TzTreeNodePtr> path() const;
    std::vector<TzTreeNodePtr> siblingsOf(const TzTreeNodePtr &node) const;

    bool navigateTo(const TzTreeNodePtr &node);
    bool navigateUp();
    void navigateToRoot();

    TzEventEmitter &events();
    const TzEventEmitter &events() const;

private:
    bool isReachable(const TzTreeNodePtr &node) const;

    TzTreeNodePtr m_root;
    TzTreeNodePtr m_current;
    TzEventEmitter m_events;
};

inline TzBreadcrumbModel::TzBreadcrumbModel(const TzTreeNodePtr &root)
    : m_root(root)
    , m_current(root)
{
}

inline TzTreeNodePtr TzBreadcrumbModel::root() const
{
    return m_root;
}

inline TzTreeNodePtr TzBreadcrumbModel::currentNode() const
{
    return m_current;
}

inline std::vector<TzTreeNodePtr> TzBreadcrumbModel::path() const
{
    std::vector<TzTreeNodePtr> result;
    for (TzTreeNodePtr n = m_current; n; n = n->parent())
        result.push_back(n);
    std::ranges::reverse(result);
    return result;
}

inline std::vector<TzTreeNodePtr> TzBreadcrumbModel::siblingsOf(const TzTreeNodePtr &node) const
{
    if (!node)
        return {};

    TzTreeNodePtr parent = node->parent();
    if (!parent)
        return {};

    return {parent->children().begin(), parent->children().end()};
}

inline bool TzBreadcrumbModel::navigateTo(const TzTreeNodePtr &node)
{
    if (!node || !isReachable(node))
        return false;

    TzTreeNodePtr old = m_current;
    m_current = node;
    m_events.emit("currentNodeChanged", m_current, old);
    return true;
}

inline bool TzBreadcrumbModel::navigateUp()
{
    if (!m_current || m_current == m_root)
        return false;

    return navigateTo(m_current->parent());
}

inline void TzBreadcrumbModel::navigateToRoot()
{
    if (m_current != m_root)
        navigateTo(m_root);
}

inline TzEventEmitter &TzBreadcrumbModel::events()
{
    return m_events;
}

inline const TzEventEmitter &TzBreadcrumbModel::events() const
{
    return m_events;
}

inline bool TzBreadcrumbModel::isReachable(const TzTreeNodePtr &node) const
{
    for (TzTreeNodePtr n = node; n; n = n->parent()) {
        if (n == m_root)
            return true;
    }
    return false;
}
