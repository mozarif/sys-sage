#include "sys-sage.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <vector>
#include <map>

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(test, tautology)
{
    EXPECT_EQ(42, 42);
}

std::vector<xmlNode *> gatherChildren(xmlNode *parent)
{
    std::vector<xmlNode *> childrenVector;
    childrenVector.reserve(xmlChildElementCount(parent));
    for (xmlNode *child = xmlFirstElementChild(parent);; child = xmlNextElementSibling(child))
    {
        childrenVector.push_back(child);
        if (child == xmlLastElementChild(parent))
        {
            break;
        }
    }
    return childrenVector;
}

std::map<const xmlChar *, std::vector<xmlNode *>> gatherChildrenByName(xmlNode *parent)
{
    std::map<const xmlChar *, std::vector<xmlNode *>> map;
    for (xmlNode *child = xmlFirstElementChild(parent);; child = xmlNextElementSibling(child))
    {
        map[child->name].push_back(child);
        if (child == xmlLastElementChild(parent))
        {
            break;
        }
    }
    return map;
}

// TEST(ExportToXml, Empty) {
//     auto topo = new Topology;
//     exportToXml(topo, "test_export.xml");
//     auto doc = xmlParseFile("foo.xml");
//     ASSERT_EQ(xmlChildElementCount(doc->children), 0);
//     xmlFreeDoc(doc);
// }

TEST(ExportToXml, SingleComponent)
{
    auto topo = new Topology;
    auto memory = new Memory{topo, "A single memory component", 16};
    exportToXml(topo, "test_export.xml");
    auto doc = xmlParseFile("foo.xml");

    ASSERT_GE(xmlChildElementCount(doc->children), 1);

    xmlNode *components = nullptr;
    // for (xmlNode* node = xmlFirstElementChild(doc->children); node != xmlLastElementChild(doc->children) + 1; node = xmlNextElementSibling(node)) {
    //     std::cout << node->name << std::endl;
    // }

    gatherChildrenByName(doc->children);

    // for (xmlNode *node = xmlFirstElementChild(doc->children);; node = xmlNextElementSibling(node))
    // {
    //     if ("")
    //     std::cout << node->name << std::endl;
    //     if (node == xmlLastElementChild(doc->children))
    //     {
    //         break;
    //     }
    // }

    xmlFreeDoc(doc);
}
