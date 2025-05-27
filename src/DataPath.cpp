#include "DataPath.hpp"

#include <cstdint>
#include <algorithm>
#include <stdexcept>


#include "Component.hpp"

////////////////// RELATION //////////////////
Relation::Relation(int _relation_type): type(_relation_type) {}
Relation::Relation(std::initializer_list<Component*> components, bool _ordered): ordered(_ordered), type(sys_sage::RelationType::Relation)
{
    for (Component* c : components) {
        AddComponent(c);
    }
}

void Relation::SetId(int _id) {id = _id;}
int Relation::GetId(){ return id; }
bool Relation::IsOrdered(){ return ordered; }
bool Relation::ContainsComponent(Component* c)
{
    if(std::find(components.begin(), components.end(), c) != components.end())
        return true;
    return false;
}
Component* Relation::GetComponent(int index)
{
    try {
        return components.at(index);
    } catch (const std::out_of_range& e) {
        std::cerr << "Component* Relation::GetComponent(int index) Caught exception: " << e.what() << std::endl;
        return NULL;
    }
}

const std::vector<Component*>& Relation::GetComponents() const { return components; }



void Relation::AddComponent(Component* c)
{
    components.emplace_back(c);
    c->_AddRelation(type, this);
}

void Relation::Print()
{
    using namespace sys_sage;
    cout << GetTypeStr() << "(" << (ordered?"ordered":"unordered") << ") -- Components:  ";
    for(Component* c : components)
    {
        cout << "(" << c->GetComponentTypeStr() << ") id " << c->GetId() << ", ";
    }
    // //TODO make this implementation nicer; make sure all is printed for DataPaths etc
    // if(type == RelationType::DataPath)
    // {
    //     cout << " - bw: " << bw << ", latency: " << latency;
    // }
    if(!attrib.empty())
    {
        cout << " - attrib: ";
        for (const auto& n : attrib) {
            uint64_t* val = (uint64_t*)n.second;
            std::cout << n.first << " = " << *val << "; ";
        }
    }
    cout << endl;
}

void Relation::Delete()
{
    for(Component* c : components)
    {
        std::vector<Relation*>& component_relation_vector = c->_GetRelations(type);
        component_relation_vector.erase(std::remove(component_relation_vector.begin(), component_relation_vector.end(), this), component_relation_vector.end());
    }
    delete this;
}
int Relation::GetType(){ return type;}
std::string Relation::GetTypeStr()
{
    std::string ret(sys_sage::RelationType::ToString(type));
    return ret;
}

// DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, int _type){
//     DataPath *dp = new DataPath(_source, _target, _oriented, _type, -1, -1);
//     return dp;
// }
// DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, double _bw, double _latency){
//     DataPath *dp = new DataPath(_source,_target,_oriented,SYS_SAGE_DATAPATH_TYPE_NONE,_bw,_latency);
//     return dp;
// }
// DataPath* NewDataPath(Component* _source, Component* _target, int _oriented, int _type, double _bw, double _latency)
// {
//     DataPath* dp = new DataPath(_source, _target, _oriented, _type, _bw, _latency);
//     return dp;
// }

Component * DataPath::GetSource() {return components[0];}
Component * DataPath::GetTarget() {return components[1];}
double DataPath::GetBandwidth() {return bw;}
void DataPath::SetBandwidth(double _bandwidth) { bw = _bandwidth;}
double DataPath::GetLatency() {return latency;}
void DataPath::SetLatency(double _latency) { latency = _latency; }
int DataPath::GetDataPathType() {return dp_type;}
int DataPath::GetOrientation() {return ordered;}


void Relation::UpdateComponent(int index, Component * _new_component)
{
    if(index >= components.size() || index < 0)
    {
        //SVTODO print error and exit/return and do nothing?
    }
    std::vector<Relation*>& component_relation_vector = components[index]->_GetRelations(type);
    component_relation_vector.erase(std::remove(component_relation_vector.begin(), component_relation_vector.end(), this), component_relation_vector.end());

    _new_component->_AddRelation(type, this);
    components[index] = _new_component;
}
void Relation::UpdateComponent(Component* _old_component, Component * _new_component)
{
    auto it = std::find(components.begin(), components.end(), _old_component);
    if(it == components.end())
    {
        //SVTODO NOT FOUND -> handle error and return
    }
    int index = it - components.begin();
    UpdateComponent(index, _new_component);
}

void DataPath::UpdateSource(Component * _new_source)
{
    UpdateComponent(0, _new_source);

    // if(ordered)
    // {
    //     std::vector<DataPath*>* source_dp_outgoing = components[0]->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
    //     source_dp_outgoing->erase(std::remove(source_dp_outgoing->begin(), source_dp_outgoing->end(), this), source_dp_outgoing->end());

    //     _new_source->AddDataPath(this, SYS_SAGE_DATAPATH_OUTGOING);
    // }
    // else
    // {
    //     std::vector<DataPath*>* source_dp_outgoing = components[0]->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
    //     std::vector<DataPath*>* source_dp_incoming = components[0]->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);

    //     source_dp_outgoing->erase(std::remove(source_dp_outgoing->begin(), source_dp_outgoing->end(), this), source_dp_outgoing->end());
    //     source_dp_incoming->erase(std::remove(source_dp_incoming->begin(), source_dp_incoming->end(), this), source_dp_incoming->end());

    //     _new_source->AddDataPath(this, SYS_SAGE_DATAPATH_OUTGOING);
    //     _new_source->AddDataPath(this, SYS_SAGE_DATAPATH_INCOMING);

    // }

    // components[0] = _new_source;

}

void DataPath::UpdateTarget(Component * _new_target)
{
    UpdateComponent(1, _new_target);
    // if(ordered)
    // {
    //     std::vector<DataPath*>* target_dp_incoming = components[1]->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);
    //     target_dp_incoming->erase(std::remove(target_dp_incoming->begin(), target_dp_incoming->end(), this), target_dp_incoming->end());
        
    //     _new_target->AddDataPath(this, SYS_SAGE_DATAPATH_INCOMING);
    // }
    // else
    // {
    //     std::vector<DataPath*>* target_dp_outgoing = components[1]->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
    //     std::vector<DataPath*>* target_dp_incoming = components[1]->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);

    //     target_dp_outgoing->erase(std::remove(target_dp_outgoing->begin(), target_dp_outgoing->end(), this), target_dp_outgoing->end());
    //     target_dp_incoming->erase(std::remove(target_dp_incoming->begin(), target_dp_incoming->end(), this), target_dp_incoming->end());

    //     _new_target->AddDataPath(this, SYS_SAGE_DATAPATH_OUTGOING);
    //     _new_target->AddDataPath(this, SYS_SAGE_DATAPATH_INCOMING);

    // }

    // components[1] = _new_target;

}

DataPath::DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, sys_sage::DataPathType::type _dp_type): DataPath(_source, _target, _oriented, _dp_type, -1, -1) {}
DataPath::DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, double _bw, double _latency): DataPath(_source, _target, _oriented, sys_sage::DataPathType::None, _bw, _latency) {}
DataPath::DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, sys_sage::DataPathType::type _dp_type, double _bw, double _latency): dp_type(_dp_type), bw(_bw), latency(_latency), Relation(sys_sage::RelationType::DataPath)
{
    if(_oriented == sys_sage::DataPathOrientation::NotOriented)
        ordered = false;
    else
        ordered = true;
    
    
    AddComponent(_source);
    AddComponent(_target);

    // if(_oriented == SYS_SAGE_DATAPATH_BIDIRECTIONAL)
    // {
    //     _source->AddDataPath(this, SYS_SAGE_DATAPATH_OUTGOING);
    //     _target->AddDataPath(this, SYS_SAGE_DATAPATH_OUTGOING);
    //     _source->AddDataPath(this, SYS_SAGE_DATAPATH_INCOMING);
    //     _target->AddDataPath(this, SYS_SAGE_DATAPATH_INCOMING);
    // }
    // else if(_oriented == SYS_SAGE_DATAPATH_ORIENTED)
    // {
    //     _source->AddDataPath(this, SYS_SAGE_DATAPATH_OUTGOING);
    //     _target->AddDataPath(this, SYS_SAGE_DATAPATH_INCOMING);
    // }
    // else
    // {
    //     delete this;
    //     //TODO return error
    //     return;
    // }
}

void DataPath::Delete()
{
    Relation::Delete();
    // if(oriented == SYS_SAGE_DATAPATH_BIDIRECTIONAL)
    // {
    //     std::vector<DataPath*>* source_dp_outgoing = source->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
    //     std::vector<DataPath*>* source_dp_incoming = source->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);
    //     std::vector<DataPath*>* target_dp_outgoing = target->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
    //     std::vector<DataPath*>* target_dp_incoming = target->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);

    //     source_dp_outgoing->erase(std::remove(source_dp_outgoing->begin(), source_dp_outgoing->end(), this), source_dp_outgoing->end());
    //     target_dp_outgoing->erase(std::remove(target_dp_outgoing->begin(), target_dp_outgoing->end(), this), target_dp_outgoing->end());
    //     source_dp_incoming->erase(std::remove(source_dp_incoming->begin(), source_dp_incoming->end(), this), source_dp_incoming->end());
    //     target_dp_incoming->erase(std::remove(target_dp_incoming->begin(), target_dp_incoming->end(), this), target_dp_incoming->end());
    // }
    // else if(oriented == SYS_SAGE_DATAPATH_ORIENTED)
    // {
    //     std::vector<DataPath*>* source_dp_outgoing = source->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
    //     std::vector<DataPath*>* target_dp_incoming = target->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);
    //     source_dp_outgoing->erase(std::remove(source_dp_outgoing->begin(), source_dp_outgoing->end(), this), source_dp_outgoing->end());
    //     target_dp_incoming->erase(std::remove(target_dp_incoming->begin(), target_dp_incoming->end(), this), target_dp_incoming->end());
    // }
    // delete this;
}


void DataPath::Print()
{
    Relation::Print();
    // cout << "DataPath src: (" << components[0]->GetComponentTypeStr() << ") id " << components[0]->GetId() << ", target: (" << components[1]->GetComponentTypeStr() << ") id " << components[1]->GetId() << " - bw: " << bw << ", latency: " << latency;
    // if(!attrib.empty())
    // {
    //     cout << " - attrib: ";
    //     for (const auto& n : attrib) {
    //         uint64_t* val = (uint64_t*)n.second;
    //         std::cout << n.first << " = " << *val << "; ";
    //     }
    // }
    // cout << endl;
}


// QuantumGate::QuantumGate(): QuantumGate(1){}
// QuantumGate::QuantumGate(size_t _gate_size) : Relation(sys_sage::RelationType::QuantumGate), gate_size(_gate_size){}
// QuantumGate::QuantumGate(size_t _gate_size, std::string _name, double _fidelity, std::string _unitary) : gate_size(_gate_size), fidelity(_fidelity), unitary(_unitary){ name =_name ;}
QuantumGate::QuantumGate(size_t _gate_size, const std::vector<Qubit *> & _qubits) : QuantumGate(_gate_size, _qubits, "QuantumGate", 0.0, ""){}
QuantumGate::QuantumGate(size_t _gate_size, const std::vector<Qubit *> & _qubits, std::string _name, double _fidelity, std::string _unitary) : Relation(sys_sage::RelationType::QuantumGate), gate_size(_gate_size), fidelity(_fidelity), unitary(_unitary) 
{ 
    name =_name ;
    for(Qubit* qp : _qubits)
    {
        AddComponent(reinterpret_cast<Component*>(qp));
    }
}

void QuantumGate::SetGateProperties(std::string _name, double _fidelity, std::string _unitary)
{
    name = _name;
    fidelity = _fidelity;
    unitary = _unitary;
    SetQuantumGateType();
}


void QuantumGate::SetQuantumGateType()
{
    //TODO unite the name in the QuantumGateType namespace, similar to RelationType?
    using namespace sys_sage;
    if(gate_size == 1)
    {
        if(name == "id") gate_type = QuantumGateType::Id;
        else if(name == "rz") gate_type = QuantumGateType::Rz;
        else if(name == "sx") gate_type = QuantumGateType::Sx;
        else if(name == "x") gate_type = QuantumGateType::X;
        else gate_type = QuantumGateType::Unknown;
    }
    else if(gate_size == 2)
    {
        if(name == "cx") gate_type = QuantumGateType::Cnot;
        else gate_type = QuantumGateType::Unknown;
    }
    else if(gate_size > 2)
    {
        if(name == "toffoli") gate_type = QuantumGateType::Toffoli;
        else gate_type = QuantumGateType::Unknown;
    }
    else
    {
        gate_type = QuantumGateType::Unknown;
    }
}

sys_sage::QuantumGateType::type QuantumGate::GetQuantumGateType()
{
    return gate_type;
}

double QuantumGate::GetFidelity() const
{
    return fidelity;
}

size_t QuantumGate::GetGateSize() const
{
    return gate_size;
}

std::string QuantumGate::GetUnitary() const
{
    return unitary;
}

std::string QuantumGate::GetName()
{
    return name;
}

void QuantumGate::Print()
{
}


// CouplingMap::CouplingMap() {}
CouplingMap::CouplingMap(Qubit* q1, Qubit* q2) : Relation(sys_sage::RelationType::CouplingMap)
{
    ordered = true;    
    AddComponent(q1);
    AddComponent(q2);
}
void CouplingMap::SetFidelity(double _fidelity){fidelity = _fidelity;}
double CouplingMap::GetFidelity(){return fidelity;}
void CouplingMap::Delete()
{
    Relation::Delete();
}

