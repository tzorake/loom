#ifndef TZABSTRACTLISTMODEL_HPP
#define TZABSTRACTLISTMODEL_HPP

#include <loom/tzabstractitemmodel.hpp>

class TzModelIndex;

class TzAbstractListModel : public TzAbstractItemModel
{
public:
    TzAbstractListModel();
    ~TzAbstractListModel();

    TzModelIndex index(int row, int column = 0, const TzModelIndex &parent = TzModelIndex()) const override;
    TzModelIndex sibling(int row, int column, const TzModelIndex &idx) const override;

    TzItemFlags flags(const TzModelIndex &index) const override;

protected:
    TzAbstractListModel(TzAbstractItemModelPrivate &dd);

private:
    TZ_DISABLE_COPY(TzAbstractListModel)
    TzModelIndex parent(const TzModelIndex &child) const override;
    int columnCount(const TzModelIndex &parent) const override;
    bool hasChildren(const TzModelIndex &parent) const override;
};

#endif // TZABSTRACTLISTMODEL_HPP
