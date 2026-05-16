#pragma once
#include <models/tztreemodel.h>
#include <tzabstractitemmodel_p.h>
#include <models/tzscopedeventlistener.h>

class TzTreeModelPrivate : public TzAbstractItemModelPrivate
{
public:
	TzTreeModelPrivate();

	TzTreeNodePtr root;

	struct NodeConnections {
		TzScopedEventListener onChildAdded;
		TzScopedEventListener onChildRemoved;
		TzScopedEventListener onDataChanged;
		TzScopedEventListener onNameChanged;
	};
	std::unordered_map<TzTreeNode*, NodeConnections> connections;
};
