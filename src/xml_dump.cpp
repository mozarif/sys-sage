#include <sstream>
#include <cstdint>

#include "xml_dump.hpp"
#include <libxml/parser.h>

std::function<int(std::string,void*,std::string*)> store_custom_attrib_fcn = NULL;
std::function<int(std::string,void*,xmlNodePtr)> store_custom_complex_attrib_fcn = NULL;

//methods for printing out default attributes, i.e. those 
//for a specific key, return the value as a string to be printed in the xml
int sys_sage::search_default_attrib_key(std::string key, void* value, std::string* ret_value_str)
{
    //value: uint64_t 
    if(!key.compare("CATcos") || 
    !key.compare("CATL3mask") )
    {
        *ret_value_str=std::to_string(*(uint64_t*)value);
        return 1;
    }
    //value: long long
    else if(!key.compare("mig_size") )
    {
        *ret_value_str=std::to_string(*(long long*)value);
        return 1;
    }
    //value: int
    else if(!key.compare("Number_of_streaming_multiprocessors") || 
    !key.compare("Number_of_cores_in_GPU") || 
    !key.compare("Number_of_cores_per_SM")  || 
    !key.compare("Bus_Width_bit") )
    {
        *ret_value_str=std::to_string(*(int*)value);
        return 1;
    }
    //value: double
    else if(!key.compare("Clock_Frequency") )
    {
        *ret_value_str=std::to_string(*(double*)value);
        return 1;
    }
    //value: float
    else if(!key.compare("latency") ||
    !key.compare("latency_min") ||
    !key.compare("latency_max") )
    {
        *ret_value_str=std::to_string(*(float*)value);
        return 1;
    }   
    //value: string
    else if(!key.compare("CUDA_compute_capability") || 
    !key.compare("mig_uuid") )
    {
        *ret_value_str=*(std::string*)value;
        return 1;
    }

    return 0;
}

int search_default_complex_attrib_key(std::string key, void* value, xmlNodePtr n)
{
    //value: std::vector<std::tuple<long long,double>>*
    if(!key.compare("freq_history"))
    {
        std::vector<std::tuple<long long,double>>* val = (std::vector<std::tuple<long long,double>>*)value;

        xmlNodePtr attrib_node = xmlNewNode(NULL, (const unsigned char *)"Attribute");
        xmlNewProp(attrib_node, (const unsigned char *)"name", (const unsigned char *)key.c_str());
        xmlAddChild(n, attrib_node);
        for(auto [ ts,freq ] : *val)
        {
            xmlNodePtr attrib = xmlNewNode(NULL, (const unsigned char *)key.c_str());
            xmlNewProp(attrib, (const unsigned char *)"timestamp", (const unsigned char *)std::to_string(ts).c_str());
            xmlNewProp(attrib, (const unsigned char *)"frequency", (const unsigned char *)std::to_string(freq).c_str());
            xmlNewProp(attrib, (const unsigned char *)"unit", (const unsigned char *)"MHz");
            xmlAddChild(attrib_node, attrib);
        }
        return 1;
    }
    //value: std::tuple<double, std::string>
    else if(!key.compare("GPU_Clock_Rate"))
    {
        xmlNodePtr attrib_node = xmlNewNode(NULL, (const unsigned char *)"Attribute");
        xmlNewProp(attrib_node, (const unsigned char *)"name", (const unsigned char *)key.c_str());
        xmlAddChild(n, attrib_node);

        auto [ freq, unit ] = *(std::tuple<double, std::string>*)value;
        xmlNodePtr attrib = xmlNewNode(NULL, (const unsigned char *)key.c_str());
        xmlNewProp(attrib, (const unsigned char *)"frequency", (const unsigned char *)std::to_string(freq).c_str());
        xmlNewProp(attrib, (const unsigned char *)"unit", (const unsigned char *)unit.c_str());
        xmlAddChild(attrib_node, attrib);
        return 1;
    }

    return 0;
}

int sys_sage::print_attrib(std::map<std::string,void*> attrib, xmlNodePtr n)
{
    std::string attrib_value;
    for (auto const& [key, val] : attrib){
        int ret = 0;
        if(store_custom_attrib_fcn != NULL)
            ret=store_custom_attrib_fcn(key,val,&attrib_value);
        if(ret==0)
            ret = search_default_attrib_key(key,val,&attrib_value);

        if(ret==1)//attrib found
        {
            xmlNodePtr attrib_node = xmlNewNode(NULL, (const unsigned char *)"Attribute");
            xmlNewProp(attrib_node, (const unsigned char *)"name", (const unsigned char *)key.c_str());
            xmlNewProp(attrib_node, (const unsigned char *)"value", (const unsigned char *)attrib_value.c_str());
            xmlAddChild(n, attrib_node);
            continue;
        }

        if(ret == 0 && store_custom_complex_attrib_fcn != NULL) //try looking in search_custom_complex_attrib_key
            ret=store_custom_complex_attrib_fcn(key,val,n);
        if(ret==0)
            ret = search_default_complex_attrib_key(key,val,n);
    }

    return 1;
}

xmlNodePtr sys_sage::Memory::_CreateXmlSubtree()
{
    xmlNodePtr n = Component::_CreateXmlSubtree();
    if(size > 0)
        xmlNewProp(n, (const unsigned char *)"size", (const unsigned char *)(std::to_string(size)).c_str());
    xmlNewProp(n, (const unsigned char *)"is_volatile", (const unsigned char *)(std::to_string(is_volatile?1:0)).c_str());
    return n;
}
xmlNodePtr sys_sage::Storage::_CreateXmlSubtree()
{
    xmlNodePtr n = Component::_CreateXmlSubtree();
    if(size > 0)
        xmlNewProp(n, (const unsigned char *)"size", (const unsigned char *)(std::to_string(size)).c_str());
    return n;
}
xmlNodePtr sys_sage::Chip::_CreateXmlSubtree()
{
    xmlNodePtr n = Component::_CreateXmlSubtree();
    if(!vendor.empty())
        xmlNewProp(n, (const unsigned char *)"vendor", (const unsigned char *)(vendor.c_str()));
    if(!model.empty())
        xmlNewProp(n, (const unsigned char *)"model", (const unsigned char *)(model.c_str()));
    return n;
}
xmlNodePtr sys_sage::Cache::_CreateXmlSubtree()
{
    xmlNodePtr n = Component::_CreateXmlSubtree();
    xmlNewProp(n, (const unsigned char *)"cache_level", (const unsigned char *)cache_type.c_str());
    if(cache_size >= 0)
        xmlNewProp(n, (const unsigned char *)"cache_size", (const unsigned char *)(std::to_string(cache_size)).c_str());
    if(cache_associativity_ways >= 0)
        xmlNewProp(n, (const unsigned char *)"cache_associativity_ways", (const unsigned char *)(std::to_string(cache_associativity_ways)).c_str());
    if(cache_line_size >= 0)
        xmlNewProp(n, (const unsigned char *)"cache_line_size", (const unsigned char *)(std::to_string(cache_line_size)).c_str());
    return n;
}
xmlNodePtr sys_sage::Subdivision::_CreateXmlSubtree()
{
    xmlNodePtr n = Component::_CreateXmlSubtree();
    xmlNewProp(n, (const unsigned char *)"subdivision_type", (const unsigned char *)(std::to_string(type)).c_str());
    return n;
}
xmlNodePtr sys_sage::Numa::_CreateXmlSubtree()
{
    xmlNodePtr n = Component::_CreateXmlSubtree();
    if(size > 0)
        xmlNewProp(n, (const unsigned char *)"size", (const unsigned char *)(std::to_string(size)).c_str());
    return n;
}
xmlNodePtr sys_sage::Component::_CreateXmlSubtree()
{
    using namespace sys_sage;
    xmlNodePtr n = xmlNewNode(NULL, (const unsigned char *)GetComponentTypeStr().c_str());
    xmlNewProp(n, (const unsigned char *)"id", (const unsigned char *)(std::to_string(id)).c_str());
    xmlNewProp(n, (const unsigned char *)"name", (const unsigned char *)name.c_str());
    if(count > 0)
        xmlNewProp(n, (const unsigned char *)"count", (const unsigned char *)(std::to_string(count)).c_str());
    std::ostringstream addr;
    addr << this;
    xmlNewProp(n, (const unsigned char *)"addr", (const unsigned char *)(addr.str().c_str()));

    print_attrib(attrib, n);

    for(Component * c : children)
    {
        xmlNodePtr child;
        switch (c->GetComponentType()) {
            case ComponentType::Cache:
                child = ((Cache*)c)->_CreateXmlSubtree();
                break;
            case ComponentType::Subdivision:
                child = ((Subdivision*)c)->_CreateXmlSubtree();
                break;
            case ComponentType::Numa:
                child = ((Numa*)c)->_CreateXmlSubtree();
                break;
            case ComponentType::Chip:
                child = ((Chip*)c)->_CreateXmlSubtree();
                break;
            case ComponentType::Memory:
                child = ((Memory*)c)->_CreateXmlSubtree();
                break;
            case ComponentType::Storage:
                child = ((Storage*)c)->_CreateXmlSubtree();
                break;
            case ComponentType::None:
            case ComponentType::Thread:
            case ComponentType::Core:
            case ComponentType::Node:
            case ComponentType::Topology:
            default:
                child = c->_CreateXmlSubtree();
                break;
        };

        xmlAddChild(n, child);
    }


    return n;
}

int sys_sage::exportToXml(
    Component* root, 
    std::string path, 
    std::function<int(std::string,void*,std::string*)> _store_custom_attrib_fcn, 
    std::function<int(std::string,void*,xmlNodePtr)> _store_custom_complex_attrib_fcn)
{
    using namespace sys_sage;
    store_custom_attrib_fcn=_store_custom_attrib_fcn;
    store_custom_complex_attrib_fcn=_store_custom_complex_attrib_fcn;

    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");

    xmlNodePtr sys_sage_root = xmlNewNode(NULL, BAD_CAST "sys-sage");
    xmlDocSetRootElement(doc, sys_sage_root);
    xmlNodePtr components_root = xmlNewNode(NULL, BAD_CAST "components");
    xmlAddChild(sys_sage_root, components_root);
    xmlNodePtr relations_root = xmlNewNode(NULL, BAD_CAST "relations");
    xmlAddChild(sys_sage_root, relations_root);

    //build a tree for Components
    xmlNodePtr n = root->_CreateXmlSubtree();
    xmlAddChild(components_root, n);

    //scan all Components for their relations
    std::vector<Component*> components;
    root->GetComponentsInSubtree(&components);
    std::cout << "Number of components to export: " << components.size() << std::endl;
    for(Component* cPtr : components)
    {
        //iterate over different relation types and process them separately
        for(RelationType::type rt : RelationType::RelationTypeList)
        {
            std::vector<Relation*> rList = cPtr->GetRelations(rt);

            for(Relation* r: rList)
            {
                //print only if this component has index 0 => print each Relation once only
                if(r->GetComponent(0) == cPtr)
                {
                    xmlNodePtr r_n = xmlNewNode(NULL, BAD_CAST r->GetTypeStr().c_str());

                    int c_idx = 0;
                    for(Component* rc : r->GetComponents())
                    {
                        std::ostringstream c_addr;
                        c_addr << rc;
                        const std::string xmlprop_name = "component" + std::to_string(c_idx);
                        xmlNewProp(r_n, (const unsigned char *)xmlprop_name.c_str(), (const unsigned char *)(c_addr.str().c_str()));
                        c_idx++;
                    }

                    xmlNewProp(r_n, (const unsigned char *)"ordered", (const unsigned char *)(std::to_string(r->IsOrdered())).c_str());

                    if(rt == RelationType::DataPath)
                    {
                        xmlNewProp(r_n, (const unsigned char *)"DataPathType", (const unsigned char *)(std::to_string(reinterpret_cast<DataPath*>(r)->GetDataPathType())).c_str());
                        xmlNewProp(r_n, (const unsigned char *)"bw", (const unsigned char *)(std::to_string(reinterpret_cast<DataPath*>(r)->GetBandwidth())).c_str());
                        xmlNewProp(r_n, (const unsigned char *)"latency", (const unsigned char *)(std::to_string(reinterpret_cast<DataPath*>(r)->GetLatency())).c_str());
                    }
                    xmlAddChild(relations_root, r_n);

                    print_attrib(r->attrib, r_n);
                }
            }  
        }
    }

    xmlSaveFormatFileEnc(path=="" ? "-" : path.c_str(), doc, "UTF-8", 1);

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return 0;
}
