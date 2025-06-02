#include "DataPath.hpp"

#include <cstdint>
#include <algorithm>
#include <stdexcept>


#include "Component.hpp"

using std::cout;
using std::endl;


////////////////// RELATION //////////////////

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

