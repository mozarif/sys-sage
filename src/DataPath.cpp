#include "DataPath.hpp"

#include <cstdint>
#include <algorithm>
#include <stdexcept>


#include "Component.hpp"

using std::cout;
using std::endl;


////////////////// RELATION //////////////////
sys_sage::Relation::Relation(int _relation_type): type(_relation_type) {}
sys_sage::Relation::Relation(const std::vector<Component*>& components, int _id, bool _ordered, int _relation_type): id(_id), ordered(_ordered), type(_relation_type)
{
    for (Component* c : components) {
        AddComponent(c);
    }
}
sys_sage::Relation::Relation(const std::vector<Component*>& components, int _id, bool _ordered): Relation(components, _id, _ordered, sys_sage::RelationType::Relation) {}
// sys_sage::Relation::Relation(std::initializer_list<Component*> components, int _id, bool _ordered): id(_id), ordered(_ordered), type(sys_sage::RelationType::Relation)
// {
//     for (Component* c : components) {
//         AddComponent(c);
//     }
// }

void sys_sage::Relation::SetId(int _id) {id = _id;}
int sys_sage::Relation::GetId() const{ return id; }
bool sys_sage::Relation::IsOrdered() const{ return ordered; }
bool sys_sage::Relation::ContainsComponent(Component* c) const
{
    if(std::find(components.begin(), components.end(), c) != components.end())
        return true;
    return false;
}
sys_sage::Component* sys_sage::Relation::GetComponent(int index) const
{
    try {
        return components.at(index);
    } catch (const std::out_of_range& e) {
        std::cerr << "Component* sys_sage::Relation::GetComponent(int index) Caught exception: " << e.what() << std::endl;
        return NULL;
    }
}

const std::vector<sys_sage::Component*>& sys_sage::Relation::GetComponents() const { return components; }



void sys_sage::Relation::AddComponent(Component* c)
{
    components.emplace_back(c);
    c->_AddRelation(type, this);
}


void sys_sage::Relation::_PrintRelationComponentInfo() const
{
    std::cout << " -- Components:  ";
    for(Component* c : components)
    {
        cout << "(" << c->GetComponentTypeStr() << ") id " << c->GetId() << ", ";
    }
}
void sys_sage::Relation::_PrintRelationAttrib() const
{
    if(!attrib.empty())
    {
        cout << " -- attrib: ";
        for (const auto& n : attrib) {
            uint64_t* val = (uint64_t*)n.second;
            std::cout << n.first << " = " << *val << "; ";
        }
    }
}
void sys_sage::Relation::Print() const
{
    std::cout << GetTypeStr() << " (" << (ordered?"ordered":"unordered") << ")";
    _PrintRelationComponentInfo();
    _PrintRelationAttrib();
    cout << endl;
}

void sys_sage::Relation::Delete()
{
    for(Component* c : components)
    {
        std::vector<Relation*>& component_relation_vector = c->_GetRelations(type);
        component_relation_vector.erase(std::remove(component_relation_vector.begin(), component_relation_vector.end(), this), component_relation_vector.end());
    }
    delete this;
}
int sys_sage::Relation::GetType() const{ return type;}
std::string sys_sage::Relation::GetTypeStr() const
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

sys_sage::Component * sys_sage::DataPath::GetSource() const {return components[0];}
sys_sage::Component * sys_sage::DataPath::GetTarget() const {return components[1];}
double sys_sage::DataPath::GetBandwidth() const {return bw;}
void sys_sage::DataPath::SetBandwidth(double _bandwidth) { bw = _bandwidth;}
double sys_sage::DataPath::GetLatency() const {return latency;}
void sys_sage::DataPath::SetLatency(double _latency) { latency = _latency; }
int sys_sage::DataPath::GetDataPathType() const {return dp_type;}
int sys_sage::DataPath::GetOrientation() const {return ordered;}


int sys_sage::Relation::UpdateComponent(int index, Component * _new_component)
{
    if(index >= components.size() || index < 0)
    {
        //TODO ho return an integer; 0=okay, 1=this error?
        std::cerr << "WARNING: sys_sage::Relation::UpdateComponent index out of bounds -- nothing updated." << std::endl;
        return 1;
    }
    std::vector<Relation*>& component_relation_vector = components[index]->_GetRelations(type);
    component_relation_vector.erase(std::remove(component_relation_vector.begin(), component_relation_vector.end(), this), component_relation_vector.end());

    _new_component->_AddRelation(type, this);
    components[index] = _new_component;
    return 0;
}
int sys_sage::Relation::UpdateComponent(Component* _old_component, Component * _new_component)
{
    auto it = std::find(components.begin(), components.end(), _old_component);
    if(it == components.end())
    {
        std::cerr << "WARNING: sys_sage::Relation::UpdateComponent component not found -- nothing updated." << std::endl;
        return 1;
    }
    int index = it - components.begin();
    return UpdateComponent(index, _new_component);
}

int sys_sage::DataPath::UpdateSource(Component * _new_source)
{
    return UpdateComponent(0, _new_source);

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

int sys_sage::DataPath::UpdateTarget(Component * _new_target)
{
    return UpdateComponent(1, _new_target);
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

sys_sage::DataPath::DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, sys_sage::DataPathType::type _dp_type): DataPath(_source, _target, _oriented, _dp_type, -1, -1) {}
sys_sage::DataPath::DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, double _bw, double _latency): DataPath(_source, _target, _oriented, sys_sage::DataPathType::None, _bw, _latency) {}
sys_sage::DataPath::DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, sys_sage::DataPathType::type _dp_type, double _bw, double _latency): dp_type(_dp_type), bw(_bw), latency(_latency), Relation(sys_sage::RelationType::DataPath)
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

void sys_sage::DataPath::Delete()
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


void sys_sage::DataPath::Print() const
{
    std::cout << GetTypeStr() << " (" << (ordered?"ordered":"unordered") << ")";
    _PrintRelationComponentInfo();
    cout << " -- bw: " << bw << ", latency: " << latency;
    _PrintRelationAttrib();
    cout << endl;
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
sys_sage::QuantumGate::QuantumGate(size_t _gate_size, const std::vector<Qubit *> & _qubits) : QuantumGate(_gate_size, _qubits, "QuantumGate", 0.0, ""){}
sys_sage::QuantumGate::QuantumGate(size_t _gate_size, const std::vector<Qubit *> & _qubits, std::string _name, double _fidelity, std::string _unitary) : Relation(sys_sage::RelationType::QuantumGate), gate_size(_gate_size), fidelity(_fidelity), unitary(_unitary) 
{ 
    name =_name ;
    for(Qubit* qp : _qubits)
    {
        AddComponent(reinterpret_cast<Component*>(qp));
    }
}
sys_sage::QuantumGate::QuantumGate(const std::vector<Component*>& components, int _id, bool _ordered, size_t _gate_size, std::string _name, int _gate_length, QuantumGateType::type _gate_type, double _fidelity, std::string _unitary) : Relation(components, _id, _ordered, sys_sage::RelationType::QuantumGate), gate_size(_gate_size), name(_name), gate_length(_gate_length), gate_type(_gate_type), fidelity(_fidelity), unitary(_unitary) {}

void sys_sage::QuantumGate::SetGateProperties(std::string _name, double _fidelity, std::string _unitary)
{
    name = _name;
    fidelity = _fidelity;
    unitary = _unitary;
    SetQuantumGateType();
}


void sys_sage::QuantumGate::SetQuantumGateType()
{
    //TODO unite the name in the QuantumGateType namespace, similar to RelationType?
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

sys_sage::QuantumGateType::type sys_sage::QuantumGate::GetQuantumGateType() const { return gate_type; }
double sys_sage::QuantumGate::GetFidelity() const { return fidelity; }
size_t sys_sage::QuantumGate::GetGateSize() const { return gate_size; }
const std::string& sys_sage::QuantumGate::GetUnitary() const { return unitary; }
std::string sys_sage::QuantumGate::GetName() const { return name; }

void sys_sage::QuantumGate::Print() const
{
    std::cout << GetTypeStr() << " (" << (ordered?"ordered":"unordered") << ")";
    _PrintRelationComponentInfo();
    std::cout << " --  Name: " << name << " (type " << QuantumGateType::ToString(gate_type) << "), GateSize: " << gate_size << ", GateLength: " << gate_length << ", Fidelity: " << fidelity;
    _PrintRelationAttrib();
    cout << endl;
}

sys_sage::CouplingMap::CouplingMap(Qubit* q1, Qubit* q2) : Relation(sys_sage::RelationType::CouplingMap)
{
    ordered = true;    
    AddComponent(q1);
    AddComponent(q2);
}
sys_sage::CouplingMap::CouplingMap(const std::vector<Component*>& components, int _id, bool _ordered): Relation(components, _id, _ordered, sys_sage::RelationType::CouplingMap) {}

void sys_sage::CouplingMap::SetFidelity(double _fidelity){fidelity = _fidelity;}
double sys_sage::CouplingMap::GetFidelity() const {return fidelity;}
void sys_sage::CouplingMap::Delete()
{
    Relation::Delete();
}

