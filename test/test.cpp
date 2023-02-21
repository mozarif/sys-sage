#include "sys-sage.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <string_view>

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/**
 * Create a string view over UTF-8 code points as used by libxml.
 */
std::basic_string_view<const xmlChar> operator""_xsv(const char *string, size_t len)
{
    return {BAD_CAST(string), len};
}

TEST(ExportToXml, MinimalTopology)
{
    {
        auto topo = new Component{42, "a name", SYS_SAGE_COMPONENT_NONE};
        exportToXml(topo, "test_export.xml");
    }
    {
        xmlDoc *doc = xmlParseFile("test_export.xml");
        ASSERT_NE(doc->children, nullptr);
        ASSERT_NE(doc->children->name, nullptr);
        xmlNode *sysSage = doc->children;
        ASSERT_EQ("sys-sage"_xsv, sysSage->name);

        bool foundComponents = false;
        bool foundDataPaths = false;

        for (xmlNode *child = sysSage->children; child; child = child->next)
        {
            if ("components"_xsv == child->name)
            {
                EXPECT_FALSE(foundComponents);
                foundComponents = true;
            }
            else if ("data-paths"_xsv == child->name)
            {
                EXPECT_FALSE(foundDataPaths);
                foundDataPaths = true;
            }
        }

        EXPECT_TRUE(foundComponents);
        EXPECT_TRUE(foundDataPaths);

        xmlFreeDoc(doc);
    }
}

TEST(ExportToXml, SingleComponent)
{
    {
        auto topo = new Topology;
        auto memory = new Memory{topo, "A single memory component", 16};
        (void)memory;
        exportToXml(topo, "test_export.xml");
    }

    {
        xmlDoc *doc = xmlParseFile("test_export.xml");

        ASSERT_GE(xmlChildElementCount(doc->children), 1);

        xmlNode *node = doc->children;

        for (xmlNode *child = node->children; child; child = child->next)
        {
            if ("components"_xsv == child->name)
            {
                node = child;
                break;
            }
        }

        {
            bool foundTopology = false;
            for (xmlNode *child = node->children; child; child = child->next)
            {
                if ("Topology"_xsv == child->name)
                {
                    EXPECT_FALSE(foundTopology);
                    foundTopology = true;
                    node = child;
                }
            }
            ASSERT_TRUE(foundTopology);
        }

        auto topo = node;

        {
            bool foundIdProp = false;
            bool foundNameProp = false;
            for (auto prop = topo->properties; prop != nullptr; prop = prop->next)
            {
                if ("id"_xsv == prop->name)
                {
                    foundIdProp = true;
                    ASSERT_NE(prop->children, nullptr);
                    EXPECT_EQ("0"_xsv, prop->children->content);
                }
                else if ("name"_xsv == prop->name)
                {
                    foundNameProp = true;
                    EXPECT_NE(prop->children, nullptr);
                }
            }
            EXPECT_TRUE(foundIdProp);
            EXPECT_TRUE(foundNameProp);
        }

        xmlNode *memory = nullptr;
        for (auto child = topo->children; child != nullptr; child = child->next)
        {
            if ("Memory"_xsv == child->name)
            {
                memory = child;
                break;
            }
        }
        ASSERT_NE(memory, nullptr);

        {
            bool foundIdProp = false;
            bool foundNameProp = false;
            bool foundSizeProp = false;
            for (auto prop = memory->properties; prop != nullptr; prop = prop->next)
            {
                if ("id"_xsv == prop->name)
                {
                    foundIdProp = true;
                    ASSERT_NE(prop->children, nullptr);
                    EXPECT_EQ("0"_xsv, prop->children->content);
                }
                else if ("name"_xsv == prop->name)
                {
                    foundNameProp = true;
                    ASSERT_NE(prop->children, nullptr);
                    EXPECT_EQ("A single memory component"_xsv, prop->children->content);
                }
                else if ("size"_xsv == prop->name)
                {
                    foundSizeProp = true;
                    ASSERT_NE(prop->children, nullptr);
                    EXPECT_EQ("16"_xsv, prop->children->content);
                }
            }
            EXPECT_TRUE(foundIdProp);
            EXPECT_TRUE(foundNameProp);
            EXPECT_TRUE(foundSizeProp);
        }

        xmlFreeDoc(doc);
    }
}
