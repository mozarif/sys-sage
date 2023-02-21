#include "sys-sage.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>

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

/**
 * Validates XML output of sys-sage using an XML schema file.
 */
void validate(std::string_view path)
{
    xmlSchemaParserCtxt *parser = xmlSchemaNewParserCtxt(TEST_RESOURCE_DIR "/schema.xml");
    if (parser == nullptr)
    {
        throw std::runtime_error{""};
    }

    xmlSchema *schema = xmlSchemaParse(parser);
    xmlSchemaFreeParserCtxt(parser);

    xmlSchemaValidCtxt *validator = xmlSchemaNewValidCtxt(schema);
    if (validator == nullptr)
    {
        xmlSchemaFree(schema);
        throw std::runtime_error{""};
    }

    xmlSchemaSetValidErrors(validator, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stdout);

    xmlDoc *doc = xmlParseFile(path.data());
    if (doc == nullptr)
    {
        xmlSchemaFree(schema);
        xmlSchemaFreeValidCtxt(validator);

        std::string message{"Cannot open "};
        message += path;
        message += " for validation";
        throw std::runtime_error{message};
    }

    int code = xmlSchemaValidateDoc(validator, doc);

    xmlSchemaFree(schema);
    xmlSchemaFreeValidCtxt(validator);

    xmlFreeDoc(doc);

    if (code != 0)
    {
        std::string message{"XML validation of "};
        message += path;
        message += " failed";
        throw std::runtime_error{message};
    }
}

TEST(ExportToXml, MinimalTopology)
{
    auto topo = new Component{42, "a name", SYS_SAGE_COMPONENT_NONE};
    exportToXml(topo, "test_export.xml");
    validate("test_export.xml");
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
        validate("test_export.xml");

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
