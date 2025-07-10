#include "DataPath.hpp"

#include <cstdint>
#include <algorithm>
#include <stdexcept>

#include "Component.hpp"

using std::cout;
using std::endl;

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
    //if(index >= components.size() || index < 0)
    if (index < 0 || static_cast<size_t>(index) >= components.size())
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
}

int sys_sage::DataPath::UpdateTarget(Component * _new_target)
{
    return UpdateComponent(1, _new_target);
}

sys_sage::DataPath::DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, sys_sage::DataPathType::type _dp_type): DataPath(_source, _target, _oriented, _dp_type, -1, -1) {}
sys_sage::DataPath::DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, double _bw, double _latency): DataPath(_source, _target, _oriented, sys_sage::DataPathType::None, _bw, _latency) {}
sys_sage::DataPath::DataPath(Component* _source, Component* _target, sys_sage::DataPathOrientation::type _oriented, sys_sage::DataPathType::type _dp_type, double _bw, double _latency) : Relation(sys_sage::RelationType::DataPath), dp_type(_dp_type), bw(_bw), latency(_latency)
{
    if(_oriented == sys_sage::DataPathOrientation::Bidirectional)
        ordered = false;
    else
        ordered = true;
    
    
    AddComponent(_source);
    AddComponent(_target);
}

void sys_sage::DataPath::Delete()
{
    Relation::Delete();
}


void sys_sage::DataPath::Print() const
{
    std::cout << GetTypeStr() << " (" << (ordered?"ordered":"unordered") << ")";
    _PrintRelationComponentInfo();
    cout << " -- bw: " << bw << ", latency: " << latency;
    _PrintRelationAttrib();
    cout << endl;
}

