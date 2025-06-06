#ifndef SYS_SAGE_BINDINGS_CPP
#define SYS_SAGE_BINDINGS_CPP

#include "defines.hpp"
#ifdef PY_SYS_SAGE

#include <cstdio>
#include <exception>
#include <libxml2/libxml/parser.h>
#include <optional>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/attr.h>
#include <string>
#include <tuple>

#include "sys-sage.hpp"

namespace py = pybind11;

std::vector<std::string> default_attribs = {"CATcos","CATL3mask","mig_size","Number_of_streaming_multiprocessors","Number_of_cores_in_GPU","Number_of_cores_per_SM","Bus_Width_bit","Clock_Frequency","latency","latency_min","latency_max","CUDA_compute_capability","mig_uuid","freq_history","GPU_Clock_Rate"};

py::function print_attributes;
py::function print_complex_attributes;

py::function read_attributes;
py::function read_complex_attributes;

int xmldumper(std::string key, void* value, std::string* ret_value_str) {
    //
    if(default_attribs.end() != std::find(default_attribs.begin(), default_attribs.end(), key))
        return 0;
    auto * ptr = static_cast<std::shared_ptr<py::object>*>(value);
    py::object res = print_attributes( py::cast(key), *ptr->get());
    //check if res is none
    if(res.is_none())
        return 0;
    *ret_value_str = res.cast<std::string>();
    return 1;
}

int xmldumper_complex(std::string key, void* value, xmlNodePtr node) {
    if(default_attribs.end() != std::find(default_attribs.begin(), default_attribs.end(), key))
        return 0;
    auto * ptr = static_cast<std::shared_ptr<py::object>*>(value);
    //idea: we expect fcn to return xml as a string and write it to node
    py::object res = print_complex_attributes(py::cast(key), *ptr->get());
    //check if res is none
    if(res.is_none())
        return 0;
    //xmlBufferPtr buffer = xmlBufferCreate();
    std::string xml_str = res.cast<std::string>();
    xmlDocPtr doc = xmlParseDoc((const xmlChar*)xml_str.c_str());
    xmlNodePtr root = xmlDocGetRootElement(doc);
    xmlAddChild(node,root->children);
    return 1;
}

void* xmlloader(xmlNodePtr node) {
    //idea: we expect fcn to return xml as a string and write it to node
    xmlBufferPtr buffer = xmlBufferCreate();
    try{
        xmlNodeDump(buffer, node->doc, node, 0, 1);
        std::string xml_str((const char*)xmlBufferContent(buffer));
        xmlBufferFree(buffer);
        py::object value = read_attributes(py::cast(xml_str));
        //check for values content
        if (value.is_none()) {
            return NULL;
        }
        std::shared_ptr<py::object> *ptr = new std::shared_ptr<py::object>(std::make_shared<py::object>(value));
        return static_cast<void*>(ptr);
    }
    catch(std::exception &e){
        std::cout << e.what() << std::endl;
        return NULL;
    }
}

int xmlloader_complex(xmlNodePtr node, sys_sage::Component *c) {
    xmlBufferPtr buffer = xmlBufferCreate();
    try{
        xmlNodeDump(buffer, node->doc, node, 0, 1);
        std::string xml_str((const char*)xmlBufferContent(buffer));
        xmlBufferFree(buffer);
        py::object comp = py::cast(c);
        py::object value = read_complex_attributes(py::cast(xml_str),comp);
        if (value.is_none())
            return 0;
        return 1;
    }
    catch(std::exception &e){
        std::cout << e.what() << std::endl;
    }
    return 0;
}

//TODO: Add dynamic allocation for values
//TODO: Delete values only after success
void set_attribute(sys_sage::Component &self, const std::string &key, py::object &value) {
    //std::cout << "set attribute: " << key << " = " << value << std::endl;

    auto val = self.attrib.find(key);
    
    if (!key.compare("CATcos") || !key.compare("CATL3mask")) {
        void * newval = static_cast<void*>(new uint64_t(py::cast<uint64_t>(value)));
        if(val != self.attrib.end())
            delete static_cast<uint64_t*>(val->second);
        self.attrib[key] = newval;
    } else if (!key.compare("mig_size")) {
        void* new_val = static_cast<void*>(new long long(py::cast<long long>(value)));
        if(val != self.attrib.end())
            delete static_cast<long long*>(val->second);
        self.attrib[key] = new_val;
    } else if (!key.compare("Number_of_streaming_multiprocessors") || 
               !key.compare("Number_of_cores_in_GPU") || 
               !key.compare("Number_of_cores_per_SM") || 
               !key.compare("Bus_Width_bit")) {
        void* new_val = static_cast<void*>(new int(py::cast<int>(value)));
        if(val != self.attrib.end())
            delete static_cast<int*>(val->second);
        self.attrib[key] = new_val;
    } else if (!key.compare("Clock_Frequency")) {
        void * new_val = static_cast<void*>(new double(py::cast<double>(value)));
        if(val != self.attrib.end())
            delete static_cast<double*>(val->second);
        self.attrib[key] = new_val;
    } else if (!key.compare("latency") ||
               !key.compare("latency_min") ||
               !key.compare("latency_max")) {
        void * new_val = static_cast<void*>(new float(py::cast<float>(value)));
        if(val != self.attrib.end())
            delete static_cast<float*>(val->second);
        self.attrib[key] = new_val;
    } else if (!key.compare("CUDA_compute_capability") || 
               !key.compare("mig_uuid")) {
        void * new_val= static_cast<void*>(new std::string(py::cast<std::string>(value)));
        if(val != self.attrib.end())
            delete static_cast<std::string*>(val->second);
        self.attrib[key] = new_val;
    } else if (!key.compare("freq_history")) {
        auto fh = new std::vector<std::tuple<long long, double>>;
        py::dict fh_dict = py::cast<py::dict>(value);
        for (auto [key, value] : fh_dict) {
            fh->push_back(std::make_tuple(py::cast<long long>(key), py::cast<double>(value)));
        }
        void * new_val = static_cast<void*>(fh);
        if(val != self.attrib.end())
            delete static_cast<std::vector<std::tuple<long long, double>>*>(val->second);
        self.attrib[key] = new_val;
    } else if (!key.compare("GPU_Clock_Rate")) {
        std::cout << "Setting attribute: " << key << std::endl;
        py::object freq = value["freq"];
        py::str unit = value["unit"];
        //double * f = new double(py::cast<double>(freq));
        //std::string * u = new std::string(py::cast<std::string>(unit));
        void * new_val = static_cast<void*>(new std::tuple<double, std::string>(py::cast<double>(freq), py::cast<std::string>(unit)));
        if(val != self.attrib.end())
            delete static_cast<std::tuple<double, std::string>*>(val->second);
        self.attrib[key] = new_val;
    } else {
        void * new_val = static_cast<void*>(new std::shared_ptr<py::object>(
            std::make_shared<py::object>(value)));
        if (val != self.attrib.end())
            delete static_cast<std::shared_ptr<py::object>*>(val->second);
        self.attrib[key] = new_val;
    }

}

py::object get_attribute(sys_sage::Component &self, const std::string &key) {
    auto val = self.attrib.find(key);
    if (val != self.attrib.end()) {
        if(!key.compare("CATcos") || !key.compare("CATL3mask")){
            uint64_t retval = *((uint64_t*)val->second); 
            return py::cast(retval);
        }
        else if(!key.compare("mig_size") )
        {
            return py::cast(*(long long*)val->second);
        }
        //val->secondue: int
        else if(!key.compare("Number_of_streaming_multiprocessors") || 
        !key.compare("Number_of_cores_in_GPU") || 
        !key.compare("Number_of_cores_per_SM")  || 
        !key.compare("Bus_Width_bit") )
        {
            return py::cast(*(int*)val->second);
        }
        //value: double
        else if(!key.compare("Clock_Frequency") )
        {
            return py::cast(*(double*)val->second);
        }
        //value: float
        else if(!key.compare("latency") ||
        !key.compare("latency_min") ||
        !key.compare("latency_max") )
        {
            return py::cast(*(float*)val->second);
        }   
        //value: string
        else if(!key.compare("CUDA_compute_capability") || 
        !key.compare("mig_uuid") )
        {
            return py::cast(*(std::string*)val->second);
        }
        else if(!key.compare("freq_history") ){
            std::vector<std::tuple<long long,double>>* value = (std::vector<std::tuple<long long,double>>*)(val->second);
            py::dict freq_dict;
             for(auto [ ts,freq ] : *value){
                 freq_dict[py::cast(ts)] = py::cast(freq);
                 //printf("ts:%lld freq:%f\n",ts,freq);
             }
             return freq_dict;
        }
        else if(!key.compare("GPU_Clock_Rate")){
            auto value = static_cast<std::tuple<double, std::string>*>(val->second);
            auto [ freq, unit ] = *value;
            py::dict freq_dict;
            freq_dict[py::str("freq")] = py::cast(freq);
            freq_dict[py::str("unit")] = py::cast(unit);  
            return freq_dict;     
        }else{
            auto * ptr = static_cast<std::shared_ptr<py::object>*>(val->second);
            return *ptr->get();
        }
    } else {
        throw py::attribute_error("Attribute '" + key + "' not found"); 
    }
}
py::object get_attribute(sys_sage::Component &self, int pos){
    if(pos >= self.attrib.size())
        throw py::index_error("Index out of bounds");
    auto it = self.attrib.begin();
    std::advance(it, pos);
    return get_attribute(self, it->first);
}


void remove_attribute(sys_sage::Component &self, const std::string &key) {
    auto val = self.attrib.find(key);
    if (val != self.attrib.end()) {
        delete static_cast<std::shared_ptr<py::object>*>(val->second);
        self.attrib.erase(val);
    } else {
        throw py::attribute_error("Attribute " + key + " not found");
    }
}


PYBIND11_MODULE(sys_sage, m) {
    using namespace sys_sage;

    m.attr("COMPONENT_NONE") = ComponentType::None;
    m.attr("COMPONENT_THREAD") = ComponentType::Thread;
    m.attr("COMPONENT_CORE") = ComponentType::Core;
    m.attr("COMPONENT_CACHE") = ComponentType::Cache;
    m.attr("COMPONENT_SUBDIVISION") = ComponentType::Subdivision;
    m.attr("COMPONENT_NUMA") = ComponentType::Numa;
    m.attr("COMPONENT_CHIP") = ComponentType::Chip;
    m.attr("COMPONENT_MEMORY") = ComponentType::Memory;
    m.attr("COMPONENT_STORAGE") = ComponentType::Storage;
    m.attr("COMPONENT_NODE") = ComponentType::Node;
    m.attr("COMPONENT_QuantumBackend") = ComponentType::QuantumBackend;
    m.attr("COMPONENT_AtomSitee") = ComponentType::AtomSite;
    m.attr("COMPONENT_Qubit") = ComponentType::Qubit;
    m.attr("COMPONENT_TOPOLOGY") = ComponentType::Topology;

    m.attr("SUBDIVISION_TYPE_NONE") = SubdivisionType::None;
    m.attr("SUBDIVISION_TYPE_GPU_SM") = SubdivisionType::GpuSM;

    m.attr("CHIP_TYPE_NONE") = ChipType::None;
    m.attr("CHIP_TYPE_CPU") = ChipType::Cpu;
    m.attr("CHIP_TYPE_CPU_SOCKET") = ChipType::CpuSocket;
    m.attr("CHIP_TYPE_GPU") = ChipType::Gpu;

    m.attr("RELATIONTYPE_Relation") = RelationType::Relation;
    m.attr("RELATIONTYPE_DataPath") = RelationType::DataPath;
    m.attr("RELATIONTYPE_QuantumGate") = RelationType::QuantumGate;
    m.attr("RELATIONTYPE_CouplingMap") = RelationType::CouplingMap;

    m.attr("DATAPATH_NONE") = DataPathDirection::Any;
    m.attr("DATAPATH_OUTGOING") = DataPathDirection::Outgoing;
    m.attr("DATAPATH_INCOMING") = DataPathDirection::Incoming;

    m.attr("DATAPATH_BIDIRECTIONAL") = DataPathOrientation::NotOriented;
    m.attr("DATAPATH_ORIENTED") = DataPathOrientation::Oriented;

    m.attr("DATAPATH_TYPE_NONE") = DataPathType::None;
    m.attr("DATAPATH_TYPE_LOGICAL") = DataPathType::Logical;
    m.attr("DATAPATH_TYPE_PHYSICAL") = DataPathType::Physical;
    m.attr("DATAPATH_TYPE_DATATRANSFER") = DataPathType::Datatransfer;
    m.attr("DATAPATH_TYPE_L3CAT") = DataPathType::L3CAT;
    m.attr("DATAPATH_TYPE_MIG") = DataPathType::MIG;
    m.attr("DATAPATH_TYPE_C2C") = DataPathType::C2C;

    //bind component class
    py::class_<Component, std::unique_ptr<Component, py::nodelete>>(m, "Component", py::dynamic_attr(),"Generic Component")
        .def(py::init<int, std::string, int>(), py::arg("id") = 0, py::arg("name") = "unknown", py::arg("type") = SYS_SAGE_COMPONENT_NONE)
        .def(py::init<Component *, int, std::string, int>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "unknown", py::arg("type") = SYS_SAGE_COMPONENT_NONE)
        .def("__setitem__", [](Component& self, const std::string& name, py::object value) {
            set_attribute(self,name, value);
        })
        .def("__getitem__", [](Component& self, const std::string& name) {
            return get_attribute(self,name);
        })
        .def("__getitem__", [](Component& self, int pos) {
            return get_attribute(self,pos);
        })
        .def("__delitem__", [](Component& self, const std::string& name) {
            remove_attribute(self,name);
        })
        .def("InsertChild", &Component::InsertChild, py::arg("child"), "Insert a child component")
        .def("InsertBetweenParentAndChild", &Component::InsertBetweenParentAndChild, py::arg("parent"), py::arg("child"), py::arg("alreadyParentsChild"),"Insert a component between parent and child")
        .def("InsertBetweenParentAndChildren", &Component::InsertBetweenParentAndChildren, py::arg("parent"), py::arg("children"), py::arg("alreadyParentsChildren"), "Insert a component between parent and children")
        .def("RemoveChild", &Component::RemoveChild, py::arg("child"),"Remove a child component")
        .def_property("parent", &Component::GetParent, &Component::SetParent, "The parent of the component")
        .def("SetParent", &Component::SetParent, py::arg("parent"), "Set the parent of the component")
        .def("PrintSubtree", (void (Component::*)()) &Component::PrintSubtree, "Print the subtree of the component up to level 0")
        .def("PrintSubtree", (void (Component::*)(int)) &Component::PrintSubtree, "Print the subtree of the component with a maximum depth of <level>")
        .def("PrintAllDataPathsInSubtree", &Component::PrintAllDataPathsInSubtree, "Print the datapath subtree of the component")
        .def_property("name", &Component::GetName, &Component::SetName, "The name of the component")
        .def_property_readonly("id", &Component::GetId, "The id of the component")
        .def_property_readonly("type", &Component::GetComponentType, "The type of the component")
        .def("GetComponentTypeStr", &Component::GetComponentTypeStr, "The type of the component as string")
        .def("GetChildren", &Component::GetChildren, "The children of the component")
        .def("GetChild", &Component::GetChild, py::arg("id"), "Like get_child_by_id()")
        .def("GetChildById", &Component::GetChildById, py::arg("id"), "Get the first child component by id")
        .def("GetChildByType", &Component::GetChildByType, py::arg("type"), "Get the first child component by type")
        //vector<Component*> as input doesnt work
        .def("GetAllChildrenByType", (void (Component::*)(vector<Component*> *, int)) &Component::GetAllChildrenByType, py::arg("children"), py::arg("type"), "Get all child components by type")
        .def("GetAllChildrenByType", (vector<Component*> (Component::*)(int))(&Component::GetAllChildrenByType), py::arg("type"), "Get all child components by type")
        .def("GetSubcomponentById", &Component::GetSubcomponentById, "Get the first sub component by id")
        .def("GetAllSubcomponentsByType", (void (Component::*)(vector<Component*> *, int))(&Component::GetAllSubcomponentsByType), py::arg("subcomponents"), py::arg("type"), "Get the first sub component by type")
        .def("GetAllSubcomponentsByType", (vector<Component*> (Component::*)(int))(&Component::GetAllSubcomponentsByType),py::arg("type") ,"Get all sub components by type")
        .def("CountAllSubcomponents", &Component::CountAllSubcomponents, "Count all sub components")
        .def("CountAllSubcomponentsByType", &Component::CountAllSubcomponentsByType, py::arg("type"),"Count sub components by type")
        .def("CountChildrenByType", &Component::CountAllChildrenByType,py::arg("type"),"Count children by type")
        .def("GetAncestorByType", &Component::GetAncestorByType, py::arg("type"),"Get the first ancestor component by type")
        .def("GetSubtreeDepth", &Component::GetSubtreeDepth, "Get the depth of the subtree")
        .def("GetNthAncestor", &Component::GetNthAncestor, py::arg("n"),"Get the nth ancestor of the component")
        .def("GetNthDescendents", (void (Component::*)(vector<Component*> *, int))&Component::GetNthDescendents,py::arg("descendents"), py::arg("n"),"Get all the nth descendents of the component")
        .def("GetNthDescendents", (vector<Component*> (Component::*)(int))&Component::GetNthDescendents,py::arg("n"),"Get all the nth descendents of the component")
        .def("GetSubcomponentsByType", (void (Component::*)(vector<Component*> *, int))&Component::GetSubcomponentsByType,py::arg("subcomponents"), py::arg("type"),"Get all the sub components of the component by type")
        .def("GetSubcomponentsByType", (vector<Component*> (Component::*)(int))&Component::GetSubcomponentsByType,py::arg("type"),"Get all the sub components of the component by type")
        .def("GetComponentsInSubtree", (void (Component::*)(vector<Component*> *))&Component::GetComponentsInSubtree,py::arg("components"),"Get all the components in the subtree of the component")
        .def("GetComponentsInSubtree", (vector<Component*> (Component::*)())&Component::GetComponentsInSubtree,"Get all the components in the subtree of the component")
        .def("GetSubcomponentById", &Component::GetSubcomponentById, py::arg("id"),py::arg("type"),"Get the first sub component by id and type")
        .def("GetDataPaths", &Component::GetDataPaths,py::arg("orientation"),"Get all the data paths associated with the component")
        .def("AddDataPath", &Component::AddDataPath,py::arg("datapath"), py::arg("orientation"),"Add a data path to the component")
        .def("GetDataPathByType", &Component::GetDataPathByType, py::arg("type"), py::arg("orientation"),"Get the first data path associated with the component by type")
        .def("GetAllDataPathsByType", (void (Component::*)(vector<DataPath*> *, int, int))&Component::GetAllDataPathsByType, py::arg("datapaths"), py::arg("type"), py::arg("orientation"),"Get all the data paths associated with the component by type")
        .def("GetAllDataPathsByType", (vector<DataPath*> (Component::*)(int, int)) &Component::GetAllDataPathsByType,py::arg("type"), py::arg("orientation"),"Get all the data paths associated with the component by type")
        .def("CheckComponentTreeConsistency", &Component::CheckComponentTreeConsistency,"Check if the component tree is consistent")
        .def("GetTopologySize", (int (Component::*)(unsigned*, unsigned*)) (&Component::GetTopologySize),py::arg("components_size"),py::arg("datapaths_size"),"Get the size of the topology")
        .def("GetTopologySize", (int (Component::*)(unsigned*, unsigned*, std::set<DataPath*>*)) (&Component::GetTopologySize),py::arg("components_size"),py::arg("datapaths_size"),py::arg("datapaths_counted"),"Get the size of the topology")
        .def("GetDepth", &Component::GetDepth,py::arg("refresh"),"Get the depth of the component, if refresh is true it will update the depth")
        .def("DeleteDataPath", &Component::DeleteDataPath, py::arg("datapath"),"Delete a data path from the component")
        .def("DeleteAllDataPaths", &Component::DeleteAllDataPaths,"Delete all the data paths from the component")
        .def("DeleteSubtree", &Component::DeleteSubtree,"Delete the subtree of the component")
        .def("Delete", &Component::Delete,py::arg("withSubtree") = true,"Delete the component")
        .def("__bool__",[](Component& self){
            std::vector<Component*>* children = self.GetChildren();
            return !children->empty();
        })
        .def("__repr__", [](Component& self) {
            //TODO: add more info
            return "<Component: " + self.GetName() + ">";
            });
    py::class_<Topology, std::unique_ptr<Topology, py::nodelete>,Component>(m, "Topology")
        .def(py::init<>());
    py::class_<Node, std::unique_ptr<Node, py::nodelete>, Component>(m, "Node")
        #ifdef INTEL_PQOS
        .def("UpdateL3CATCoreCOS", &Node::UpdateL3CATCoreCOS, py::arg("core"), py::arg("cos"))
        #endif
        #ifdef PROC_CPUINFO
        .def("RefreshCpuCoreFrequency", &Node::RefreshCpuCoreFrequency, py::arg("keep_history")=false,"Refresh the cpu core frequency")
        #endif
        .def(py::init<int, std::string>(), py::arg("id") = 0, py::arg("name")= "Node")
        .def(py::init<Component*, int, std::string>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "Node");
    py::class_<Memory,std::unique_ptr<Memory, py::nodelete>, Component>(m, "Memory")
        #ifdef NVIDIA_MIG
        .def("GetMIGSize", &Memory::GetMIGSize, py::arg("uuid"))
        #endif
        .def(py::init<long long, bool>(), py::arg("size") = -1, py::arg("isVolatile") = false)
        .def(py::init<Component*,int, std::string, long long, bool>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "Memory", py::arg("size")=-1, py::arg("isVolatile")=false)
        .def_property("size", &Memory::GetSize, &Memory::SetSize, "The size of the memory")
        .def_property("isVolatile", &Memory::GetIsVolatile, &Memory::SetIsVolatile, "Whether the memory is volatile or not");
    py::class_<Storage, std::unique_ptr<Storage, py::nodelete>, Component>(m, "Storage")
        .def(py::init<long long>(), py::arg("size")=-1)
        .def(py::init<Component*,long long>(), py::arg("parent"), py::arg("size")= -1)
        .def_property("size", &Storage::GetSize, &Storage::SetSize, "The size of the storage");
    py::class_<Chip,std::unique_ptr<Chip, py::nodelete>, Component>(m, "Chip")
        #ifdef NVIDIA_MIG
        .def("GetMIGNumCores", &Chip::GetMIGNumCores, py::arg("uuid"))
        .def("GetMIGNumSMs", &Chip::GetMIGNumSMs, py::arg("uuid"))
        .def("UpdateMIGSettings", &Chip::UpdateMIGSettings, py::arg("uuid"))
        #endif
        .def(py::init<int,std::string,int,std::string,std::string>(), py::arg("id") = 0, py::arg("name") = "Chip", py::arg("chipType")= 1, py::arg("vendor") = "", py::arg("model") = "", py::return_value_policy::reference)
        .def(py::init<Component*,int,std::string,int,std::string,std::string>(), py::arg("parent"),py::arg("id") = 0, py::arg("name") = "Chip", py::arg("chipType") = 1, py::arg("vendor") = "", py::arg("model") = "")
        .def_property("vendor", &Chip::GetVendor, &Chip::SetVendor, "The vendor of the chip")
        .def_property("model", &Chip::GetModel, &Chip::SetModel, "The model of the chip")
        .def_property("chipType", &Chip::GetChipType, &Chip::SetChipType, "The type of the chip");
    py::class_<Cache, std::unique_ptr<Cache, py::nodelete>, Component>(m, "Cache")
        #ifdef NVIDIA_MIG
        .def("GetMIGSize", &Cache::GetMIGSize, py::arg("uuid"))
        #endif
        .def(py::init<int,int,long long, int, int>(), py::arg("id") = 0, py::arg("level") = 0, py::arg("size") = -1, py::arg("associativity") = -1, py::arg("lineSize") = -1)
        .def(py::init<Component*,int,int,long long, int, int>(), py::arg("parent"), py::arg("id") = 0, py::arg("level") = 0, py::arg("size") = -1, py::arg("associativity") = -1, py::arg("lineSize") = -1)
        .def(py::init<Component*, int, std::string, long long, int, int>(), py::arg("parent"), py::arg("id"), py::arg("cache_type"), py::arg("size") = -1, py::arg("associativity") = -1, py::arg("lineSize") = -1)
        .def_property("cacheLevel", &Cache::GetCacheLevel, &Cache::SetCacheLevel, "The level of the cache")
        .def_property("cacheName", &Cache::GetCacheName, &Cache::SetCacheName, "The name of the cache")
        .def_property("cacheSize", &Cache::GetCacheSize, &Cache::SetCacheSize, "The size of the cache")
        .def_property("cacheAssociativity", &Cache::GetCacheAssociativityWays, &Cache::SetCacheAssociativityWays, "The associativity of the cache")
        .def_property("cacheLineSize", &Cache::GetCacheLineSize, &Cache::SetCacheLineSize, "The line size of the cache");
    py::class_<Subdivision, std::unique_ptr<Subdivision, py::nodelete>, Component>(m, "Subdivision")
        .def(py::init<int,std::string, int>(), py::arg("id") = 0, py::arg("name") = "Subdivision", py::arg("componentType") = 16)
        .def(py::init<Component*,int,std::string, int>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "Subdivision", py::arg("componentType") = 16)
        .def_property("subdivisionType", &Subdivision::GetSubdivisionType, &Subdivision::SetSubdivisionType, "The type of the subdivision");
    py::class_<Numa,std::unique_ptr<Numa, py::nodelete>, Subdivision>(m, "Numa")
        .def(py::init<int, long long>(), py::arg("id") = 0, py::arg("size") = -1)
        .def(py::init<Component*, int, long long>(), py::arg("parent"), py::arg("id") = 0, py::arg("size") = -1)
        .def_property("size", &Numa::GetSize, &Numa::SetSize, "Size of the NUMA region");
    py::class_<Core, std::unique_ptr<Core, py::nodelete>, Component>(m, "Core")
        
        #ifdef PROC_CPUINFO
        .def("RefreshFreq", &Core::RefreshFreq,py::arg("keep_history") = false,"Refresh the frequency of the component")
        .def_property("freq", &Core::GetFreq, &Core::SetFreq, "Frequency of this core")
        #endif
        .def(py::init<int,std::string>(),py::arg("id") = 0, py::arg("name") = "Core")
        .def(py::init<Component*,int,std::string>(),py::arg("parent"),py::arg("id") = 0 ,py::arg("name") = "Core");

    py::class_<Thread, std::unique_ptr<Thread, py::nodelete>,Component>(m,"Thread")
        #ifdef INTEL_PQOS
        .def("GetCATAwareL3Size", &Thread::GetCATAwareL3Size, "Get L3 size of this thread")
        #endif
        #ifdef PROC_CPUINFO
        .def("RefreshFreq", &Thread::RefreshFreq,py::arg("keep_history") = false,"Refresh the frequency of the component")
        .def("GetCATAwareL3Size", &Thread::GetCATAwareL3Size, "Get L3 size of this thread")
        .def_property_readonly("freq", &Thread::GetFreq, "Get Frequency of this thread")
        #endif
        .def(py::init<int,std::string>(),py::arg("id") = 0,py::arg("name") = "Thread")
        .def(py::init<Component*,int,std::string>(),py::arg("parent"),py::arg("id") = 0,py::arg("name") = "Thread");
    py::class_<DataPath, std::unique_ptr<DataPath, py::nodelete>>(m,"DataPath",py::dynamic_attr())
        .def(py::init<Component*, Component*, int, int>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("type") = 32)
        .def(py::init<Component*, Component*, int, double, double>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("bw"), py::arg("latency"))
        .def(py::init<Component*, Component*, int, int, double, double>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("type"), py::arg("bw"), py::arg("latency"))
        .def_property("bandwidth", &DataPath::GetBandwidth, &DataPath::SetBandwidth, "The bandwidth of the data path")
        .def_property("latency", &DataPath::GetLatency, &DataPath::SetLatency, "The latency of the data path")
        .def_property_readonly("type", &DataPath::GetDataPathType, "The type of the data path")
        .def_property_readonly("oriented", &DataPath::GetOrientation, "The orientation of the data path")
        .def_property("source", &DataPath::GetSource, &DataPath::UpdateSource, "The source of the data path")
        .def_property("target", &DataPath::GetTarget, &DataPath::UpdateTarget, "The target of the data path");

    m.def("parseMt4gTopo", (int (*) (Node*,std::string,int, std::string)) &parseMt4gTopo, "parseMt4gTopo", py::arg("parent"), py::arg("dataSourcePath"), py::arg("gpuID"), py::arg("delim") = ";");
    m.def("parseMt4gTopo", (int (*) (Component*,std::string,int, std::string)) &parseMt4gTopo, "parseMt4gTopo", py::arg("parent"), py::arg("dataSourcePath"), py::arg("gpuID"), py::arg("delim") = ";");
    m.def("parseMt4gTopo", (int (*) (Chip*,std::string, std::string)) &parseMt4gTopo, "parseMt4gTopo", py::arg("parent"), py::arg("dataSourcePath"),  py::arg("delim") = ";");

    m.def("parseHwlocOutput", &parseHwlocOutput, "parseHwlocOutput", py::arg("root"), py::arg("xmlPath"));

    m.def("parseCccbenchOutput", &parseCccbenchOutput, "parseCccbenchOutput", py::arg("root"), py::arg("cccPath"));

    m.def("parseCapsNumaBenchmark", &parseCapsNumaBenchmark,  py::arg("root"), py::arg("benchmarkPath"), py::arg("delim") = ";");

    m.def("exportToXml", [](Component& root, std::string xmlPath, std::optional<py::function> print_att = std::nullopt, std::optional<py::function> print_catt = std::nullopt) {
        if(print_att)
            print_attributes = *print_att;
        if(print_catt)
            print_complex_attributes = *print_catt;
        exportToXml(&root, xmlPath,print_att ? xmldumper : nullptr,print_catt ? xmldumper_complex : nullptr);
    },py::arg("root"), py::arg("xmlPath") = "out.xml", py::arg("print_att") = py::none(), py::arg("print_catt") = py::none());
    
    m.def("importFromXml",[](std::string path, std::optional<py::function> search_custom_attrib_key_fcn = std::nullopt, std::optional<py::function> search_custom_complex_attrib_key_fcn = std::nullopt) {
        if(search_custom_attrib_key_fcn)
            read_attributes = *search_custom_attrib_key_fcn;
        if(search_custom_complex_attrib_key_fcn)
            read_complex_attributes = *search_custom_complex_attrib_key_fcn;
        return importFromXml(path,search_custom_attrib_key_fcn ? xmlloader : nullptr, search_custom_complex_attrib_key_fcn ? xmlloader_complex : nullptr );
    }, py::arg("path"), py::arg("search_custom_attrib_key_fcn") = py::none(), py::arg("search_custom_complex_attrib_key_fcn") = py::none());
}


#endif //PY_SYS_SAGE
#endif //SYS_SAGE_BINDINGS_CPP