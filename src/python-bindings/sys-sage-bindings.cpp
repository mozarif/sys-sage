#include "defines.hpp"

// #ifdef PYBIND

#include <exception>
#include <libxml2/libxml/parser.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/attr.h>
#include <string>
#include <tuple>
#include "Component.hpp"
#include "DataPath.hpp"
#include "sys-sage.hpp"



namespace py = pybind11;

std::vector<std::string> default_attribs = {"CATcos","CATL3mask","mig_size","Number_of_streaming_multiprocessors","Number_of_cores_in_GPU","Number_of_cores_per_SM","Bus_Width_bit","Clock_Frequency","latency","latency_min","latency_max","CUDA_compute_capability","mig_uuid","freq_history","GPU_Clock_Rate"};

py::function print_attributes;
py::function print_complex_attributes;

py::function read_attributes;
py::function read_complex_attributes;

int convert(std::map<std::string, void*> &attributes, py::object comp, py::function fcn, bool toString) {
    py::dict object_attributes = comp.attr("__dict__");
    for (auto const& [key, value] : object_attributes) {
        std::string val_str = fcn(key, value).cast<std::string>();
        attributes[py::cast<std::string>(key)] = static_cast<void*>(&val_str);
    }
    return 1;
}

int xmldumper(std::string key, void* value, std::string* ret_value_str) {
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
    xmlBufferPtr buffer = xmlBufferCreate();

    //xmlDocPtr doc = xmlParseDoc(res.cast<const unsigned char*>());
    return 0;
}

void* xmlloader(xmlNodePtr node) {
    //idea: we expect fcn to return xml as a string and write it to node
    xmlBufferPtr buffer = xmlBufferCreate();
    xmlNodeDump(buffer, node->doc, node, 0, 1);
    std::string xml_str((const char*)buffer->content, buffer->size);
    xmlBufferFree(buffer);
    //now pass to read_attributes
    py::object value = read_attributes(py::cast(xml_str));
    std::shared_ptr<py::object> *ptr = new std::shared_ptr<py::object>(std::make_shared<py::object>(value));
    return static_cast<void*>(ptr);
}

int xmlloader_complex(xmlNodePtr node, Component *c) {
    py::object comp = py::cast(c);
    //idea: we expect fcn to return xml as a string and write it to node
    xmlBufferPtr buffer = xmlBufferCreate();
    // xmlNodeDump(buffer, node->doc, node, 0, 1);
    // std::string xml_str((const char*)buffer->content, buffer->size);
    // xmlBufferFree(buffer);
    // //now pass to read_attributes
    // py::object ret = read_complex_attributes(py::cast(xml_str), comp);
    // return ret.cast<int>();
    return 0;
}

void set_attribute(Component &self, const std::string &key, py::object &value) {
    if(default_attribs.end() != std::find(default_attribs.begin(), default_attribs.end(), key))
        throw py::type_error("Attribute " + key + " is read-only");
    auto obj = new std::shared_ptr<py::object>(std::make_shared<py::object>(value));
    // free existing value first
    auto val = self.attrib.find(key);
    if (val != self.attrib.end()) {
        delete static_cast<std::shared_ptr<py::object>*>(val->second);
    }
    self.attrib[key] = static_cast<void*>(obj);
}

py::object get_attribute(Component &self, const std::string &key) {
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
            return py::cast(*(string*)val->second);
        }
        else if(!key.compare("freq_history") ){
            std::vector<std::tuple<long long,double>>* value = (std::vector<std::tuple<long long,double>>*)(val->second);
            py::dict freq_dict;
             for(auto [ ts,freq ] : *value){
                 freq_dict[py::cast(ts)] = py::cast(freq);
             }
             return freq_dict;
        }
        else if(!key.compare("GPU_Clock_Rate")){
            auto [ freq, unit ] = *(std::tuple<double, std::string>*)val->second;
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

void remove_attribute(Component &self, const std::string &key) {
    auto val = self.attrib.find(key);
    if (val != self.attrib.end()) {
        delete static_cast<std::shared_ptr<py::object>*>(val->second);
        self.attrib.erase(val);
    } else {
        throw py::attribute_error("Attribute " + key + " not found");
    }
}

py::dict syncAttributes(std::map<std::string, void*> &attributes, py::dict object_attributes) {
    py::dict dict;
    for (auto const& [key, value] : attributes) {
        if(!key.compare("CATcos") || !key.compare("CATL3mask")){
            uint64_t retval = *((uint64_t*)value); 
            dict[key.c_str()] = py::cast(retval);
        }
        else if(!key.compare("mig_size") )
        {
            dict[key.c_str()] = py::cast(*(long long*)value);
        }
        //value: int
        else if(!key.compare("Number_of_streaming_multiprocessors") || 
        !key.compare("Number_of_cores_in_GPU") || 
        !key.compare("Number_of_cores_per_SM")  || 
        !key.compare("Bus_Width_bit") )
        {
            dict[key.c_str()] = py::cast(*(int*)value);
        }
        //value: double
        else if(!key.compare("Clock_Frequency") )
        {
            dict[key.c_str()] = py::cast(*(double*)value);
        }
        //value: float
        else if(!key.compare("latency") ||
        !key.compare("latency_min") ||
        !key.compare("latency_max") )
        {
            dict[key.c_str()] = py::cast(*(float*)value);
        }   
        //value: string
        else if(!key.compare("CUDA_compute_capability") || 
        !key.compare("mig_uuid") )
        {
            dict[key.c_str()] = py::cast(*(string*)value);
        }
        else if(!key.compare("freq_history") ){
            std::vector<std::tuple<long long,double>>* val = (std::vector<std::tuple<long long,double>>*)value;
            py::dict freq_dict;
             for(auto [ ts,freq ] : *val){
                 freq_dict[py::cast(ts)] = py::cast(freq);
             }
             dict[key.c_str()] = freq_dict;
        }
        else if(!key.compare("GPU_Clock_Rate")){
            auto [ freq, unit ] = *(std::tuple<double, std::string>*)value;
            py::dict freq_dict;
            freq_dict[py::str("freq")] = py::cast(freq);
            freq_dict[py::str("unit")] = py::cast(unit);       
            dict[key.c_str()] = freq_dict;
        }
        else{
            //try to cast into py::object
           py::object val = py::cast<py::object>(static_cast<PyObject*>(value));
           val.dec_ref();
           dict[key.c_str()] = val; 
        }
    }
    for (auto const& [key, value] : object_attributes) {
        value.inc_ref();
        PyObject * obj = value.ptr();
        attributes[py::cast<std::string>(key)] = static_cast<void*>(obj);
        delete &attributes;
        //if value is int
        // py::type value_type = py::type::of(value);
        // if(value_type.is(py::type::of(py::int_()))){
        //     attributes[py::cast<std::string>(key)] = (void*) new int (py::cast<int>(value));
        // }
        // //if value is long
        // else if(py::isinstance<py::long_(value)){
        //     attributes[py::cast<std::string>(key)] = (void*) new long (py::cast<long>(value));
        // }
        // //if value is float
        // else if(py::isinstance<py::float_(value)){
        //     attributes[py::cast<std::string>(key)] = (void*) new float (py::cast<float>(value));
        // }
        // //if value is double
        // else if(py::isinstance<py::double_(value)){
        //     attributes[py::cast<std::string>(key)] = (void*) new double (py::cast<double>(value));
        // }
        // //if value is bool
        // else if(py::isinstance<py::object>(value) && py::cast<bool>(value)){
        //     attributes[py::cast<std::string>(key)] = (void*) new bool (py::cast<bool>(value));
        // }
        // //else cast to string
        // else {
        //     attributes[py::cast<std::string>(key)] = (void*) new std::string (py::cast<std::string>(value));
        // }
    }
    return dict;
}


PYBIND11_MODULE(sys_sage, m) {

        m.attr("COMPONENT_NONE") = SYS_SAGE_COMPONENT_NONE;
        m.attr("COMPONENT_THREAD") = SYS_SAGE_COMPONENT_THREAD;
        m.attr("COMPONENT_CORE") = SYS_SAGE_COMPONENT_CORE;
        m.attr("COMPONENT_CACHE") = SYS_SAGE_COMPONENT_CACHE;
        m.attr("COMPONENT_SUBDIVISION") = SYS_SAGE_COMPONENT_SUBDIVISION;
        m.attr("COMPONENT_NUMA") = SYS_SAGE_COMPONENT_NUMA;
        m.attr("COMPONENT_CHIP") = SYS_SAGE_COMPONENT_CHIP;
        m.attr("COMPONENT_MEMORY") = SYS_SAGE_COMPONENT_MEMORY;
        m.attr("COMPONENT_STORAGE") = SYS_SAGE_COMPONENT_STORAGE;
        m.attr("COMPONENT_NODE") = SYS_SAGE_COMPONENT_NODE;
        m.attr("COMPONENT_TOPOLOGY") = SYS_SAGE_COMPONENT_TOPOLOGY;

        m.attr("SUBDIVISION_TYPE_NONE") = SYS_SAGE_SUBDIVISION_TYPE_NONE;
        m.attr("SUBDIVISION_TYPE_GPU_SM") = SYS_SAGE_SUBDIVISION_TYPE_GPU_SM;

        m.attr("CHIP_TYPE_NONE") = SYS_SAGE_CHIP_TYPE_NONE;
        m.attr("CHIP_TYPE_CPU") = SYS_SAGE_CHIP_TYPE_CPU;
        m.attr("CHIP_TYPE_CPU_SOCKET") = SYS_SAGE_CHIP_TYPE_CPU_SOCKET;
        m.attr("CHIP_TYPE_GPU") = SYS_SAGE_CHIP_TYPE_GPU;

        m.attr("DATAPATH_NONE") = SYS_SAGE_DATAPATH_NONE;
        m.attr("DATAPATH_OUTGOING") = SYS_SAGE_DATAPATH_OUTGOING;
        m.attr("DATAPATH_INCOMING") = SYS_SAGE_DATAPATH_INCOMING;
        m.attr("DATAPATH_BIDIRECTIONAL") = SYS_SAGE_DATAPATH_BIDIRECTIONAL;
        m.attr("DATAPATH_ORIENTED") = SYS_SAGE_DATAPATH_ORIENTED;
        m.attr("DATAPATH_TYPE_NONE") = SYS_SAGE_DATAPATH_TYPE_NONE;
        m.attr("DATAPATH_TYPE_LOGICAL") = SYS_SAGE_DATAPATH_TYPE_LOGICAL;
        m.attr("DATAPATH_TYPE_PHYSICAL") = SYS_SAGE_DATAPATH_TYPE_PHYSICAL;
        m.attr("DATAPATH_TYPE_L3CAT") = SYS_SAGE_DATAPATH_TYPE_L3CAT;
        m.attr("DATAPATH_TYPE_MIG") = SYS_SAGE_DATAPATH_TYPE_MIG;
        m.attr("DATAPATH_TYPE_DATATRANSFER") = SYS_SAGE_DATAPATH_TYPE_DATATRANSFER;
        m.attr("DATAPATH_TYPE_C2C") = SYS_SAGE_DATAPATH_TYPE_C2C;

        m.def("test_fcn_integration", [](py::function f, int x, int y) { return f(x, y); });

    //bind component class
    py::class_<Component, std::unique_ptr<Component, py::nodelete>>(m, "Component", py::dynamic_attr(),"Generic Component")
       
        //.def(py::init<int, string, int>(), py::arg("id") = 0, py::arg("name") = "unknown", py::arg("componentType") = 1)
        //.def(py::init<Component*, int, string, int>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "unknown", py::arg("componentType") = 1)
        
        .def("syncAttrib",[](Component& self) {
            //TODO: Better use vector instead of dict 
            py::object pyself = py::cast(&self);
            py::dict dict = syncAttributes((self.attrib), pyself.attr("__dict__"));
            for(auto const& [key, value] : dict){
                py::setattr(pyself, key, value);
            }
        })
        .def("__setattr__", [](Component& self, const std::string& name, py::object value) {
            set_attribute(self,name, value);
        })
        .def("__getattr__", [](Component& self, const std::string& name) {
            return get_attribute(self,name);
        })
        .def("__delattr__", [](Component& self, const std::string& name) {
            remove_attribute(self,name);
        })
        // TODO: implement get and set attributes according to ipad sketches 
        //.def("__getattr__", [](Component& self, const std::string& name) {})
        // .def("__setattr__", [](Component& self, const std::string& name, py::object value) {})
        // .def("get_attrib", [](Component& self) {} 
        // .def("set_attrib", [](Component& self, const py::dict& dict) {})
        .def("InsertChild", &Component::InsertChild, py::arg("child"))
        //rename functions
        .def("InsertBetweenParentAndChild", &Component::InsertBetweenParentAndChild, "Insert a component between parent and child")
        .def("InsertBetweenParentAndChildren", &Component::InsertBetweenParentAndChildren, "Insert a component between parent and children")
        .def("RemoveChild", &Component::RemoveChild, "Remove a child component")
        //.def("SetParent", &Component::SetParent, "Set the parent of the component")
        .def_property("parent", &Component::GetParent, &Component::SetParent, "The parent of the component")
        .def("PrintSubtree", (void (Component::*)()) &Component::PrintSubtree, "Print the subtree of the component")
        .def("PrintSubtree", (void (Component::*)(int)) &Component::PrintSubtree, "Print the subtree of the component with a maximum depth of <level>")
        .def("PrintAllDataPathsInSubtree", &Component::PrintAllDataPathsInSubtree, "Print the datapath subtree of the component")
        //.def("GetName", &Component::GetName, "The name of the component")
        //.def("SetName", &Component::SetName, "Set the name of the component")
        .def_property("name", &Component::GetName, &Component::SetName, "The name of the component")
        .def("GetId", &Component::GetId, "The id of the component")
        .def_property_readonly("id", &Component::GetId, "The id of the component")
        //.def("GetComponentType", &Component::GetComponentType, "The type of the component")
        .def_property_readonly("type", &Component::GetComponentType, "The type of the component")
        .def("GetComponentTypeStr", &Component::GetComponentTypeStr, "The type of the component as string")
        .def("GetChildren", &Component::GetChildren, "The children of the component")
        //.def("GetParent", &Component::GetParent, "The parent of the component")
        .def("GetChild", &Component::GetChild, "Like get_child_by_id()")
        .def("GetChildById", &Component::GetChildById, "Get the first child component by id")
        .def("GetChildByType", &Component::GetChildByType, "Get the first child component by type")
        .def("GetAllChildrenByType", (void (Component::*)(vector<Component*> *, int)) &Component::GetAllChildrenByType, "Get all child components by type")
        .def("GetAllChildrenByType", (vector<Component*> (Component::*)(int))(&Component::GetAllChildrenByType), "Get all child components by type")
        .def("GetSubcomponentById", &Component::GetSubcomponentById, "Get the first sub component by id")
        .def("GetAllSubcomponentsByType", (void (Component::*)(vector<Component*> *, int))(&Component::GetAllSubcomponentsByType), "Get the first sub component by type")
        .def("GetAllSubcomponentsByType", (vector<Component*> (Component::*)(int))(&Component::GetAllSubcomponentsByType), "Get all sub components by type")
        .def("CountAllSubcomponents", &Component::CountAllSubcomponents, "Count all sub components")
        .def("CountAllSubcomponentsByType", &Component::CountAllSubcomponentsByType, "Count sub components by type")
        .def("CountChildrenByType", &Component::CountAllChildrenByType, "Count children by type")
        .def("GetAncestorByType", &Component::GetAncestorByType, "Get the first ancestor component by type")
        .def("GetSubtreeDepth", &Component::GetSubtreeDepth, "Get the depth of the subtree")
        .def("GetNthAncestor", &Component::GetNthAncestor, "Get the nth ancestors of the component")
        .def("GetNthDescendents", (void (Component::*)(vector<Component*> *, int))&Component::GetNthDescendents,"Get all the nth descendents of the component")
        .def("GetNthDescendents", (vector<Component*> (Component::*)(int))&Component::GetNthDescendents,"Get all the nth descendents of the component")
        .def("GetSubcomponentsByType", (void (Component::*)(vector<Component*> *, int))&Component::GetSubcomponentsByType,"Get all the sub components of the component by type")
        .def("GetSubcomponentsByType", (vector<Component*> (Component::*)(int))&Component::GetSubcomponentsByType,"Get all the sub components of the component by type")
        .def("GetComponentsInSubtree", (void (Component::*)(vector<Component*> *))&Component::GetComponentsInSubtree,"Get all the components in the subtree of the component")
        .def("GetComponentsInSubtree", (vector<Component*> (Component::*)())&Component::GetComponentsInSubtree,"Get all the components in the subtree of the component")
        .def("GetDataPaths", &Component::GetDataPaths,"Get all the data paths associated with the component")
        .def("AddDataPath", &Component::AddDataPath,"Add a data path to the component")
        .def("GetDataPathByType", &Component::GetDataPathByType,"Get the first data path associated with the component by type")
        .def("GetAllDataPathsByType", (void (Component::*)(vector<DataPath*> *, int, int)) &Component::GetAllDataPathsByType,"Get all the data paths associated with the component by type")
        .def("GetAllDataPathsByType", (vector<DataPath*> (Component::*)(int, int)) &Component::GetAllDataPathsByType,"Get all the data paths associated with the component by type")
        .def("CheckComponentTreeConsistency", &Component::CheckComponentTreeConsistency,"Check if the component tree is consistent")
        .def("GetTopologySize", (int (Component::*)(unsigned*, unsigned*)) (&Component::GetTopologySize),"Get the size of the topology")
        .def("GetDepth", &Component::GetDepth,"Get the depth of the component")
        .def("DeleteDataPath", &Component::DeleteDataPath,"Delete a data path from the component")
        .def("DeleteAllDataPaths", &Component::DeleteAllDataPaths,"Delete all the data paths from the component")
        .def("DeleteSubtree", &Component::DeleteSubtree,"Delete the subtree of the component")
        //.def("Nullcheck", [](Component& self){ return ( == nullptr);})
        .def("Delete", &Component::Delete,"Delete the component")
        .def("StoreAttributes",[](Component& self){
            py::object obj = py::cast(&self);
            obj.inc_ref();
        })
        .def_readwrite("attrib", &Component::attrib)
        .def("__bool__",[](Component& self){
            py::object obj = py::cast(&self);
            //if(obj.get() == nullptr) return false;
            return true;
        });
    py::class_<Topology, std::unique_ptr<Topology, py::nodelete>,Component>(m, "Topology")
        .def(py::init<>());
    py::class_<Node, std::unique_ptr<Node, py::nodelete>, Component>(m, "Node")
        .def(py::init<int, string>(), py::arg("id") = 0, py::arg("name")= "Node")
        .def(py::init<Component*, int, string>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "Node")
        .def("RefreshCpuCoreFrequency", &Node::RefreshCpuCoreFrequency, py::arg("keep_history")=false,"Refresh the cpu core frequency");
    py::class_<Memory,std::unique_ptr<Memory, py::nodelete>, Component>(m, "Memory")
        .def(py::init<long long, bool>(), py::arg("size") = -1, py::arg("isVolatile") = false)
        .def(py::init<Component*,int, string, long long, bool>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "Memory", py::arg("size")=-1, py::arg("isVolatile")=false)
        .def_property("size", &Memory::GetSize, &Memory::SetSize, "The size of the memory")
        .def_property("isVolatile", &Memory::GetIsVolatile, &Memory::SetIsVolatile, "Whether the memory is volatile or not");
    py::class_<Storage, std::unique_ptr<Storage, py::nodelete>, Component>(m, "Storage")
        .def(py::init<long long>(), py::arg("size")=-1)
        .def(py::init<Component*,long long>(), py::arg("parent"), py::arg("size")= -1)
        .def_property("size", &Storage::GetSize, &Storage::SetSize, "The size of the storage");
    py::class_<Chip,std::unique_ptr<Chip, py::nodelete>, Component>(m, "Chip")
        .def(py::init<int,string,int,string,string>(), py::arg("id") = 0, py::arg("name") = "Chip", py::arg("chipType")= 1, py::arg("vendor") = "", py::arg("model") = "", py::return_value_policy::reference)
        .def(py::init<Component*,int,string,int,string,string>(), py::arg("parent"),py::arg("id") = 0, py::arg("name") = "Chip", py::arg("chipType") = 1, py::arg("vendor") = "", py::arg("model") = "")
        .def_property("vendor", &Chip::GetVendor, &Chip::SetVendor, "The vendor of the chip")
        .def_property("model", &Chip::GetModel, &Chip::SetModel, "The model of the chip")
        .def_property("chipType", &Chip::GetChipType, &Chip::SetChipType, "The type of the chip");
    py::class_<Cache, std::unique_ptr<Cache, py::nodelete>, Component>(m, "Cache")
        .def(py::init<int,int,long long, int, int>(), py::arg("id") = 0, py::arg("level") = 0, py::arg("size") = -1, py::arg("associativity") = -1, py::arg("lineSize") = -1)
        .def(py::init<Component*,int,int,long long, int, int>(), py::arg("parent"), py::arg("id") = 0, py::arg("level") = 0, py::arg("size") = -1, py::arg("associativity") = -1, py::arg("lineSize") = -1)
        .def(py::init<Component*, int, string, long long, int, int>(), py::arg("parent"), py::arg("id"), py::arg("cache_type"), py::arg("size") = -1, py::arg("associativity") = -1, py::arg("lineSize") = -1)
        .def_property("cacheLevel", &Cache::GetCacheLevel, &Cache::SetCacheLevel, "The level of the cache")
        .def_property("cacheName", &Cache::GetCacheName, &Cache::SetCacheName, "The name of the cache")
        .def_property("cacheSize", &Cache::GetCacheSize, &Cache::SetCacheSize, "The size of the cache")
        .def_property("cacheAssociativity", &Cache::GetCacheAssociativityWays, &Cache::SetCacheAssociativityWays, "The associativity of the cache")
        .def_property("cacheLineSize", &Cache::GetCacheLineSize, &Cache::SetCacheLineSize, "The line size of the cache");
    py::class_<Subdivision, std::unique_ptr<Subdivision, py::nodelete>, Component>(m, "Subdivision")
        .def(py::init<int,string, int>(), py::arg("id") = 0, py::arg("name") = "Subdivision", py::arg("componentType") = 16)
        .def(py::init<Component*,int,string, int>(), py::arg("parent"), py::arg("id") = 0, py::arg("name") = "Subdivision", py::arg("componentType") = 16)
        .def_property("subdivisionType", &Subdivision::GetSubdivisionType, &Subdivision::SetSubdivisionType, "The type of the subdivision");
    py::class_<Numa,std::unique_ptr<Numa, py::nodelete>, Subdivision>(m, "Numa")
        .def(py::init<int, long long>(), py::arg("id") = 0, py::arg("size") = -1)
        .def(py::init<Component*, int, long long>(), py::arg("parent"), py::arg("id") = 0, py::arg("size") = -1)
        .def_property("size", &Numa::GetSize, &Numa::SetSize, "Size of the NUMA region");
    py::class_<Core, std::unique_ptr<Core, py::nodelete>, Component>(m, "Core")
        .def(py::init<int,string>(),py::arg("id") = 0, py::arg("name") = "Core")
        .def(py::init<Component*,int,string>(),py::arg("parent"),py::arg("id") = 0 ,py::arg("name") = "Core")
        .def("RefreshFreq", &Core::RefreshFreq,py::arg("keep_history") = false,"Refresh the frequency of the component")
        .def_property("freq", &Core::GetFreq, &Core::SetFreq, "Frequency of this core");
        
    py::class_<Thread, std::unique_ptr<Thread, py::nodelete>,Component>(m,"Thread")
        .def(py::init<int,string>(),py::arg("id") = 0,py::arg("name") = "Thread")
        .def(py::init<Component*,int,string>(),py::arg("parent"),py::arg("id") = 0,py::arg("name") = "Thread")
        .def("RefreshFreq", &Thread::RefreshFreq,py::arg("keep_history") = false,"Refresh the frequency of the component")
        .def_property_readonly("freq", &Thread::GetFreq, "Get Frequency of this thread");

    py::class_<DataPath>(m,"DataPath")
        .def(py::init<Component*, Component*, int, int>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("type") = 32)
        .def(py::init<Component*, Component*, int, double, double>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("bw"), py::arg("latency"))
        .def(py::init<Component*, Component*, int, int, double, double>(), py::arg("source"), py::arg("target"), py::arg("oriented"), py::arg("type"), py::arg("bw"), py::arg("latency"))
        .def_property("bandwidth", &DataPath::GetBandwidth, &DataPath::SetBandwidth, "The bandwidth of the data path")
        .def_property("latency", &DataPath::GetLatency, &DataPath::SetLatency, "The latency of the data path")
        .def_property_readonly("type", &DataPath::GetDataPathType, "The type of the data path")
        .def_property_readonly("oriented", &DataPath::GetOrientation, "The orientation of the data path")
        .def_property("source", &DataPath::GetSource, &DataPath::UpdateSource, "The source of the data path")
        .def_property("target", &DataPath::GetTarget, &DataPath::UpdateTarget, "The target of the data path");

    m.def("parseMt4gTopo", (int (*) (Node*,string,int, string)) &parseMt4gTopo, "parseMt4gTopo", py::arg("parent"), py::arg("dataSourcePath"), py::arg("gpuID"), py::arg("delim") = ";");
    m.def("parseMt4gTopo", (int (*) (Component*,string,int, string)) &parseMt4gTopo, "parseMt4gTopo", py::arg("parent"), py::arg("dataSourcePath"), py::arg("gpuID"), py::arg("delim") = ";");
    m.def("parseMt4gTopo", (int (*) (Chip*,string, string)) &parseMt4gTopo, "parseMt4gTopo", py::arg("parent"), py::arg("dataSourcePath"),  py::arg("delim") = ";");

    m.def("parseHwlocOutput", &parseHwlocOutput, "parseHwlocOutput", py::arg("root"), py::arg("xmlPath"));

    m.def("parseCccbenchOutput", &parseCccbenchOutput, "parseCccbenchOutput", py::arg("root"), py::arg("cccPath"));

    m.def("parseCapsNumaBenchmark", &parseCapsNumaBenchmark,  py::arg("root"), py::arg("benchmarkPath"), py::arg("delim") = ";");

    m.def("exportToXml", [](Component& root, string xmlPath, py::function print_att, py::function print_catt) {
        print_attributes = print_att;
        print_complex_attributes = print_catt;
        exportToXml(&root, xmlPath,xmldumper,xmldumper_complex);
    },py::arg("root"), py::arg("xmlPath") = "out.xml", py::arg("print_att") = py::none(), py::arg("print_catt") = py::none());
    
    m.def("importFromXml",[](string path, py::function search_custom_attrib_key_fcn = py::none(), py::function search_custom_complex_attrib_key_fcn = py::none()) {
        read_attributes = search_custom_attrib_key_fcn;
        read_complex_attributes = search_custom_complex_attrib_key_fcn;
        return importFromXml(path,xmlloader,xmlloader_complex);
    }, py::arg("path"), py::arg("search_custom_attrib_key_fcn") = py::none(), py::arg("search_custom_complex_attrib_key_fcn") = py::none());
}


// #endif