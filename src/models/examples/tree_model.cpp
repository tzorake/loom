#include "tztreemodel.hpp"
#include "tztreenode.hpp"
#include <loom/tzeventemitter.hpp>
#include <string>

static void printModel(const TzTreeModel &model, const TzModelIndex &parent = TzModelIndex(),
                       int depth = 0)
{
    const int rows = model.rowCount(parent);
    for (int r = 0; r < rows; ++r) {
        TzModelIndex idx = model.index(r, 0, parent);
        auto name = std::any_cast<std::string>(model.data(idx));
        auto node = model.nodeForIndex(idx);
        char tag = node->isFolder() ? 'd' : 'f';
        tzInfo("{:{}}{} [{}]", "", depth * 2, name, tag);
        if (model.hasChildren(idx))
            printModel(model, idx, depth + 1);
    }
}

int main()
{
    auto src = TzTreeNode::createFolder("src");
    auto mainCpp = TzTreeNode::createItem("main.cpp");
    auto utils = TzTreeNode::createItem("utils.cpp");
    auto include = TzTreeNode::createFolder("include");
    auto utilsH = TzTreeNode::createItem("utils.hpp");
    auto docs = TzTreeNode::createFolder("docs");
    auto readme = TzTreeNode::createItem("README.md");

    utilsH->setParent(include);
    mainCpp->setParent(src);
    utils->setParent(src);
    include->setParent(src);
    readme->setParent(docs);

    TzTreeModel model({src, docs});

    auto onInserted = model.events().on("rowsInserted", [&](const TzModelIndex &parent, int first,
                                                            int last) {
        auto parentName = parent.isValid() ? std::any_cast<std::string>(model.data(parent))
                                           : std::string("<root>");
        tzInfo("  [+] rows {}..{} inserted under \"{}\"", first, last, parentName);
    });

    auto onRemoved = model.events().on("rowsRemoved", [&](const TzModelIndex &parent, int first,
                                                          int last) {
        auto parentName = parent.isValid() ? std::any_cast<std::string>(model.data(parent))
                                           : std::string("<root>");
        tzInfo("  [-] rows {}..{} removed from \"{}\"", first, last, parentName);
    });

    auto onChanged = model.events().on("dataChanged", [&](const TzModelIndex &topLeft,
                                                          const TzModelIndex & /*bottomRight*/,
                                                          const std::vector<int> & /*roles*/) {
        auto newName = std::any_cast<std::string>(model.data(topLeft));
        tzInfo("  [~] row {} renamed to \"{}\"", topLeft.row(), newName);
    });

    tzInfo("Initial tree:  (f=file, d=dir)");
    printModel(model);

    tzInfo("\nAdding \"parser.cpp\" to src/:");
    TzTreeNode::createItem("parser.cpp")->setParent(src);
    printModel(model);

    tzInfo("\nAdding \"tests/\" folder with \"test_main.cpp\" to root:");
    auto tests = TzTreeNode::createFolder("tests");
    auto testMain = TzTreeNode::createItem("test_main.cpp");
    testMain->setParent(tests);

    tests->setParent(model.invisibleRoot());
    printModel(model);

    tzInfo("\nRenaming \"utils.cpp\" → \"util.cpp\":");
    utils->setName("util.cpp");
    printModel(model);

    tzInfo("\nRemoving \"docs/\":");
    docs->setParent(nullptr);
    printModel(model);

    tzInfo("\nIndex walk — column 0 of every top-level row:");
    for (int r = 0; r < model.rowCount(); ++r) {
        TzModelIndex idx = model.index(r, 0);
        auto name = std::any_cast<std::string>(model.data(idx));
        tzInfo("  row {}: \"{}\"  hasChildren={}", r, name, model.hasChildren(idx));
    }

    auto idxSrc = model.indexForNode(src);
    auto recovered = model.nodeForIndex(idxSrc);
    tzInfo("\nRound-trip src node: \"{}\" (same={})", recovered->name(), (recovered == src));

    return 0;
}
