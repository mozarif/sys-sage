#include <sstream>
#include <cstdint>

#include "xml_dump.hpp"
#include <libxml/parser.h>

#include "Topology.hpp"
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
#include "QuantumBackend.hpp"
#include "Qubit.hpp"
#include "AtomSite.hpp"
#include "Relation.hpp"
#include "DataPath.hpp"
#include "QuantumGate.hpp"
#include "CouplingMap.hpp"

std::function<int(std::string,void*,std::string*)> store_custom_attrib_fcn = NULL;
std::function<int(std::string,void*,xmlNodePtr)> store_custom_complex_attrib_fcn = NULL;

//methods for printing out default attributes, i.e. those 
//for a specific key, return the value as a string to be printed in the xml
int sys_sage::_search_default_attrib_key(std::string key, void* value, std::string* ret_value_str)
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

int sys_sage::_search_default_complex_attrib_key(std::string key, void* value, xmlNodePtr n)
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

int sys_sage::_print_attrib(std::map<std::string,void*> attrib, xmlNodePtr n)
{
    std::string attrib_value;
    for (auto const& [key, val] : attrib){
        int ret = 0;
        if(store_custom_attrib_fcn != NULL)
            ret=store_custom_attrib_fcn(key,val,&attrib_value);
        if(ret==0)
            ret = _search_default_attrib_key(key,val,&attrib_value);

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
            ret = _search_default_complex_attrib_key(key,val,n);
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
    xmlNewProp(n, (const unsigned char *)"ChipType", (const unsigned char *)(std::to_string(type).c_str()));
    return n;
}
xmlNodePtr sys_sage::Cache::_CreateXmlSubtree()
{
    xmlNodePtr n = Component::_CreateXmlSubtree();
    xmlNewProp(n, (const unsigned char *)"cache_type", (const unsigned char *)cache_type.c_str());
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
xmlNodePtr sys_sage::Qubit::_CreateXmlSubtree()
{
    xmlNodePtr n = Component::_CreateXmlSubtree();
    xmlNewProp(n, (const unsigned char *)"q1_fidelity", (const unsigned char *)(std::to_string(q1_fidelity)).c_str());
    xmlNewProp(n, (const unsigned char *)"t1", (const unsigned char *)(std::to_string(t1)).c_str());
    xmlNewProp(n, (const unsigned char *)"t2", (const unsigned char *)(std::to_string(t2)).c_str());
    xmlNewProp(n, (const unsigned char *)"readout_fidelity", (const unsigned char *)(std::to_string(readout_fidelity)).c_str());
    xmlNewProp(n, (const unsigned char *)"readout_length", (const unsigned char *)(std::to_string(readout_length)).c_str());
    xmlNewProp(n, (const unsigned char *)"fequency", (const unsigned char *)(std::to_string(fequency)).c_str());
    xmlNewProp(n, (const unsigned char *)"calibration_time", (const unsigned char *)(calibration_time).c_str());
    return n;
}
xmlNodePtr sys_sage::QuantumBackend::_CreateXmlSubtree()
{
    //SVTODO deal with gate_types -- can this go into Relations?
    xmlNodePtr n = Component::_CreateXmlSubtree();
    xmlNewProp(n, (const unsigned char *)"num_qubits", (const unsigned char *)(std::to_string(num_qubits)).c_str());
    xmlNewProp(n, (const unsigned char *)"num_gates", (const unsigned char *)(std::to_string(num_gates)).c_str());
    // if(gate_types.size() > 0)
    // {
    //     xmlNodePtr xml_gt = xmlNewNode(NULL, (const unsigned char *)"gateTypes");
    //     xmlAddChild(n, xml_gt);
    //     for(QuantumGate* : gate_types)
    //     {
    //         xmlNodePtr attrib = xmlNewNode(NULL, (const unsigned char *)key.c_str());
    //         xmlNewProp(attrib, (const unsigned char *)"timestamp", (const unsigned char *)std::to_string(ts).c_str());
    //         xmlNewProp(attrib, (const unsigned char *)"frequency", (const unsigned char *)std::to_string(freq).c_str());
    //         xmlNewProp(attrib, (const unsigned char *)"unit", (const unsigned char *)"MHz");
    //         xmlAddChild(xml_gt, attrib);
    //     }
    // }

    return n;
}
xmlNodePtr sys_sage::AtomSite::_CreateXmlSubtree()
{
    xmlNodePtr n = Component::_CreateXmlSubtree();
    
    xmlNodePtr xml_siteprops = xmlNewNode(NULL, (const unsigned char *)"SiteProperties");
    xmlNewProp(xml_siteprops, (const unsigned char *)"nRows", (const unsigned char *)(std::to_string(properties.nRows)).c_str());
    xmlNewProp(xml_siteprops, (const unsigned char *)"nColumns", (const unsigned char *)(std::to_string(properties.nColumns)).c_str());
    xmlNewProp(xml_siteprops, (const unsigned char *)"nAods", (const unsigned char *)(std::to_string(properties.nAods)).c_str());
    xmlNewProp(xml_siteprops, (const unsigned char *)"nAodIntermediateLevels", (const unsigned char *)(std::to_string(properties.nAodIntermediateLevels)).c_str());
    xmlNewProp(xml_siteprops, (const unsigned char *)"nAodCoordinates", (const unsigned char *)(std::to_string(properties.nAodCoordinates)).c_str());
    xmlNewProp(xml_siteprops, (const unsigned char *)"interQubitDistance", (const unsigned char *)(std::to_string(properties.interQubitDistance)).c_str());
    xmlNewProp(xml_siteprops, (const unsigned char *)"interactionRadius", (const unsigned char *)(std::to_string(properties.interactionRadius)).c_str());
    xmlNewProp(xml_siteprops, (const unsigned char *)"blockingFactor", (const unsigned char *)(std::to_string(properties.blockingFactor)).c_str());
    xmlAddChild(n, xml_siteprops);

    //SVTODO handle shuttlingTimes, shuttlingAverageFidelities

    return n;
}
xmlNodePtr sys_sage::Component::_CreateXmlSubtree()
{
    xmlNodePtr n = xmlNewNode(NULL, (const unsigned char *)GetComponentTypeStr().c_str());
    xmlNewProp(n, (const unsigned char *)"id", (const unsigned char *)(std::to_string(id)).c_str());
    xmlNewProp(n, (const unsigned char *)"name", (const unsigned char *)name.c_str());
    if(count > 0)
        xmlNewProp(n, (const unsigned char *)"count", (const unsigned char *)(std::to_string(count)).c_str());
    std::ostringstream addr;
    addr << this;
    xmlNewProp(n, (const unsigned char *)"addr", (const unsigned char *)(addr.str().c_str()));

    _print_attrib(attrib, n);

    for(Component * c : children)
    {
        xmlNodePtr child = _buildComponentSubtree(c);
        xmlAddChild(n, child);
    }

    return n;
}

xmlNodePtr sys_sage::_buildComponentSubtree(Component* c)
{
    switch(c->GetComponentType()) //not all necessarily have their specific implementation; if not, it will just call the default Component->_CreateXmlSubtree 
    {
        case ComponentType::None:
            return reinterpret_cast<Component*>(c)->_CreateXmlSubtree();
        case ComponentType::Thread:
            return reinterpret_cast<Thread*>(c)->_CreateXmlSubtree();
        case ComponentType::Core:
            return reinterpret_cast<Core*>(c)->_CreateXmlSubtree();
        case ComponentType::Cache:
            return reinterpret_cast<Cache*>(c)->_CreateXmlSubtree();
        case ComponentType::Subdivision:
            return reinterpret_cast<Subdivision*>(c)->_CreateXmlSubtree();
        case ComponentType::Numa:
            return reinterpret_cast<Numa*>(c)->_CreateXmlSubtree();
        case ComponentType::Chip:
            return reinterpret_cast<Chip*>(c)->_CreateXmlSubtree();
        case ComponentType::Memory:
            return reinterpret_cast<Memory*>(c)->_CreateXmlSubtree();
        case ComponentType::Storage:
            return reinterpret_cast<Storage*>(c)->_CreateXmlSubtree();
        case ComponentType::Node:
            return reinterpret_cast<Node*>(c)->_CreateXmlSubtree();
        case ComponentType::QuantumBackend:
            return reinterpret_cast<QuantumBackend*>(c)->_CreateXmlSubtree();
        case ComponentType::Qubit:
            return reinterpret_cast<Qubit*>(c)->_CreateXmlSubtree();
        case ComponentType::AtomSite:
            return reinterpret_cast<AtomSite*>(c)->_CreateXmlSubtree();
        case ComponentType::Topology:
            return reinterpret_cast<Topology*>(c)->_CreateXmlSubtree();
        default:
            std::cerr << "ERROR: sys_sage::_buildComponentSubtree -- unknown Component Type" << std::endl;
            return NULL;
    }
    return NULL;
}


xmlNodePtr sys_sage::DataPath::_CreateXmlEntry()
{
    xmlNodePtr r_xml = Relation::_CreateXmlEntry();

    xmlNewProp(r_xml, (const unsigned char *)"DataPathType", (const unsigned char *)(std::to_string(dp_type)).c_str());
    xmlNewProp(r_xml, (const unsigned char *)"bw", (const unsigned char *)(std::to_string(bw)).c_str());
    xmlNewProp(r_xml, (const unsigned char *)"latency", (const unsigned char *)(std::to_string(latency)).c_str());
    return r_xml;
}
xmlNodePtr sys_sage::QuantumGate::_CreateXmlEntry()
{
    xmlNodePtr r_xml = Relation::_CreateXmlEntry();

    xmlNewProp(r_xml, (const unsigned char *)"gate_size", (const unsigned char *)(std::to_string(gate_size)).c_str());
    xmlNewProp(r_xml, (const unsigned char *)"name", (const unsigned char *)name.c_str());
    xmlNewProp(r_xml, (const unsigned char *)"gate_length", (const unsigned char *)(std::to_string(gate_length)).c_str());
    xmlNewProp(r_xml, (const unsigned char *)"gate_type", (const unsigned char *)(std::to_string(gate_type)).c_str());
    xmlNewProp(r_xml, (const unsigned char *)"fidelity", (const unsigned char *)(std::to_string(fidelity)).c_str());
    xmlNewProp(r_xml, (const unsigned char *)"unitary", (const unsigned char *)unitary.c_str());    
    return r_xml;
}
xmlNodePtr sys_sage::CouplingMap::_CreateXmlEntry()
{
    xmlNodePtr r_xml = Relation::_CreateXmlEntry();

    xmlNewProp(r_xml, (const unsigned char *)"fidelity", (const unsigned char *)(std::to_string(fidelity)).c_str());
    return r_xml;
}
xmlNodePtr sys_sage::Relation::_CreateXmlEntry()
{
    xmlNodePtr r_xml = xmlNewNode(NULL, BAD_CAST GetTypeStr().c_str());

    int c_idx = 0;
    for(Component* rc : components)
    {
        std::ostringstream c_addr;
        c_addr << rc;
        const std::string xmlprop_name = "component" + std::to_string(c_idx);
        xmlNewProp(r_xml, (const unsigned char *)xmlprop_name.c_str(), (const unsigned char *)(c_addr.str().c_str()));
        c_idx++;
    }

    xmlNewProp(r_xml, (const unsigned char *)"ordered", (const unsigned char *)(std::to_string(ordered).c_str()));
    xmlNewProp(r_xml, (const unsigned char *)"id", (const unsigned char *)(std::to_string(id).c_str()));
    //xmlNewProp(r_xml, (const unsigned char *)"RelationType", (const unsigned char *)(std::to_string(type).c_str()));
    //RelationType provided through the xml node name

    _print_attrib(attrib, r_xml);

    return r_xml;
}

int sys_sage::exportToXml(
    Component* root, 
    std::string path, 
    std::function<int(std::string,void*,std::string*)> _store_custom_attrib_fcn, 
    std::function<int(std::string,void*,xmlNodePtr)> _store_custom_complex_attrib_fcn)
{
    store_custom_attrib_fcn=_store_custom_attrib_fcn;
    store_custom_complex_attrib_fcn=_store_custom_complex_attrib_fcn;

    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");

    xmlNodePtr sys_sage_root = xmlNewNode(NULL, BAD_CAST "sys-sage");
    xmlDocSetRootElement(doc, sys_sage_root);
    xmlNodePtr components_root = xmlNewNode(NULL, BAD_CAST "Components");
    xmlAddChild(sys_sage_root, components_root);
    xmlNodePtr relations_root = xmlNewNode(NULL, BAD_CAST "Relations");
    xmlAddChild(sys_sage_root, relations_root);

    //build a tree for Components
    xmlNodePtr c = _buildComponentSubtree(root);
    xmlAddChild(components_root, c);
    ////

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
                    xmlNodePtr r_xml;
                    switch(r->GetType()) //not all necessarily have their specific implementation; if not, it will just call the default Relation->_CreateXmlEntry 
                    {
                        case RelationType::Relation:
                            r_xml = reinterpret_cast<Relation*>(r)->_CreateXmlEntry();
                            break;
                        case RelationType::DataPath:
                            r_xml = reinterpret_cast<DataPath*>(r)->_CreateXmlEntry();
                        break;
                        case RelationType::QuantumGate:
                            r_xml = reinterpret_cast<QuantumGate*>(r)->_CreateXmlEntry();
                            break;
                        case RelationType::CouplingMap:
                            r_xml = reinterpret_cast<CouplingMap*>(r)->_CreateXmlEntry();
                            break;
                    }
                    xmlAddChild(relations_root, r_xml);
                }
            }  
        }
    }

    xmlSaveFormatFileEnc(path=="" ? "-" : path.c_str(), doc, "UTF-8", 1);

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return 0;
}
