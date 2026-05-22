#ifndef TZITEMSELECTIONMODEL_HPP
#define TZITEMSELECTIONMODEL_HPP

#include <loom/tzabstractitemmodel.hpp>
#include <loom/tzeventemitter.hpp>

class TzItemSelectionRange
{
public:
    TzItemSelectionRange() = default;
    TzItemSelectionRange(const TzModelIndex &topL, const TzModelIndex &bottomR) : tl(topL), br(bottomR) {}
    explicit TzItemSelectionRange(const TzModelIndex &index) : tl(index), br(tl) {}

    void swap(TzItemSelectionRange &other) noexcept
    {
        tl.swap(other.tl);
        br.swap(other.br);
    }

    inline int top() const { return tl.row(); }
    inline int left() const { return tl.column(); }
    inline int bottom() const { return br.row(); }
    inline int right() const { return br.column(); }
    inline int width() const { return br.column() - tl.column() + 1; }
    inline int height() const { return br.row() - tl.row() + 1; }

    inline const TzPersistentModelIndex &topLeft() const { return tl; }
    inline const TzPersistentModelIndex &bottomRight() const { return br; }
    inline TzModelIndex parent() const { return tl.parent(); }
    inline const TzAbstractItemModel *model() const { return tl.model(); }

    inline bool contains(const TzModelIndex &index) const
    { return contains(index.row(), index.column(), index.parent()); }

    inline bool contains(int row, int column, const TzModelIndex &parentIndex) const
    {
        return (br.row() >= row && br.column() >= column &&
                tl.row() <= row && tl.column() <= column &&
                parent() == parentIndex);
    }

    bool intersects(const TzItemSelectionRange &other) const;
    TzItemSelectionRange intersected(const TzItemSelectionRange &other) const;

    inline bool operator==(const TzItemSelectionRange &other) const noexcept
    { return tl == other.tl && br == other.br; }

    inline bool operator!=(const TzItemSelectionRange &other) const noexcept
    { return !operator==(other); }

    inline bool isValid() const
    {
        return (tl.isValid() && br.isValid() && tl.parent() == br.parent()
                && top() <= bottom() && left() <= right());
    }

    bool isEmpty() const;
    TzModelIndexList indexes() const;

private:
    TzPersistentModelIndex tl, br;
};

class TzItemSelection;
class TzItemSelectionModelPrivate;

class TzItemSelectionModel
{
    TZ_DECLARE_PRIVATE(TzItemSelectionModel)
public:
    enum SelectionFlag {
        NoUpdate       = 0x0000,
        Clear          = 0x0001,
        Select         = 0x0002,
        Deselect       = 0x0004,
        Toggle         = 0x0008,
        Current        = 0x0010,
        Rows           = 0x0020,
        Columns        = 0x0040,
        SelectCurrent  = Select | Current,
        ToggleCurrent  = Toggle | Current,
        ClearAndSelect = Clear | Select
    };

    TZ_DECLARE_FLAGS(SelectionFlags, SelectionFlag)

    explicit TzItemSelectionModel(TzAbstractItemModel *model = nullptr);
    virtual ~TzItemSelectionModel();

    TzModelIndex currentIndex() const;

    bool isSelected(const TzModelIndex &index) const;
    bool isRowSelected(int row, const TzModelIndex &parent = TzModelIndex()) const;
    bool isColumnSelected(int column, const TzModelIndex &parent = TzModelIndex()) const;

    bool rowIntersectsSelection(int row, const TzModelIndex &parent = TzModelIndex()) const;
    bool columnIntersectsSelection(int column, const TzModelIndex &parent = TzModelIndex()) const;

    bool hasSelection() const;

    TzModelIndexList selectedIndexes() const;
    TzModelIndexList selectedRows(int column = 0) const;
    TzModelIndexList selectedColumns(int row = 0) const;
    const TzItemSelection selection() const;

    TzEventEmitter &events();
    const TzEventEmitter &events() const;

    void setModel(TzAbstractItemModel *model);
    TzAbstractItemModel *model();
    const TzAbstractItemModel *model() const;

    virtual void setCurrentIndex(const TzModelIndex &index, TzItemSelectionModel::SelectionFlags command);
    virtual void select(const TzModelIndex &index, TzItemSelectionModel::SelectionFlags command);
    virtual void select(const TzItemSelection &selection, TzItemSelectionModel::SelectionFlags command);
    virtual void clear();
    virtual void reset();

    void clearSelection();
    virtual void clearCurrentIndex();

// TZ_SIGNALS.begin — subscribe via selectionModel.emitter.on("selectionChanged", cb)
private:
    void selectionChanged(const TzItemSelection &selected, const TzItemSelection &deselected);
    void currentChanged(const TzModelIndex &current, const TzModelIndex &previous);
    void currentRowChanged(const TzModelIndex &current, const TzModelIndex &previous);
    void currentColumnChanged(const TzModelIndex &current, const TzModelIndex &previous);
    void modelChanged(TzAbstractItemModel *model);
// TZ_SIGNALS.end

protected:
    TzItemSelectionModel(TzItemSelectionModelPrivate &dd, TzAbstractItemModel *model);
    void emitSelectionChanged(const TzItemSelection &newSelection, const TzItemSelection &oldSelection);

private:
    TzItemSelectionModelPrivate *d_ptr;
    TZ_DISABLE_COPY(TzItemSelectionModel)
};

TZ_DECLARE_OPERATORS_FOR_FLAGS(TzItemSelectionModel::SelectionFlags)

class TzItemSelection : public std::vector<TzItemSelectionRange>
{
public:
    using std::vector<TzItemSelectionRange>::vector;
    TzItemSelection(const TzModelIndex &topLeft, const TzModelIndex &bottomRight);

    TzItemSelection &operator<<(const TzItemSelectionRange &range)
    { push_back(range); return *this; }

    void select(const TzModelIndex &topLeft, const TzModelIndex &bottomRight);
    bool contains(const TzModelIndex &index) const;
    TzModelIndexList indexes() const;
    bool isEmpty() const { return empty(); }
    void merge(const TzItemSelection &other, TzItemSelectionModel::SelectionFlags command);
    static void split(const TzItemSelectionRange &range,
                      const TzItemSelectionRange &other,
                      TzItemSelection *result);
};

#endif // TZITEMSELECTIONMODEL_HPP
