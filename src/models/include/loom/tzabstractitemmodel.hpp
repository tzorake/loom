// #pragma once
// #include <loom/tzclasshelpermacros.hpp>
// #include <loom/tzeventemitter.hpp>
// #include <any>
// #include <cstdint>
// #include <functional>
// #include <list>
// #include <memory>
// #include <string>
// #include <unordered_map>
// #include <vector>

// using TzModelIndexId = uintptr_t;

// class TzAbstractItemModel;
// class TzPersistentTzModelIndex;

// class TzModelIndex
// {
// public:
//     TzModelIndex() noexcept
//         : r(-1)
//         , c(-1)
//         , p(nullptr)
//         , m(nullptr)
//     {}

//     TzModelIndex(int row, int column, void *ptr, const TzAbstractItemModel *model) noexcept
//         : r(row)
//         , c(column)
//         , p(ptr)
//         , m(model)
//     {}

//     inline int row() const noexcept { return r; }
//     inline int column() const noexcept { return c; }
//     inline void *internalPointer() const noexcept { return p; }
//     inline TzModelIndexId internalId() const noexcept
//     {
//         return reinterpret_cast<TzModelIndexId>(p);
//     }
//     inline TzModelIndex parent() const;
//     inline const TzAbstractItemModel *model() const noexcept { return m; }

//     inline bool isValid() const noexcept { return (r >= 0) && (c >= 0) && (m != nullptr); }

//     inline bool operator==(const TzModelIndex &other) const noexcept
//     {
//         return (r == other.r) && (c == other.c) && (p == other.p) && (m == other.m);
//     }

//     inline bool operator!=(const TzModelIndex &other) const noexcept { return !(*this == other); }

//     inline bool operator<(const TzModelIndex &other) const noexcept
//     {
//         if (m < other.m)
//             return true;
//         if (m > other.m)
//             return false;
//         if (r < other.r)
//             return true;
//         if (r > other.r)
//             return false;
//         if (c < other.c)
//             return true;
//         if (c > other.c)
//             return false;
//         return p < other.p;
//     }

//     // Hash support
//     struct Hash
//     {
//         std::size_t operator()(const TzModelIndex &idx) const noexcept
//         {
//             // Qt's std::unordered_map implementation for QTzModelIndex
//             return std::hash<TzModelIndexId>{}(TzModelIndexId(idx.r) ^ (TzModelIndexId(idx.c) << 4)
//                                                ^ (TzModelIndexId(idx.p) << 8)
//                                                ^ (TzModelIndexId(idx.m) << 12));
//         }
//     };

// private:
//     int r, c;
//     void *p;
//     const TzAbstractItemModel *m;
// };

// class TzPersistentTzModelIndexData;

// class TzPersistentTzModelIndex
// {
// public:
//     TzPersistentTzModelIndex() noexcept;
//     TzPersistentTzModelIndex(const TzPersistentTzModelIndex &other) noexcept;
//     TzPersistentTzModelIndex(const TzModelIndex &index);
//     ~TzPersistentTzModelIndex();

//     TzPersistentTzModelIndex &operator=(const TzPersistentTzModelIndex &other) noexcept;
//     TzPersistentTzModelIndex &operator=(const TzModelIndex &index);

//     TzPersistentTzModelIndex(TzPersistentTzModelIndex &&other) noexcept;
//     TzPersistentTzModelIndex &operator=(TzPersistentTzModelIndex &&other) noexcept;

//     void swap(TzPersistentTzModelIndex &other) noexcept { std::swap(d, other.d); }

//     bool operator==(const TzPersistentTzModelIndex &other) const noexcept;
//     bool operator!=(const TzPersistentTzModelIndex &other) const noexcept
//     {
//         return !(*this == other);
//     }
//     bool operator<(const TzPersistentTzModelIndex &other) const noexcept;

//     int row() const noexcept;
//     int column() const noexcept;
//     void *internalPointer() const noexcept;
//     TzModelIndexId internalId() const noexcept;
//     const TzAbstractItemModel *model() const noexcept;

//     bool isValid() const noexcept;

//     operator TzModelIndex() const;

// private:
//     friend class TzAbstractItemModel;
//     TzPersistentTzModelIndexData *d;
// };

// namespace std {

// template<>
// struct hash<TzModelIndex>
// {
//     std::size_t operator()(const TzModelIndex &idx) const noexcept
//     {
//         return TzModelIndex::Hash{}(idx);
//     }
// };

// } // namespace std

// class TzAbstractItemModelPrivate;

// enum TzItemDataRole : int {
//     Display = 0x0,
//     Edit = 0x1,
//     CheckState = 0x2,
//     User = 0x1000,
// };

// enum class TzItemFlag : uint32_t {
//     NoItemFlags = 0,
//     IsSelectable = 1,
//     IsEditable = 2,
//     IsDragEnabled = 4,
//     IsDropEnabled = 8,
//     IsUserCheckable = 16,
//     IsEnabled = 32,
//     IsAutoTristate = 64,
//     NeverHasChildren = 128,
//     IsUserTristate = 256
// };

// inline TzItemFlag operator|(TzItemFlag a, TzItemFlag b)
// {
//     return static_cast<TzItemFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
// }

// inline uint32_t operator&(TzItemFlag a, TzItemFlag b)
// {
//     return static_cast<uint32_t>(a) & static_cast<uint32_t>(b);
// }

// inline TzItemFlag operator|=(TzItemFlag &a, TzItemFlag b)
// {
//     a = a | b;
//     return a;
// }

// class TzAbstractItemModel
// {
//     TZ_DECLARE_PRIVATE(TzAbstractItemModel)
// public:
//     enum class Orientation { Horizontal = 1, Vertical = 2 };
//     enum class LayoutChangeHint { NoLayoutChangeHint, VerticalSortHint, HorizontalSortHint };

//     TzAbstractItemModel();
//     virtual ~TzAbstractItemModel();

//     virtual TzModelIndex index(int row, int column,
//                                const TzModelIndex &parent = TzModelIndex()) const
//         = 0;
//     virtual TzModelIndex parent(const TzModelIndex &child) const = 0;
//     virtual int rowCount(const TzModelIndex &parent = TzModelIndex()) const = 0;
//     virtual int columnCount(const TzModelIndex &parent = TzModelIndex()) const = 0;
//     virtual std::any data(const TzModelIndex &index, int role = TzItemDataRole::Display) const = 0;

//     virtual bool setData(const TzModelIndex &index, const std::any &value,
//                          int role = TzItemDataRole::Edit);
//     virtual std::any headerData(int section, Orientation orientation,
//                                 int role = TzItemDataRole::Display) const;
//     virtual bool setHeaderData(int section, Orientation orientation, const std::any &value,
//                                int role = TzItemDataRole::Edit);

//     virtual TzItemFlag flags(const TzModelIndex &index) const;

//     virtual bool insertRows(int row, int count, const TzModelIndex &parent = TzModelIndex());
//     virtual bool insertColumns(int column, int count, const TzModelIndex &parent = TzModelIndex());
//     virtual bool removeRows(int row, int count, const TzModelIndex &parent = TzModelIndex());
//     virtual bool removeColumns(int column, int count, const TzModelIndex &parent = TzModelIndex());
//     virtual bool moveRows(const TzModelIndex &sourceParent, int sourceRow, int count,
//                           const TzModelIndex &destinationParent, int destinationChild);
//     virtual bool moveColumns(const TzModelIndex &sourceParent, int sourceColumn, int count,
//                              const TzModelIndex &destinationParent, int destinationChild);

//     bool hasIndex(int row, int column, const TzModelIndex &parent = TzModelIndex()) const;
//     TzModelIndex sibling(int row, int column, const TzModelIndex &index) const;
//     virtual bool hasChildren(const TzModelIndex &parent = TzModelIndex()) const;

//     virtual void sort(int column, bool ascending = true);

//     virtual std::vector<std::string> mimeTypes() const;
//     virtual std::any mimeData(const std::vector<TzModelIndex> &indexes) const;
//     virtual bool canDropMimeData(const std::any &data, int action, int row, int column,
//                                  const TzModelIndex &parent) const;
//     virtual bool dropMimeData(const std::any &data, int action, int row, int column,
//                               const TzModelIndex &parent);

//     TzEventEmitter &events() { return m_events; }
//     const TzEventEmitter &events() const { return m_events; }

// protected:
//     TzAbstractItemModel(TzAbstractItemModelPrivate &dd);

//     TzModelIndex createIndex(int row, int column, void *ptr = nullptr) const;
//     TzModelIndex createIndex(int row, int column, TzModelIndexId id) const;

//     void emitDataChanged(const TzModelIndex &topLeft, const TzModelIndex &bottomRight,
//                          const std::vector<int> &roles = {});
//     void emitHeaderDataChanged(Orientation orientation, int first, int last);

//     bool beginInsertRows(const TzModelIndex &parent, int first, int last);
//     void endInsertRows();
//     bool beginRemoveRows(const TzModelIndex &parent, int first, int last);
//     void endRemoveRows();
//     bool beginMoveRows(const TzModelIndex &sourceParent, int sourceFirst, int sourceLast,
//                        const TzModelIndex &destinationParent, int destinationRow);
//     void endMoveRows();

//     bool beginInsertColumns(const TzModelIndex &parent, int first, int last);
//     void endInsertColumns();
//     bool beginRemoveColumns(const TzModelIndex &parent, int first, int last);
//     void endRemoveColumns();
//     bool beginMoveColumns(const TzModelIndex &sourceParent, int sourceFirst, int sourceLast,
//                           const TzModelIndex &destinationParent, int destinationColumn);
//     void endMoveColumns();

//     void beginResetModel();
//     void endResetModel();

//     void changePersistentIndex(const TzModelIndex &from, const TzModelIndex &to);
//     void changePersistentIndexList(const std::vector<TzModelIndex> &from,
//                                    const std::vector<TzModelIndex> &to);
//     std::vector<TzModelIndex> persistentIndexList() const;

// protected:
//     std::unique_ptr<TzAbstractItemModelPrivate> d_ptr;

// private:
//     friend class TzPersistentTzModelIndex;
//     friend class TzPersistentTzModelIndexData;

//     TzEventEmitter m_events;
// };

// inline TzModelIndex TzModelIndex::parent() const
// {
//     return m ? m->parent(*this) : TzModelIndex();
// }


// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2020 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef TZABSTRACTITEMMODEL_HPP
#define TZABSTRACTITEMMODEL_HPP

#include <QtCore/qcompare.h>
#include <QtCore/qhash.h>
#include <QtCore/qlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>

class TzAbstractItemModel;
class TzPersistentModelIndex;

class TzModelIndex
{
    friend class TzAbstractItemModel;
public:
    constexpr inline TzModelIndex() noexcept : r(-1), c(-1), i(0), m(nullptr) {}
    constexpr inline int row() const noexcept { return r; }
    constexpr inline int column() const noexcept { return c; }
    constexpr inline quintptr internalId() const noexcept { return i; }
    inline void *internalPointer() const noexcept { return reinterpret_cast<void*>(i); }
    inline const void *constInternalPointer() const noexcept { return reinterpret_cast<const void *>(i); }
    inline TzModelIndex parent() const;
    inline TzModelIndex sibling(int row, int column) const;
    inline TzModelIndex siblingAtColumn(int column) const;
    inline TzModelIndex siblingAtRow(int row) const;
    inline std::any data(int role = Qt::DisplayRole) const;
    inline TzItemFlags flags() const;
    constexpr inline const TzAbstractItemModel *model() const noexcept { return m.get(); }
    constexpr inline bool isValid() const noexcept { return (r >= 0) && (c >= 0) && (m != nullptr); }

private:
    friend constexpr bool comparesEqual(const TzModelIndex &lhs, const TzModelIndex &rhs) noexcept
    {
        return lhs.r == rhs.r && lhs.c == rhs.c && lhs.i == rhs.i && lhs.m == rhs.m;
    }
    friend constexpr Qt::strong_ordering compareThreeWay(const TzModelIndex &lhs, const TzModelIndex &rhs) noexcept
    {
        if (auto val = Qt::compareThreeWay(lhs.r, rhs.r); !is_eq(val))
            return val;
        if (auto val = Qt::compareThreeWay(lhs.c, rhs.c); !is_eq(val))
            return val;
        if (auto val = Qt::compareThreeWay(lhs.i, rhs.i); !is_eq(val))
            return val;
        if (auto val = Qt::compareThreeWay(lhs.m, rhs.m); !is_eq(val))
            return val;
        return Qt::strong_ordering::equivalent;
    }
    Q_DECLARE_STRONGLY_ORDERED_LITERAL_TYPE(TzModelIndex)
private:
    inline TzModelIndex(int arow, int acolumn, const void *ptr, const TzAbstractItemModel *amodel) noexcept
        : r(arow), c(acolumn), i(reinterpret_cast<quintptr>(ptr)), m(amodel) {}
    constexpr inline TzModelIndex(int arow, int acolumn, quintptr id, const TzAbstractItemModel *amodel) noexcept
        : r(arow), c(acolumn), i(id), m(amodel) {}
    int r, c;
    quintptr i;
    const TzAbstractItemModel *m;
};

class TzPersistentModelIndexData;

size_t qHash(const TzPersistentModelIndex &index, size_t seed = 0) noexcept;

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
    inline TzPersistentModelIndex(TzPersistentModelIndex &&other) noexcept : d(std::exchange(other.d, nullptr)) {}
    TzPersistentModelIndex &operator=(TzPersistentModelIndex &&other) noexcept { swap(other); return *this; }
    void swap(TzPersistentModelIndex &other) noexcept { tzPtrSwap(d, other.d); }
    bool operator==(const TzModelIndex &other) const noexcept;
    bool operator!=(const TzModelIndex &other) const noexcept;
    TzPersistentModelIndex &operator=(const TzModelIndex &other);
    operator TzModelIndex() const;
    int row() const;
    int column() const;
    void *internalPointer() const;
    const void *constInternalPointer() const;
    quintptr internalId() const;
    TzModelIndex parent() const;
    TzModelIndex sibling(int row, int column) const;
    std::any data(int role = Qt::DisplayRole) const;
    TzItemFlags flags() const;
    const TzAbstractItemModel *model() const;
    bool isValid() const;
private:
    TzPersistentModelIndexData *d;
    friend size_t qHash(const TzPersistentModelIndex &, size_t seed) noexcept;
    friend bool qHashEquals(const TzPersistentModelIndex &a, const TzPersistentModelIndex &b) noexcept
    { return a.d == b.d; }
    friend bool comparesEqual(const TzPersistentModelIndex &lhs, const TzPersistentModelIndex &rhs) noexcept;
    friend bool comparesEqual(const TzPersistentModelIndex &lhs, const TzModelIndex &rhs) noexcept;
    friend Qt::strong_ordering compareThreeWay(const TzPersistentModelIndex &lhs, const TzPersistentModelIndex &rhs) noexcept;
    friend Qt::strong_ordering compareThreeWay(const TzPersistentModelIndex &lhs, const TzModelIndex &rhs) noexcept;
};

inline size_t qHash(const TzPersistentModelIndex &index, size_t seed) noexcept
{ return qHash(index.d, seed); }

using TzModelIndexList = std::vector<TzModelIndex>;

class TzAbstractItemModelPrivate;

class TzAbstractItemModel
{
    friend class TzPersistentModelIndexData;
public:
    explicit TzAbstractItemModel(QObject *parent = nullptr);
    virtual ~TzAbstractItemModel();

    bool hasIndex(int row, int column, const TzModelIndex &parent = TzModelIndex()) const;
    virtual TzModelIndex index(int row, int column, const TzModelIndex &parent = TzModelIndex()) const = 0;
    virtual TzModelIndex parent(const TzModelIndex &child) const = 0;

    virtual TzModelIndex sibling(int row, int column, const TzModelIndex &idx) const;
    virtual int rowCount(const TzModelIndex &parent = TzModelIndex()) const = 0;
    virtual int columnCount(const TzModelIndex &parent = TzModelIndex()) const = 0;
    virtual bool hasChildren(const TzModelIndex &parent = TzModelIndex()) const;

    virtual std::any data(const TzModelIndex &index, int role = Qt::DisplayRole) const = 0;
    virtual bool setData(const TzModelIndex &index, const std::any &value, int role = Qt::EditRole);

    virtual std::any headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const std::any &value,
                               int role = Qt::EditRole);

    virtual  std::unordered_map<int, std::any> itemData(const TzModelIndex &index) const;
    virtual bool setItemData(const TzModelIndex &index, const  std::unordered_map<int, std::any> &roles);
    virtual bool clearItemData(const TzModelIndex &index);

    virtual QStringList mimeTypes() const;
    virtual QMimeData *mimeData(const TzModelIndexList &indexes) const;
    virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action,
                                 int row, int column, const TzModelIndex &parent) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                              int row, int column, const TzModelIndex &parent);
    virtual Qt::DropActions supportedDropActions() const;
    virtual Qt::DropActions supportedDragActions() const;

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
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    virtual TzModelIndex buddy(const TzModelIndex &index) const;

    virtual std::unordered_map<int, std::string> roleNames() const;

    enum LayoutChangeHint
    {
        NoLayoutChangeHint,
        VerticalSortHint,
        HorizontalSortHint
    };
    TZ_ENUM(LayoutChangeHint)

    enum class CheckIndexOption {
        NoOption = 0x0000,
        IndexIsValid = 0x0001,
        DoNotUseParent = 0x0002,
        ParentIsInvalid = 0x0004,
    };
    TZ_ENUM(CheckIndexOption)
    TZ_DECLARE_FLAGS(CheckIndexOptions, CheckIndexOption)

    [[nodiscard]] bool checkIndex(const TzModelIndex &index, CheckIndexOptions options = CheckIndexOption::NoOption) const;

// Q_SIGNALS: BEGIN
    void dataChanged(const TzModelIndex &topLeft, const TzModelIndex &bottomRight,
                     const std::vector<int> &roles = std::vector<int>());
    void headerDataChanged(Qt::Orientation orientation, int first, int last);
    void layoutChanged(const std::vector<TzPersistentModelIndex> &parents = std::vector<TzPersistentModelIndex>(), TzAbstractItemModel::LayoutChangeHint hint = TzAbstractItemModel::NoLayoutChangeHint);
    void layoutAboutToBeChanged(const std::vector<TzPersistentModelIndex> &parents = std::vector<TzPersistentModelIndex>(), TzAbstractItemModel::LayoutChangeHint hint = TzAbstractItemModel::NoLayoutChangeHint);

    void rowsAboutToBeInserted(const TzModelIndex &parent, int first, int last, QPrivateSignal);
    void rowsInserted(const TzModelIndex &parent, int first, int last, QPrivateSignal);

    void rowsAboutToBeRemoved(const TzModelIndex &parent, int first, int last, QPrivateSignal);
    void rowsRemoved(const TzModelIndex &parent, int first, int last, QPrivateSignal);

    void columnsAboutToBeInserted(const TzModelIndex &parent, int first, int last, QPrivateSignal);
    void columnsInserted(const TzModelIndex &parent, int first, int last, QPrivateSignal);

    void columnsAboutToBeRemoved(const TzModelIndex &parent, int first, int last, QPrivateSignal);
    void columnsRemoved(const TzModelIndex &parent, int first, int last, QPrivateSignal);

    void modelAboutToBeReset(QPrivateSignal);
    void modelReset(QPrivateSignal);

    void rowsAboutToBeMoved( const TzModelIndex &sourceParent, int sourceStart, int sourceEnd, const TzModelIndex &destinationParent, int destinationRow, QPrivateSignal);
    void rowsMoved( const TzModelIndex &sourceParent, int sourceStart, int sourceEnd, const TzModelIndex &destinationParent, int destinationRow, QPrivateSignal);

    void columnsAboutToBeMoved( const TzModelIndex &sourceParent, int sourceStart, int sourceEnd, const TzModelIndex &destinationParent, int destinationColumn, QPrivateSignal);
    void columnsMoved( const TzModelIndex &sourceParent, int sourceStart, int sourceEnd, const TzModelIndex &destinationParent, int destinationColumn, QPrivateSignal);
// Q_SIGNALS: END

    virtual bool submit();
    virtual void revert();

protected:
    virtual void resetInternalData();

protected:
    TzAbstractItemModel(TzAbstractItemModelPrivate &dd);

    inline TzModelIndex createIndex(int row, int column, const void *data = nullptr) const;
    inline TzModelIndex createIndex(int row, int column, quintptr id) const;

    void encodeData(const TzModelIndexList &indexes, QDataStream &stream) const;
    bool decodeData(int row, int column, const TzModelIndex &parent, QDataStream &stream);

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

    static Qt::weak_ordering compareData(const std::any &lhs, const std::any &rhs,
                                         const QCollator *collator = nullptr);

private:
    TZ_DECLARE_PRIVATE(TzAbstractItemModel)
    TZ_DISABLE_COPY(TzAbstractItemModel)
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
inline TzModelIndex TzAbstractItemModel::createIndex(int arow, int acolumn, quintptr aid) const
{ return TzModelIndex(arow, acolumn, aid, this); }

class TzAbstractListModel : public TzAbstractItemModel
{
public:
    explicit TzAbstractListModel(QObject *parent = nullptr);
    ~TzAbstractListModel();

    TzModelIndex index(int row, int column = 0, const TzModelIndex &parent = TzModelIndex()) const override;
    TzModelIndex sibling(int row, int column, const TzModelIndex &idx) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const TzModelIndex &parent) override;

    TzItemFlags flags(const TzModelIndex &index) const override;

protected:
    TzAbstractListModel(TzAbstractItemModelPrivate &dd, QObject *parent);

private:
    TZ_DECLARE_PRIVATE(TzAbstractItemModel)
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

inline size_t qHash(const TzModelIndex &index, size_t seed = 0) noexcept
{
    return qHashMulti(seed, index.row(), index.column(), index.internalId());
}
