#pragma once
#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzeventemitter.hpp>
#include <any>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <list>
#include <cstdint>
#include <functional>

using TzModelIndexId = uintptr_t;

class TzAbstractItemModel;
class TzPersistentTzModelIndex;

class TzModelIndex
{
public:
    TzModelIndex() noexcept : r(-1), c(-1), p(nullptr), m(nullptr) {}
    
    TzModelIndex(int row, int column, void* ptr, const TzAbstractItemModel* model) noexcept
        : r(row), c(column), p(ptr), m(model) {}

    inline int row() const noexcept { return r; }
    inline int column() const noexcept { return c; }
    inline void* internalPointer() const noexcept { return p; }
    inline TzModelIndexId internalId() const noexcept { return reinterpret_cast<TzModelIndexId>(p); }
    inline TzModelIndex parent() const;
    inline const TzAbstractItemModel* model() const noexcept { return m; }
    
    inline bool isValid() const noexcept { return (r >= 0) && (c >= 0) && (m != nullptr); }
    
    inline bool operator==(const TzModelIndex& other) const noexcept {
        return (r == other.r) && (c == other.c) && (p == other.p) && (m == other.m);
    }
    
    inline bool operator!=(const TzModelIndex& other) const noexcept {
        return !(*this == other);
    }
    
    inline bool operator<(const TzModelIndex& other) const noexcept {
        if (m < other.m) return true;
        if (m > other.m) return false;
        if (r < other.r) return true;
        if (r > other.r) return false;
        if (c < other.c) return true;
        if (c > other.c) return false;
        return p < other.p;
    }

    // Hash support
    struct Hash {
        std::size_t operator()(const TzModelIndex& idx) const noexcept {
            // Qt's qHash implementation for QTzModelIndex
            return std::hash<TzModelIndexId>{}(TzModelIndexId(idx.r) 
                                        ^ (TzModelIndexId(idx.c) << 4)
                                        ^ (TzModelIndexId(idx.p) << 8)
                                        ^ (TzModelIndexId(idx.m) << 12));
        }
    };

private:
    int r, c;
    void* p;
    const TzAbstractItemModel* m;
};

class TzPersistentTzModelIndexData;

class TzPersistentTzModelIndex
{
public:
    TzPersistentTzModelIndex() noexcept;
    TzPersistentTzModelIndex(const TzPersistentTzModelIndex& other) noexcept;
    TzPersistentTzModelIndex(const TzModelIndex& index);
    ~TzPersistentTzModelIndex();
    
    TzPersistentTzModelIndex& operator=(const TzPersistentTzModelIndex& other) noexcept;
    TzPersistentTzModelIndex& operator=(const TzModelIndex& index);
    
    TzPersistentTzModelIndex(TzPersistentTzModelIndex&& other) noexcept;
    TzPersistentTzModelIndex& operator=(TzPersistentTzModelIndex&& other) noexcept;
    
    void swap(TzPersistentTzModelIndex& other) noexcept { std::swap(d, other.d); }
    
    bool operator==(const TzPersistentTzModelIndex& other) const noexcept;
    bool operator!=(const TzPersistentTzModelIndex& other) const noexcept { return !(*this == other); }
    bool operator<(const TzPersistentTzModelIndex& other) const noexcept;
    
    int row() const noexcept;
    int column() const noexcept;
    void* internalPointer() const noexcept;
    TzModelIndexId internalId() const noexcept;
    const TzAbstractItemModel* model() const noexcept;
    
    bool isValid() const noexcept;
    
    operator TzModelIndex() const;

private:
    friend class TzAbstractItemModel;
    TzPersistentTzModelIndexData* d;
};

namespace std {

template<>
struct hash<TzModelIndex> {
    std::size_t operator()(const TzModelIndex& idx) const noexcept {
        return TzModelIndex::Hash{}(idx);
    }
};

} // namespace std

class TzAbstractItemModelPrivate;

enum TzItemDataRole : int
{
    Display = 0x0,
    Edit = 0x1,
    CheckState = 0x2,
    User = 0x1000,
};

enum class TzItemFlag : uint32_t
{
    NoItemFlags = 0,
    IsSelectable = 1,
    IsEditable = 2,
    IsDragEnabled = 4,
    IsDropEnabled = 8,
    IsUserCheckable = 16,
    IsEnabled = 32,
    IsAutoTristate = 64,
    NeverHasChildren = 128,
    IsUserTristate = 256
};

inline TzItemFlag operator|(TzItemFlag a, TzItemFlag b) {
    return static_cast<TzItemFlag>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline uint32_t operator&(TzItemFlag a, TzItemFlag b) {
    return static_cast<uint32_t>(a) & static_cast<uint32_t>(b);
}

inline TzItemFlag operator|=(TzItemFlag& a, TzItemFlag b) {
    a = a | b;
    return a;
}

class TzAbstractItemModel
{
    TZ_DECLARE_PRIVATE(TzAbstractItemModel)
public:
    enum class Orientation { Horizontal = 1, Vertical = 2 };
    enum class LayoutChangeHint { NoLayoutChangeHint, VerticalSortHint, HorizontalSortHint };

    TzAbstractItemModel();
    virtual ~TzAbstractItemModel();

    virtual TzModelIndex index(int row, int column, const TzModelIndex& parent = TzModelIndex()) const = 0;
    virtual TzModelIndex parent(const TzModelIndex& child) const = 0;
    virtual int rowCount(const TzModelIndex& parent = TzModelIndex()) const = 0;
    virtual int columnCount(const TzModelIndex& parent = TzModelIndex()) const = 0;
    virtual std::any data(const TzModelIndex& index, int role = TzItemDataRole::Display) const = 0;

    virtual bool setData(const TzModelIndex& index, const std::any& value, int role = TzItemDataRole::Edit);
    virtual std::any headerData(int section, Orientation orientation, int role = TzItemDataRole::Display) const;
    virtual bool setHeaderData(int section, Orientation orientation, const std::any& value, int role = TzItemDataRole::Edit);
    
    virtual TzItemFlag flags(const TzModelIndex& index) const;
    
    virtual bool insertRows(int row, int count, const TzModelIndex& parent = TzModelIndex());
    virtual bool insertColumns(int column, int count, const TzModelIndex& parent = TzModelIndex());
    virtual bool removeRows(int row, int count, const TzModelIndex& parent = TzModelIndex());
    virtual bool removeColumns(int column, int count, const TzModelIndex& parent = TzModelIndex());
    virtual bool moveRows(const TzModelIndex& sourceParent, int sourceRow, int count,
                         const TzModelIndex& destinationParent, int destinationChild);
    virtual bool moveColumns(const TzModelIndex& sourceParent, int sourceColumn, int count,
                            const TzModelIndex& destinationParent, int destinationChild);
    
    bool hasIndex(int row, int column, const TzModelIndex& parent = TzModelIndex()) const;
    TzModelIndex sibling(int row, int column, const TzModelIndex& index) const;
    virtual bool hasChildren(const TzModelIndex& parent = TzModelIndex()) const;
    
    virtual void sort(int column, bool ascending = true);
    
    virtual std::vector<std::string> mimeTypes() const;
    virtual std::any mimeData(const std::vector<TzModelIndex>& indexes) const;
    virtual bool canDropMimeData(const std::any& data, int action, int row, int column, const TzModelIndex& parent) const;
    virtual bool dropMimeData(const std::any& data, int action, int row, int column, const TzModelIndex& parent);
    
    TzEventEmitter& events() { return m_events; }
    const TzEventEmitter& events() const { return m_events; }

protected:
    TzAbstractItemModel(TzAbstractItemModelPrivate& dd);

    TzModelIndex createIndex(int row, int column, void* ptr = nullptr) const;
    TzModelIndex createIndex(int row, int column, TzModelIndexId id) const;
    
    void emitDataChanged(const TzModelIndex& topLeft, const TzModelIndex& bottomRight,
                        const std::vector<int>& roles = {});
    void emitHeaderDataChanged(Orientation orientation, int first, int last);
    
    bool beginInsertRows(const TzModelIndex& parent, int first, int last);
    void endInsertRows();
    bool beginRemoveRows(const TzModelIndex& parent, int first, int last);
    void endRemoveRows();
    bool beginMoveRows(const TzModelIndex& sourceParent, int sourceFirst, int sourceLast,
                      const TzModelIndex& destinationParent, int destinationRow);
    void endMoveRows();
    
    bool beginInsertColumns(const TzModelIndex& parent, int first, int last);
    void endInsertColumns();
    bool beginRemoveColumns(const TzModelIndex& parent, int first, int last);
    void endRemoveColumns();
    bool beginMoveColumns(const TzModelIndex& sourceParent, int sourceFirst, int sourceLast,
                         const TzModelIndex& destinationParent, int destinationColumn);
    void endMoveColumns();
    
    void beginResetModel();
    void endResetModel();
    
    void changePersistentIndex(const TzModelIndex& from, const TzModelIndex& to);
    void changePersistentIndexList(const std::vector<TzModelIndex>& from, const std::vector<TzModelIndex>& to);
    std::vector<TzModelIndex> persistentIndexList() const;

protected:
    std::unique_ptr<TzAbstractItemModelPrivate> d_ptr;

private:
    friend class TzPersistentTzModelIndex;
    friend class TzPersistentTzModelIndexData;
    
    TzEventEmitter m_events;
};

inline TzModelIndex TzModelIndex::parent() const { return m ? m->parent(*this) : TzModelIndex(); }
