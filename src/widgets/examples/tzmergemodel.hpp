#pragma once
#include <loom/tzabstractitemmodel.hpp>
#include <loom/tzeventlistener.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// TzMergeModel
//
// Presents multiple source models as a single tree:
//
//   [invisible root]
//   └── <rootName>              (virtual, e.g. "home")
//       └── <categoryName>      (virtual, e.g. "materials")
//           ├── <sub[0].name>   (virtual group, e.g. "metals")
//           │   ├── ...         (source model 0 top-level items)
//           │   └── ...         (nested children delegated to source)
//           ├── <sub[1].name>   (virtual group, e.g. "fabric")
//           │   └── ...
//           └── <sub[2].name>   (virtual group, e.g. "wood")
//               └── ...
//
// Each source model's invisible root is excluded: its top-level items appear
// directly as children of the corresponding virtual group node.
// ---------------------------------------------------------------------------

class TzMergeModel : public TzAbstractItemModel
{
public:
    explicit TzMergeModel(std::string rootName     = "home",
                          std::string categoryName = "materials");
    ~TzMergeModel() override;

    // Append a source model with a display label.
    // Must be called before any model indexes are created.
    void addSubModel(std::string name, TzAbstractItemModel *model);

    // --- TzAbstractItemModel interface ---
    TzModelIndex index(int row, int column,
                       const TzModelIndex &parent = TzModelIndex()) const override;
    TzModelIndex parent(const TzModelIndex &child) const override;
    int rowCount(const TzModelIndex &parent  = TzModelIndex()) const override;
    int columnCount(const TzModelIndex &parent = TzModelIndex()) const override;
    std::any data(const TzModelIndex &index,
                  int role = TzItemDataRole::DisplayRole) const override;

    // Map a source index from sub-model `subIdx` to a proxy index.
    TzModelIndex mapFromSource(int subIdx, const TzModelIndex &srcIdx) const;
    // Map a proxy source-item index back to the original source index.
    TzModelIndex mapToSource(const TzModelIndex &proxyIdx) const;

private:
    // ------------------------------------------------------------------
    // Internal node representation stored as internalPointer in indices.
    // All pointers handed to createIndex() must remain stable for the
    // lifetime of the model, so virtual nodes are pre-allocated members
    // and source cells are heap-allocated in a map.
    // ------------------------------------------------------------------
    struct MergeNode {
        enum Kind { Home, Materials, Group, Source } kind;
        int subModelIdx{-1};
        TzModelIndex srcIdx; // valid only when kind == Source
    };

    TzModelIndex makeIndex(int row, int col, MergeNode *node) const;
    static MergeNode *nodeFor(const TzModelIndex &idx);

    MergeNode *getOrCreateSourceCell(int subIdx, const TzModelIndex &srcIdx) const;
    void      clearSourceCells(int subIdx);

    // ------------------------------------------------------------------
    std::string m_rootName;
    std::string m_categoryName;

    // Pre-allocated virtual nodes (addresses never change)
    MergeNode m_homeNode{MergeNode::Home};
    MergeNode m_materialsNode{MergeNode::Materials};

    struct SubModelEntry {
        std::string            name;
        TzAbstractItemModel   *model{nullptr};
        std::unique_ptr<MergeNode> groupNode; // stable heap address
        std::vector<TzEventListener> listeners;
        // Track whether a begin*Rows was emitted for the current signal pair.
        mutable bool insertPending{false};
        mutable bool removePending{false};
    };
    std::vector<SubModelEntry> m_subs;

    // Source cell cache: key = (subModelIdx, srcIdx.internalId())
    mutable std::map<std::pair<int, uintptr_t>, std::unique_ptr<MergeNode>> m_cells;
};
