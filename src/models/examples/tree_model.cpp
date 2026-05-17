#include "tztreemodel.hpp"
#include "tztreenode.hpp"
#include <loom/tzeventemitter.hpp>
#include <print>
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
        std::println("{:{}}{} [{}]", "", depth * 2, name, tag);
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

    include->addChild(utilsH);
    src->addChild(mainCpp);
    src->addChild(utils);
    src->addChild(include);
    docs->addChild(readme);

    TzTreeModel model({src, docs});

    auto onInserted = model.events().on("rowsInserted", [&](const TzModelIndex &parent, int first,
                                                            int last) {
        auto parentName = parent.isValid() ? std::any_cast<std::string>(model.data(parent))
                                           : std::string("<root>");
        std::println("  [+] rows {}..{} inserted under \"{}\"", first, last, parentName);
    });

    auto onRemoved = model.events().on("rowsRemoved", [&](const TzModelIndex &parent, int first,
                                                          int last) {
        auto parentName = parent.isValid() ? std::any_cast<std::string>(model.data(parent))
                                           : std::string("<root>");
        std::println("  [-] rows {}..{} removed from \"{}\"", first, last, parentName);
    });

    auto onChanged = model.events().on("dataChanged", [&](const TzModelIndex &topLeft,
                                                          const TzModelIndex & /*bottomRight*/,
                                                          const std::vector<int> & /*roles*/) {
        auto newName = std::any_cast<std::string>(model.data(topLeft));
        std::println("  [~] row {} renamed to \"{}\"", topLeft.row(), newName);
    });

    std::println("Initial tree:  (f=file, d=dir)");
    printModel(model);

    std::println("\nAdding \"parser.cpp\" to src/:");
    src->addChild(TzTreeNode::createItem("parser.cpp"));
    printModel(model);

    std::println("\nAdding \"tests/\" folder with \"test_main.cpp\" to root:");
    auto tests = TzTreeNode::createFolder("tests");
    auto testMain = TzTreeNode::createItem("test_main.cpp");
    tests->addChild(testMain);

    model.invisibleRoot()->addChild(tests);
    printModel(model);

    std::println("\nRenaming \"utils.cpp\" → \"util.cpp\":");
    utils->setName("util.cpp");
    printModel(model);

    std::println("\nRemoving \"docs/\":");
    model.invisibleRoot()->removeChild(docs);
    printModel(model);

    std::println("\nIndex walk — column 0 of every top-level row:");
    for (int r = 0; r < model.rowCount(); ++r) {
        TzModelIndex idx = model.index(r, 0);
        auto name = std::any_cast<std::string>(model.data(idx));
        std::println("  row {}: \"{}\"  hasChildren={}", r, name, model.hasChildren(idx));
    }

    auto idxSrc = model.indexForNode(src);
    auto recovered = model.nodeForIndex(idxSrc);
    std::println("\nRound-trip src node: \"{}\" (same={})", recovered->name(), (recovered == src));

    return 0;
}
