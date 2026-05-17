#include "tzbreadcrumbmodel.hpp"
#include <algorithm>
#include <ranges>

TzBreadcrumbModel::TzBreadcrumbModel(const TzTreeNodePtr &root)
    : m_root(root)
    , m_current(root)
{}

TzTreeNodePtr TzBreadcrumbModel::root() const
{
    return m_root;
}

TzTreeNodePtr TzBreadcrumbModel::currentNode() const
{
    return m_current;
}

std::vector<TzTreeNodePtr> TzBreadcrumbModel::path() const
{
    std::vector<TzTreeNodePtr> result;
    for (TzTreeNodePtr n = m_current; n; n = n->parent())
        result.push_back(n);
    std::ranges::reverse(result);
    return result;
}

std::vector<TzTreeNodePtr> TzBreadcrumbModel::siblingsOf(const TzTreeNodePtr &node) const
{
    if (!node)
        return {};

    TzTreeNodePtr parent = node->parent();
    if (!parent)
        return {};

    return {parent->children().begin(), parent->children().end()};
}

bool TzBreadcrumbModel::navigateTo(const TzTreeNodePtr &node)
{
    if (!node || !isReachable(node))
        return false;

    TzTreeNodePtr old = m_current;
    m_current = node;
    m_events.emit("currentNodeChanged", m_current, old);
    return true;
}

bool TzBreadcrumbModel::navigateUp()
{
    if (!m_current || m_current == m_root)
        return false;

    return navigateTo(m_current->parent());
}

void TzBreadcrumbModel::navigateToRoot()
{
    if (m_current != m_root)
        navigateTo(m_root);
}

TzEventEmitter &TzBreadcrumbModel::events()
{
    return m_events;
}

const TzEventEmitter &TzBreadcrumbModel::events() const
{
    return m_events;
}

bool TzBreadcrumbModel::isReachable(const TzTreeNodePtr &node) const
{
    for (TzTreeNodePtr n = node; n; n = n->parent()) {
        if (n == m_root)
            return true;
    }
    return false;
}
