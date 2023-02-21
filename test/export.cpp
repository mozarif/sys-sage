#include "gtest/gtest.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>
#include <libxml/xpath.h>

#include "sys-sage.hpp"

#include "raii.hpp"

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
    auto parser = RAII{xmlSchemaNewParserCtxt(TEST_RESOURCE_DIR "/schema.xml"), xmlSchemaFreeParserCtxt};
    if (parser == nullptr)
    {
        throw std::runtime_error{""};
    }

    auto schema = RAII{xmlSchemaParse(parser), xmlSchemaFree};

    auto validator = RAII{xmlSchemaNewValidCtxt(schema), xmlSchemaFreeValidCtxt};
    if (validator == nullptr)
    {
        throw std::runtime_error{""};
    }

    xmlSchemaSetValidErrors(validator, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stdout);

    auto doc = RAII{xmlParseFile(path.data()), xmlFreeDoc};
    if (doc == nullptr)
    {
        std::string message{"Cannot open "};
        message += path;
        message += " for validation";
        throw std::runtime_error{message};
    }

    int code = xmlSchemaValidateDoc(validator, doc);
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

TEST(ExportToXml, SampleOutput)
{
    validate("sys-sage_sample_output.xml");
}

xmlNode *getSingleNodeByPath(xmlChar *path, xmlXPathContext *context)
{
    auto result = RAII{xmlXPathEvalExpression(path, context), xmlXPathFreeObject};
    if (result->nodesetval == nullptr || result->type != XPATH_NODESET || result->nodesetval->nodeNr != 1)
    {
        std::string message{"Invalid query with path "};
        message += reinterpret_cast<const char *>(path);
        throw std::runtime_error{message};
    }
    return result->nodesetval->nodeTab[0];
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

        auto doc = RAII{xmlParseFile("test_export.xml"), xmlFreeDoc};
        auto pathContext = RAII{xmlXPathNewContext(doc), xmlXPathFreeContext};

        ASSERT_NO_THROW(getSingleNodeByPath(BAD_CAST("/sys-sage/components/Topology"), pathContext));

        {
            auto result = RAII{xmlXPathEvalExpression(BAD_CAST("string(/sys-sage/components/Topology/@id)"), pathContext), xmlXPathFreeObject};
            ASSERT_EQ("0"_xsv, result->stringval);
        }

        {
            auto result = RAII{xmlXPathEvalExpression(BAD_CAST("string(/sys-sage/components/Topology/@name)"), pathContext), xmlXPathFreeObject};
            ASSERT_EQ("sys-sage Topology"_xsv, result->stringval);
        }

        ASSERT_NO_THROW(getSingleNodeByPath(BAD_CAST("/sys-sage/components/Topology/Memory"), pathContext));

        {
            auto result = RAII{xmlXPathEvalExpression(BAD_CAST("string(/sys-sage/components/Topology/Memory/@id)"), pathContext), xmlXPathFreeObject};
            ASSERT_EQ("0"_xsv, result->stringval);
        }

        {
            auto result = RAII{xmlXPathEvalExpression(BAD_CAST("string(/sys-sage/components/Topology/Memory/@name)"), pathContext), xmlXPathFreeObject};
            ASSERT_EQ("A single memory component"_xsv, result->stringval);
        }

        {
            auto result = RAII{xmlXPathEvalExpression(BAD_CAST("string(/sys-sage/components/Topology/Memory/@size)"), pathContext), xmlXPathFreeObject};
            ASSERT_EQ("16"_xsv, result->stringval);
        }
    }
}
