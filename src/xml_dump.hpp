#ifndef XML_DUMP
#define XML_DUMP

#include <functional>

#include "Component.hpp"
#include "DataPath.hpp"

namespace sys_sage{
   /**
    * Exports the Component Tree to an XML file.
    */
   int exportToXml(Component *root, std::string path = "", std::function<int(std::string, void *, std::string *)> search_custom_attrib_key_fcn = NULL, std::function<int(std::string, void *, xmlNodePtr)> search_custom_complex_attrib_key_fcn = NULL);
   //SVDOCTODO private
   int _search_default_complex_attrib_key(std::string key, void* value, xmlNodePtr n);
   /**
    * @private
    * For searching default attributes, i.e. those 
       for a specific key, return the value as a string to be printed in the xml
   */
   int _search_default_attrib_key(std::string key, void *value, std::string *ret_value_str);
   //SVDOCTODO private
   xmlNodePtr _buildComponentSubtree(Component* root);
   /**
    * @private
    * Prints the attributes.
    */
   int _print_attrib(std::map<std::string, void *> attrib, xmlNodePtr n);
} //namespace sys_sage
#endif
