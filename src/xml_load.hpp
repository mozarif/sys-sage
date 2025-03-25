#ifndef XML_LOAD
#define XML_LOAD

#include <functional>

#include "Component.hpp"
#include "DataPath.hpp"

/**
 * Imports the sys-sage internal representation from an XML file.
 * @param path Path to the XML file.
 * @param search_custom_attrib_key_fcn Function pointer for custom attribute key search.
 * @param search_custom_complex_attrib_key_fcn Function pointer for custom complex attribute key search.
 */
Component* importFromXml(string path, std::function<void*(xmlNodePtr)> search_custom_attrib_key_fcn = NULL, std::function<int(xmlNodePtr, Component*)> search_custom_complex_attrib_key_fcn = NULL);

/**
 * @private
 * For searching default attributes, i.e. those 
    for a specific key, and adding them to the Component.
 */
void* search_default_attrib_key(xmlNodePtr n);

/**
 * @private
 * Collects the attributes from an xmlNode and adds them to a Component.
 */
int collect_attrib(xmlNodePtr n, Component* c);

#endif

