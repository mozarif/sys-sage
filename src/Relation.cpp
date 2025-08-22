#include <algorithm>
#include "Relation.hpp"
#include <iostream>
#include "Component.hpp"

using std::cout;
using std::endl;

sys_sage::Relation::Relation(RelationType::type _relation_type): type(_relation_type) {}
sys_sage::Relation::Relation(const std::vector<Component*>& components, int _id, bool _ordered, RelationType::type _relation_type): ordered(_ordered), id(_id), type(_relation_type)
{
    for (Component* c : components) {
        AddComponent(c);
    }
}
sys_sage::Relation::Relation(const std::vector<Component*>& components, int _id, bool _ordered): Relation(components, _id, _ordered, sys_sage::RelationType::Relation) {}

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
sys_sage::RelationType::type sys_sage::Relation::GetType() const{ return type;}
std::string sys_sage::Relation::GetTypeStr() const
{
    std::string ret(sys_sage::RelationType::ToString(type));
    return ret;
}

int sys_sage::Relation::UpdateComponent(int index, Component * _new_component)
{
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
