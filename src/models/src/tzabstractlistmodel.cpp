#include <loom/tzabstractlistmodel.hpp>
#include <tzabstractitemmodel_p.hpp>

TzAbstractListModel::TzAbstractListModel()
    : TzAbstractItemModel()
{
}

TzAbstractListModel::TzAbstractListModel(TzAbstractItemModelPrivate& dd)
    : TzAbstractItemModel(dd)
{
}

TzAbstractListModel::~TzAbstractListModel() = default;

TzModelIndex TzAbstractListModel::index(int row, int column, const TzModelIndex& parent) const
{
    if (parent.isValid())
        return TzModelIndex();
    if (!hasIndex(row, column, parent))
        return TzModelIndex();
    return createIndex(row, column);
}

TzModelIndex TzAbstractListModel::parent(const TzModelIndex& /*child*/) const
{
    return TzModelIndex();
}

int TzAbstractListModel::columnCount(const TzModelIndex& parent) const
{
    return parent.isValid() ? 0 : 1;
}

bool TzAbstractListModel::hasChildren(const TzModelIndex& parent) const
{
    if (parent.isValid())
        return false;
    return rowCount() > 0;
}
