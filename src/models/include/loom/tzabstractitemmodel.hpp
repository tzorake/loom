#ifndef TZABSTRACTITEMMODEL_HPP
#define TZABSTRACTITEMMODEL_HPP

#include <loom/tzflags.hpp>
#include <loom/tzglobal.hpp>
#include <loom/tzhash.hpp>
#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzeventemitter.hpp>

#include <any>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum TzItemDataRole {
    DisplayRole = 0,
    EditRole = 2,
    UserRole = 0x0100
};

enum TzItemFlag {
    NoItemFlags = 0,
    ItemIsSelectable = 1,
    ItemIsEditable = 2,
    ItemIsDragEnabled = 4,
    ItemIsDropEnabled = 8,
    ItemIsUserCheckable = 16,
    ItemIsEnabled = 32,
    ItemIsAutoTristate = 64,
    ItemNeverHasChildren = 128,
    ItemIsUserTristate = 256
};
TZ_DECLARE_FLAGS(TzItemFlags, TzItemFlag)
TZ_DECLARE_OPERATORS_FOR_FLAGS(TzItemFlags)

class TzAbstractItemModel;
class TzPersistentModelIndex;

class TzModelIndex
{
    friend class TzAbstractItemModel;
public:
    constexpr inline TzModelIndex() noexcept : r(-1), c(-1), i(0), m(nullptr) {}
    constexpr inline int row() const noexcept { return r; }
    constexpr inline int column() const noexcept { return c; }
    constexpr inline uintptr_t internalId() const noexcept { return i; }
    inline void *internalPointer() const noexcept { return reinterpret_cast<void*>(i); }
    inline const void *constInternalPointer() const noexcept { return reinterpret_cast<const void *>(i); }
    inline TzModelIndex parent() const;
    inline TzModelIndex sibling(int row, int column) const;
    inline TzModelIndex siblingAtColumn(int column) const;
    inline TzModelIndex siblingAtRow(int row) const;
    inline std::any data(int role = TzItemDataRole::DisplayRole) const;
    inline TzItemFlags flags() const;
    constexpr inline const TzAbstractItemModel *model() const noexcept { return m; }
    constexpr inline bool isValid() const noexcept { return (r >= 0) && (c >= 0) && (m != nullptr); }

    constexpr inline bool operator==(const TzModelIndex &other) const noexcept
    { return r == other.r && c == other.c && i == other.i && m == other.m; }
    constexpr inline bool operator!=(const TzModelIndex &other) const noexcept
    { return !operator==(other); }
    constexpr inline bool operator<(const TzModelIndex &other) const noexcept
    {
        if (m != other.m) return m < other.m;
        if (r != other.r) return r < other.r;
        if (c != other.c) return c < other.c;
        return i < other.i;
    }

private:
    inline TzModelIndex(int arow, int acolumn, const void *ptr, const TzAbstractItemModel *amodel) noexcept
        : r(arow), c(acolumn), i(reinterpret_cast<uintptr_t>(ptr)), m(amodel) {}
    constexpr inline TzModelIndex(int arow, int acolumn, uintptr_t id, const TzAbstractItemModel *amodel) noexcept
        : r(arow), c(acolumn), i(id), m(amodel) {}
    int r, c;
    uintptr_t i;
    const TzAbstractItemModel *m;
};

namespace std {
template<>
struct hash<TzModelIndex> {
    size_t operator()(const TzModelIndex &index) const noexcept {
        return tzHashMulti(0, index.row(), index.column(), index.internalId());
    }
};
} // namespace std

class TzPersistentModelIndexData;

class TzPersistentModelIndex
{
public:
    TzPersistentModelIndex();
    TzPersistentModelIndex(const TzModelIndex &index);
    TzPersistentModelIndex(const TzPersistentModelIndex &other);
    ~TzPersistentModelIndex();

    bool operator<(const TzPersistentModelIndex &other) const noexcept;
    bool operator==(const TzPersistentModelIndex &other) const noexcept;
    inline bool operator!=(const TzPersistentModelIndex &other) const noexcept
    { return !operator==(other); }

    TzPersistentModelIndex &operator=(const TzPersistentModelIndex &other);
    inline TzPersistentModelIndex(TzPersistentModelIndex &&other) noexcept
        : d(std::exchange(other.d, nullptr)) {}
    TzPersistentModelIndex &operator=(TzPersistentModelIndex &&other) noexcept
    { swap(other); return *this; }
    void swap(TzPersistentModelIndex &other) noexcept { tzPtrSwap(d, other.d); }
    TzPersistentModelIndex &operator=(const TzModelIndex &other);
    operator TzModelIndex() const;
    int row() const;
    int column() const;
    void *internalPointer() const;
    const void *constInternalPointer() const;
    uintptr_t internalId() const;
    TzModelIndex parent() const;
    TzModelIndex sibling(int row, int column) const;
    std::any data(int role = TzItemDataRole::DisplayRole) const;
    TzItemFlags flags() const;
    const TzAbstractItemModel *model() const;
    bool isValid() const;
private:
    TzPersistentModelIndexData *d;
    friend class std::hash<TzPersistentModelIndex>;
};

namespace std {
template<>
struct hash<TzPersistentModelIndex> {
    size_t operator()(const TzPersistentModelIndex &index) const noexcept {
        return tzHash(index.d, 0);
    }
};
} // namespace std

using TzModelIndexList = std::vector<TzModelIndex>;

class TzAbstractItemModelPrivate;

class TzAbstractItemModel
{
    friend class TzPersistentModelIndexData;
    friend class TzAbstractProxyModel;
public:
    TzAbstractItemModel();
    virtual ~TzAbstractItemModel();

    bool hasIndex(int row, int column, const TzModelIndex &parent = TzModelIndex()) const;
    virtual TzModelIndex index(int row, int column,
                              const TzModelIndex &parent = TzModelIndex()) const = 0;
    virtual TzModelIndex parent(const TzModelIndex &child) const = 0;

    virtual TzModelIndex sibling(int row, int column, const TzModelIndex &idx) const;
    virtual int rowCount(const TzModelIndex &parent = TzModelIndex()) const = 0;
    virtual int columnCount(const TzModelIndex &parent = TzModelIndex()) const = 0;
    virtual bool hasChildren(const TzModelIndex &parent = TzModelIndex()) const;

    virtual std::any data(const TzModelIndex &index, int role = TzItemDataRole::DisplayRole) const = 0;
    virtual bool setData(const TzModelIndex &index, const std::any &value, int role = TzItemDataRole::EditRole);

    virtual std::any headerData(int section, TzOrientation orientation,
                                int role = TzItemDataRole::DisplayRole) const;
    virtual bool setHeaderData(int section, TzOrientation orientation, const std::any &value,
                               int role = TzItemDataRole::EditRole);

    virtual bool insertRows(int row, int count, const TzModelIndex &parent = TzModelIndex());
    virtual bool insertColumns(int column, int count, const TzModelIndex &parent = TzModelIndex());
    virtual bool removeRows(int row, int count, const TzModelIndex &parent = TzModelIndex());
    virtual bool removeColumns(int column, int count, const TzModelIndex &parent = TzModelIndex());
    virtual bool moveRows(const TzModelIndex &sourceParent, int sourceRow, int count,
                          const TzModelIndex &destinationParent, int destinationChild);
    virtual bool moveColumns(const TzModelIndex &sourceParent, int sourceColumn, int count,
                             const TzModelIndex &destinationParent, int destinationChild);

    inline bool insertRow(int row, const TzModelIndex &parent = TzModelIndex());
    inline bool insertColumn(int column, const TzModelIndex &parent = TzModelIndex());
    inline bool removeRow(int row, const TzModelIndex &parent = TzModelIndex());
    inline bool removeColumn(int column, const TzModelIndex &parent = TzModelIndex());
    inline bool moveRow(const TzModelIndex &sourceParent, int sourceRow,
                        const TzModelIndex &destinationParent, int destinationChild);
    inline bool moveColumn(const TzModelIndex &sourceParent, int sourceColumn,
                           const TzModelIndex &destinationParent, int destinationChild);

    virtual void fetchMore(const TzModelIndex &parent);
    virtual bool canFetchMore(const TzModelIndex &parent) const;
    virtual TzItemFlags flags(const TzModelIndex &index) const;
    virtual void sort(int column, TzSortOrder order = TzSortOrder::AscendingOrder);
    virtual TzModelIndex buddy(const TzModelIndex &index) const;

    virtual std::unordered_map<int, std::string> roleNames() const;

    enum class LayoutChangeHint {
        NoLayoutChangeHint,
        VerticalSortHint,
        HorizontalSortHint
    };

    enum class CheckIndexOption {
        NoOption = 0x0000,
        IndexIsValid = 0x0001,
        DoNotUseParent = 0x0002,
        ParentIsInvalid = 0x0004,
    };
    TZ_DECLARE_FLAGS(CheckIndexOptions, CheckIndexOption)

    [[nodiscard]] bool checkIndex(const TzModelIndex &index, CheckIndexOptions options = CheckIndexOption::NoOption) const;

    TzEventEmitter &events();
    const TzEventEmitter &events() const;

// TZ_SIGNALS.begin — call these to notify subscribers; subscribe via model.emitter.on("signalName", cb)
public:
    void dataChanged(const TzModelIndex &topLeft, const TzModelIndex &bottomRight, const std::vector<int> &roles = std::vector<int>());
    void headerDataChanged(TzOrientation orientation, int first, int last);
    void layoutChanged(const std::vector<TzPersistentModelIndex> &parents = std::vector<TzPersistentModelIndex>(), TzAbstractItemModel::LayoutChangeHint hint = TzAbstractItemModel::LayoutChangeHint::NoLayoutChangeHint);
    void layoutAboutToBeChanged(const std::vector<TzPersistentModelIndex> &parents = std::vector<TzPersistentModelIndex>(), TzAbstractItemModel::LayoutChangeHint hint = TzAbstractItemModel::LayoutChangeHint::NoLayoutChangeHint);

private:
    void rowsAboutToBeInserted(const TzModelIndex &parent, int first, int last);
    void rowsInserted(const TzModelIndex &parent, int first, int last);

    void rowsAboutToBeRemoved(const TzModelIndex &parent, int first, int last);
    void rowsRemoved(const TzModelIndex &parent, int first, int last);

    void columnsAboutToBeInserted(const TzModelIndex &parent, int first, int last);
    void columnsInserted(const TzModelIndex &parent, int first, int last);

    void columnsAboutToBeRemoved(const TzModelIndex &parent, int first, int last);
    void columnsRemoved(const TzModelIndex &parent, int first, int last);

    void modelAboutToBeReset();
    void modelReset();

    void rowsAboutToBeMoved( const TzModelIndex &sourceParent, int sourceStart, int sourceEnd, const TzModelIndex &destinationParent, int destinationRow);
    void rowsMoved( const TzModelIndex &sourceParent, int sourceStart, int sourceEnd, const TzModelIndex &destinationParent, int destinationRow);

    void columnsAboutToBeMoved( const TzModelIndex &sourceParent, int sourceStart, int sourceEnd, const TzModelIndex &destinationParent, int destinationColumn);
    void columnsMoved( const TzModelIndex &sourceParent, int sourceStart, int sourceEnd, const TzModelIndex &destinationParent, int destinationColumn);
// TZ_SIGNALS.end

public:
// TZ_SLOTS.begin
    virtual bool submit();
    virtual void revert();
// TZ_SLOTS.end

protected:
// TZ_SLOTS.begin
    virtual void resetInternalData();
// TZ_SLOTS.end

protected:
    explicit TzAbstractItemModel(TzAbstractItemModelPrivate &dd);

    inline TzModelIndex createIndex(int row, int column, const void *data = nullptr) const;
    inline TzModelIndex createIndex(int row, int column, uintptr_t id) const;

    void beginInsertRows(const TzModelIndex &parent, int first, int last);
    void endInsertRows();

    void beginRemoveRows(const TzModelIndex &parent, int first, int last);
    void endRemoveRows();

    bool beginMoveRows(const TzModelIndex &sourceParent, int sourceFirst, int sourceLast, const TzModelIndex &destinationParent, int destinationRow);
    void endMoveRows();

    void beginInsertColumns(const TzModelIndex &parent, int first, int last);
    void endInsertColumns();

    void beginRemoveColumns(const TzModelIndex &parent, int first, int last);
    void endRemoveColumns();

    bool beginMoveColumns(const TzModelIndex &sourceParent, int sourceFirst, int sourceLast, const TzModelIndex &destinationParent, int destinationColumn);
    void endMoveColumns();

    void beginResetModel();
    void endResetModel();

    void changePersistentIndex(const TzModelIndex &from, const TzModelIndex &to);
    void changePersistentIndexList(const TzModelIndexList &from, const TzModelIndexList &to);
    TzModelIndexList persistentIndexList() const;

private:
    TZ_DECLARE_PRIVATE(TzAbstractItemModel)
    TZ_DISABLE_COPY(TzAbstractItemModel)
    TzAbstractItemModelPrivate *d_ptr;
};

TZ_DECLARE_OPERATORS_FOR_FLAGS(TzAbstractItemModel::CheckIndexOptions)

inline bool TzAbstractItemModel::insertRow(int arow, const TzModelIndex &aparent)
{ return insertRows(arow, 1, aparent); }
inline bool TzAbstractItemModel::insertColumn(int acolumn, const TzModelIndex &aparent)
{ return insertColumns(acolumn, 1, aparent); }
inline bool TzAbstractItemModel::removeRow(int arow, const TzModelIndex &aparent)
{ return removeRows(arow, 1, aparent); }
inline bool TzAbstractItemModel::removeColumn(int acolumn, const TzModelIndex &aparent)
{ return removeColumns(acolumn, 1, aparent); }
inline bool TzAbstractItemModel::moveRow(const TzModelIndex &sourceParent, int sourceRow,
                                        const TzModelIndex &destinationParent, int destinationChild)
{ return moveRows(sourceParent, sourceRow, 1, destinationParent, destinationChild); }
inline bool TzAbstractItemModel::moveColumn(const TzModelIndex &sourceParent, int sourceColumn,
                                           const TzModelIndex &destinationParent, int destinationChild)
{ return moveColumns(sourceParent, sourceColumn, 1, destinationParent, destinationChild); }
inline TzModelIndex TzAbstractItemModel::createIndex(int arow, int acolumn, const void *adata) const
{ return TzModelIndex(arow, acolumn, adata, this); }
inline TzModelIndex TzAbstractItemModel::createIndex(int arow, int acolumn, uintptr_t aid) const
{ return TzModelIndex(arow, acolumn, aid, this); }

class TzAbstractTableModel : public TzAbstractItemModel
{
public:
    TzAbstractTableModel();
    ~TzAbstractTableModel();

    TzModelIndex index(int row, int column, const TzModelIndex &parent = TzModelIndex()) const override;
    TzModelIndex sibling(int row, int column, const TzModelIndex &idx) const override;

    TzItemFlags flags(const TzModelIndex &index) const override;

protected:
    TzAbstractTableModel(TzAbstractItemModelPrivate &dd);

private:
    TZ_DISABLE_COPY(TzAbstractTableModel)
    TzModelIndex parent(const TzModelIndex &child) const override;
    bool hasChildren(const TzModelIndex &parent) const override;
};

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

inline TzModelIndex TzModelIndex::parent() const
{ return m ? m->parent(*this) : TzModelIndex(); }

inline TzModelIndex TzModelIndex::sibling(int arow, int acolumn) const
{ return m ? (r == arow && c == acolumn) ? *this : m->sibling(arow, acolumn, *this) : TzModelIndex(); }

inline TzModelIndex TzModelIndex::siblingAtColumn(int acolumn) const
{ return m ? (c == acolumn) ? *this : m->sibling(r, acolumn, *this) : TzModelIndex(); }

inline TzModelIndex TzModelIndex::siblingAtRow(int arow) const
{ return m ? (r == arow) ? *this : m->sibling(arow, c, *this) : TzModelIndex(); }

inline std::any TzModelIndex::data(int arole) const
{ return m ? m->data(*this, arole) : std::any(); }

inline TzItemFlags TzModelIndex::flags() const
{ return m ? m->flags(*this) : TzItemFlags(); }

#endif // TZABSTRACTITEMMODEL_HPP
