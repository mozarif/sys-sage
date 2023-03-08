#include "gtest/gtest.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <libxml/xpath.h>

#include "sys-sage.hpp"

#include <memory>
#include <set>
#include <string>

using XmlStringView = std::basic_string_view<const xmlChar>;

/**
 * Create a string view over UTF-8 code points as used by libxml.
 */
XmlStringView operator""_xsv(const char *string, size_t len)
{
    return {BAD_CAST(string), len};
}

template <class T>
using raii = std::unique_ptr<T, void (*)(T *)>;

/**
 * Validates XML output of sys-sage using an XML schema file.
 */
void validate(std::string_view path)
{
    auto parser = raii<xmlSchemaParserCtxt>{xmlSchemaNewParserCtxt(SYS_SAGE_TEST_RESOURCE_DIR "/schema.xml"), xmlSchemaFreeParserCtxt};
    ASSERT_NE(parser, nullptr);

    auto schema = raii<xmlSchema>{xmlSchemaParse(parser.get()), xmlSchemaFree};
    ASSERT_NE(schema, nullptr);

    auto validator = raii<xmlSchemaValidCtxt>{xmlSchemaNewValidCtxt(schema.get()), xmlSchemaFreeValidCtxt};

    xmlSchemaSetValidErrors(validator.get(), (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stdout);

    auto doc = raii<xmlDoc>{xmlParseFile(path.data()), xmlFreeDoc};
    if (doc == nullptr)
    {
        std::string message{"Cannot open "};
        message += path;
        message += " for validation";
        throw std::runtime_error{message};
    }

    int code = xmlSchemaValidateDoc(validator.get(), doc.get());
    if (code != 0)
    {
        std::string message{"XML validation of "};
        message += path;
        message += " failed";
        throw std::runtime_error{message};
    }

    auto pathContext = raii<xmlXPathContext>{xmlXPathNewContext(doc.get()), xmlXPathFreeContext};
    ASSERT_NE(pathContext, nullptr);

    // Validate that attributes either have child nodes or a value attribute
    {
        auto attributeNodes = raii<xmlXPathObject>{xmlXPathEvalExpression(BAD_CAST("//Attribute"), pathContext.get()), xmlXPathFreeObject};
        ASSERT_NE(attributeNodes->nodesetval, nullptr);
        ASSERT_EQ(attributeNodes->type, XPATH_NODESET);
        for (int i = 0; i < attributeNodes->nodesetval->nodeNr; ++i)
        {
            xmlNode *node = attributeNodes->nodesetval->nodeTab[i];
            bool hasChildren = node->children != nullptr;

            auto attributes = raii<xmlXPathObject>{xmlXPathNodeEval(node, BAD_CAST("@value"), pathContext.get()), xmlXPathFreeObject};
            bool hasValueAttribute = attributes->nodesetval->nodeNr > 0;

            bool hasNeither = !(hasChildren || hasValueAttribute);
            bool hasEitherChildrenOrValueAttribute = hasChildren != hasValueAttribute;

            EXPECT_TRUE(hasEitherChildrenOrValueAttribute || hasNeither)
                << path << ':' << node->line << " <Attribute> must have either child nodes or a value attribute";
        }
    }

    // Validate that all component addresses are unique and that all data path endpoints exist
    {
        auto componentAddrs = raii<xmlXPathObject>{xmlXPathEvalExpression(BAD_CAST("/sys-sage/components//*/@addr"), pathContext.get()), xmlXPathFreeObject};
        auto dataPathEndpoints = raii<xmlXPathObject>{
            xmlXPathEvalExpression(BAD_CAST("/sys-sage/data-paths/datapath/@source | /sys-sage/data-paths/datapath/@target"), pathContext.get()),
            xmlXPathFreeObject};

        std::set<XmlStringView> addrs;

        if (componentAddrs->nodesetval != nullptr && componentAddrs->nodesetval->nodeNr > 0)
        {
            for (int i = 0; i < componentAddrs->nodesetval->nodeNr; ++i)
            {
                auto node = componentAddrs->nodesetval->nodeTab[i];
                XmlStringView addr = node->children->content;
                EXPECT_FALSE(addrs.contains(addr))
                    << path << ':' << node->children->line << " The addr attribute must be unique for each component";
                addrs.insert(addr);
            }
        }

        if (dataPathEndpoints->nodesetval != nullptr && dataPathEndpoints->nodesetval->nodeNr > 0)
        {
            for (int i = 0; i < dataPathEndpoints->nodesetval->nodeNr; ++i)
            {
                auto endpoint = dataPathEndpoints->nodesetval->nodeTab[i];
                XmlStringView addr = endpoint->children->content;
                EXPECT_TRUE(addrs.contains(addr))
                    << path << ':' << endpoint->children->line << " The data path endpoint must be an existing component address";
            }
        }
    }
}

TEST(Export, MinimalTopology)
{
    auto topo = new Component{42, "a name", SYS_SAGE_COMPONENT_NONE};
    exportToXml(topo, "test_export.xml");
    validate("test_export.xml");
}

TEST(Export, SampleOutput)
{
    validate(SYS_SAGE_TEST_RESOURCE_DIR "/sys-sage_sample_output.xml");
}

TEST(Export, SampleOutputWithAttributes)
{
    validate(SYS_SAGE_TEST_RESOURCE_DIR "/sys-sage_custom_attributes.xml");
}

xmlNode *getSingleNodeByPath(xmlChar *path, xmlXPathContext *context)
{
    auto result = raii<xmlXPathObject>{xmlXPathEvalExpression(path, context), xmlXPathFreeObject};
    if (result->nodesetval == nullptr || result->type != XPATH_NODESET || result->nodesetval->nodeNr != 1)
    {
        std::string message{"Invalid query with path "};
        message += reinterpret_cast<const char *>(path);
        throw std::runtime_error{message};
    }
    return result->nodesetval->nodeTab[0];
}

TEST(Export, SingleComponent)
{
    {
        auto topo = new Topology;
        auto memory = new Memory{topo, "A single memory component", 16};
        (void)memory;
        exportToXml(topo, "test_export.xml");
    }

    {
        validate("test_export.xml");

        auto doc = raii<xmlDoc>{xmlParseFile("test_export.xml"), xmlFreeDoc};
        ASSERT_NE(doc, nullptr);

        auto pathContext = raii<xmlXPathContext>{xmlXPathNewContext(doc.get()), xmlXPathFreeContext};
        ASSERT_NE(pathContext, nullptr);

        xmlNode *topo = nullptr;
        ASSERT_NO_THROW(topo = getSingleNodeByPath(BAD_CAST("/sys-sage/components/Topology"), pathContext.get()));

        {
            auto result = raii<xmlXPathObject>{xmlXPathNodeEval(topo, BAD_CAST("string(@id)"), pathContext.get()), xmlXPathFreeObject};
            ASSERT_NE(result, nullptr);
            ASSERT_EQ("0"_xsv, result->stringval);
        }

        {
            auto result = raii<xmlXPathObject>{xmlXPathNodeEval(topo, BAD_CAST("string(@name)"), pathContext.get()), xmlXPathFreeObject};
            ASSERT_NE(result, nullptr);
            ASSERT_EQ("sys-sage Topology"_xsv, result->stringval);
        }

        xmlNode *memory = nullptr;
        ASSERT_NO_THROW(memory = getSingleNodeByPath(BAD_CAST("/sys-sage/components/Topology/Memory"), pathContext.get()));

        {
            auto result = raii<xmlXPathObject>{xmlXPathNodeEval(memory, BAD_CAST("string(@id)"), pathContext.get()), xmlXPathFreeObject};
            ASSERT_NE(result, nullptr);
            ASSERT_EQ("0"_xsv, result->stringval);
        }

        {
            auto result = raii<xmlXPathObject>{xmlXPathNodeEval(memory, BAD_CAST("string(@name)"), pathContext.get()), xmlXPathFreeObject};
            ASSERT_NE(result, nullptr);
            ASSERT_EQ("A single memory component"_xsv, result->stringval);
        }

        {
            auto result = raii<xmlXPathObject>{xmlXPathNodeEval(memory, BAD_CAST("string(@size)"), pathContext.get()), xmlXPathFreeObject};
            ASSERT_NE(result, nullptr);
            ASSERT_EQ("16"_xsv, result->stringval);
        }
    }
}
