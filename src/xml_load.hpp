#ifndef XML_LOAD
#define XML_LOAD

#include <functional>

#include "Component.hpp"
#include "DataPath.hpp"

//SVTODO make sure all functions from the .cpp are also in the header
//SVTODO check the import and export functionalities and adapt them to Relations
namespace sys_sage {
   /**
    * Imports the sys-sage internal representation from an XML file.
    * @param path Path to the XML file.
    * @param search_custom_attrib_key_fcn Function pointer for custom attribute key search.
    * @param search_custom_complex_attrib_key_fcn Function pointer for custom complex attribute key search.
    */
   Component* importFromXml(std::string path, std::function<void*(xmlNodePtr)> search_custom_attrib_key_fcn = NULL, std::function<int(xmlNodePtr, Component*)> search_custom_complex_attrib_key_fcn = NULL);

   //SVDOCTODO private
   std::string _getStringFromProp(xmlNodePtr n, std::string prop);
   //SVDOCTODO private
   int _CreateRelations(xmlNodePtr relationNode);
   //SVDOCTODO private
   Component* _CreateComponentSubtree(xmlNodePtr n);
   /**
    * @private
    * For searching default attributes, i.e. those 
       for a specific key, and adding them to the Component.
   */
   void* _search_default_attrib_key(xmlNodePtr n);
   //SVDOCTODO private??
   int _search_default_complex_attrib_key(xmlNodePtr n, Component *c);
   /**
    * @private
    * Collects the attributes from an xmlNode and adds them to a Component.
    */
   int _collect_attrib(xmlNodePtr n, Component* c);
} //namespace sys_sage
#endif

