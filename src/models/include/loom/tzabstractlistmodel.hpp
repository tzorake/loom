#pragma once
#include <loom/tzabstractitemmodel.hpp>

class TzAbstractItemModelPrivate;

class TzAbstractListModel : public TzAbstractItemModel
{
public:
    TzAbstractListModel();
    ~TzAbstractListModel() override;

    TzModelIndex index(int row, int column, const TzModelIndex& parent = TzModelIndex()) const final override;
    TzModelIndex parent(const TzModelIndex& child) const final override;
    int columnCount(const TzModelIndex& parent = TzModelIndex()) const final override;
    bool hasChildren(const TzModelIndex& parent = TzModelIndex()) const override;

protected:
    TzAbstractListModel(TzAbstractItemModelPrivate& dd);
};
