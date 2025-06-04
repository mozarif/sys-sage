#ifndef HWLOC
#define HWLOC

#include <vector>
#include <string>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "Component.hpp"
#include "Thread.hpp"
#include "Core.hpp"
#include "Cache.hpp"
#include "Subdivision.hpp"
#include "Numa.hpp"
#include "Chip.hpp"
#include "Memory.hpp"
#include "Storage.hpp"
#include "Node.hpp"

/*! \file */

namespace sys_sage {
    /**
    Parser function for importing hwloc XML output to sys-sage.
    \n The parser looks for the XML object names defined in xmlRelevantNames, and considers (i.e. parses) the XML object types as defined in xmlRelevantObjectTypes.
    @param n - Pointer to an already existing Node where the hwloc topology will get parsed.
    @param xmlPath - Path to the XML output of hwloc that should be parsed and uploaded to sys-sage.
    */
    int parseHwlocOutput(Node* n, std::string xmlPath);
    /// @private
    int xmlProcessChildren(Component* c, xmlNode* parent, int level);
    /// @private
    Component* createChildC(std::string type, xmlNode* node);
    /// @private
    std::string xmlGetPropStr(xmlNode* node, std::string key);

    //SVDOCTODO private?
    int removeUnknownCompoents(Component* c);
    /**
    Defines parsed XML object names: "topology", "object"
    */
    extern std::vector<std::string> xmlRelevantNames;
    /**
    Defines parsed XML object types: "Machine", "Package", "Cache", "L3Cache", "L2Cache", "L1Cache", "NUMANode", "Core", "PU"
    */
    extern std::vector<std::string> xmlRelevantObjectTypes;

} //namespace sys_sage
#endif
