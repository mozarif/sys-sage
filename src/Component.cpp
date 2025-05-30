#include "Component.hpp"

#include <algorithm>

using std::string;
using std::vector;
using std::cout;
using std::endl;

void sys_sage::Component::PrintSubtree() const { _PrintSubtree(0); }
void sys_sage::Component::_PrintSubtree(int level) const
{
    //TODO maybe print more info based on component type? (override this function?)
    //cout << "---PrintSubtree---" << endl;
    for (int i = 0; i < level; ++i)
        std::cout << "  ";

    cout << GetComponentTypeStr() << " (name " << name << ") id " << id << " - children: " << children.size();
    cout << " level: " << level<<"\n";
    // sleep(2);
    for(Component* child: children)
    {
        //cout << "size of children: " << child->children.size() << "\n";
        child->_PrintSubtree(level + 1);
    }
}
void sys_sage::Component::PrintAllDataPathsInSubtree()
{
    PrintAllRelationsInSubtree(RelationType::DataPath);
    // vector<Component*> subtreeList;
    // GetComponentsInSubtree(&subtreeList);
    // for(Component * c : subtreeList)
    // {   
    //     vector<DataPath*>* dp_in = c->GetDataPaths(SYS_SAGE_DATAPATH_INCOMING);
    //     vector<DataPath*>* dp_out = c->GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING);
    //     if(dp_in->size() > 0 || dp_out->size() > 0 )
    //     {
    //         cout << "DataPaths regarding Component (" << c->GetComponentTypeStr() << ") id " << c->GetId() << endl;
    //         for(DataPath * dp : *dp_out)
    //         {
    //             cout << "    ";
    //             dp->Print();
    //         }
    //         for(DataPath * dp : *dp_in)
    //         {
    //             cout << "    ";
    //             dp->Print();
    //         }
    //     }
    // }
}
void sys_sage::Component::PrintAllRelationsInSubtree(RelationType::type relationType)
{
    vector<Component*> subtreeList;
    GetComponentsInSubtree(&subtreeList);
    for(Component * c : subtreeList)
    {   
        for(RelationType::type rt : RelationType::RelationTypeList)
        {
            if (relationType == rt || relationType == RelationType::Any)
            {
                vector<Relation*> c_relations = c->GetRelations(rt);
                if(c_relations.size() > 0)
                {
                    std::cout << RelationType::ToString(rt) << "s regarding Component (" << c->GetComponentTypeStr() << ") id " << c->GetId() << std::endl;
                    for(Relation * r : c_relations )
                    {
                        cout << "    ";
                        r->Print();
                    }
                }
            }
        }


    }
}

void sys_sage::Component::InsertChild(Component * child)
{
    child->SetParent(this);
    children.push_back(child);
}
int sys_sage::Component::InsertBetweenParentAndChild(Component* parent, Component* child, bool alreadyParentsChild)
{
    //consistency check
    vector<Component*> p_children = parent->_GetChildren();
    if(child->GetParent() != parent){
        if(std::find(p_children.begin(), p_children.end(), child) != p_children.end())
            return 1; //child and parent are not child and parent in the component tree
        else
            return 2; //corrupt component tree -> bad thing
    }
    else{
        if(std::find(p_children.begin(), p_children.end(), child) == p_children.end())
            return 3; //corrupt component tree -> bad thing
    }

    //remove from grandparent's list; set new parent; insert child into the new component's list
    p_children.erase(std::remove(p_children.begin(), p_children.end(), child), p_children.end());
    child->SetParent(this);
    this->InsertChild(child);

    //finally, insert new component to grandparent's children list
    if(!alreadyParentsChild)
    {
        this->SetParent(parent);
        parent->InsertChild(this);
    }

    return 0;
}
int sys_sage::Component::InsertBetweenParentAndChildren(Component* parent, std::vector<Component*> children, bool alreadyParentsChild)
{
    vector<Component*> p_children = parent->_GetChildren();
    for(Component* child: children) //first just check for consistency
    {
        bool isParent = (child->GetParent() == parent);      
        if(std::find(p_children.begin(), p_children.end(), child) == p_children.end()){  //child not listed as parent's child
            if(isParent)
                return 2; //corrupt component tree -> bad thing
            else
                return 1; // just entered a component in the list, which is not a child of the parent
        }
        if(!isParent)
            return 3; //corrupt component tree -> bad thing
    }

    for(Component* child: children) //second time do the actual inserting
    {
        //remove from grandparent's list; set new parent; insert child into the new component's list
        p_children.erase(std::remove(p_children.begin(), p_children.end(), child), p_children.end());
        child->SetParent(this);
        this->InsertChild(child);
    }

    //finally, insert new component to grandparent's children list
    if(!alreadyParentsChild)
    {
        this->SetParent(parent);
        parent->InsertChild(this);
    }
    
    return 0;
}
int sys_sage::Component::RemoveChild(Component * child)
{
    int orig_size = children.size();
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
    return orig_size - children.size();
    //return std::erase(children, child); -- not supported in some compilers
}
sys_sage::Component* sys_sage::Component::GetChild(int _id) const
{
    return GetChildById(_id);
}

sys_sage::Component* sys_sage::Component::GetChildById(int _id) const
{
    for(Component* child: children)
    {
        if(child->id == _id)
            return child;
    }
    return NULL;
}
sys_sage::Component* sys_sage::Component::GetChildByType(int _componentType) const
{
    for(Component* child: children)
    {
        if(child->GetComponentType() == _componentType)
            return child;
    }
    return NULL;
}

std::vector<sys_sage::Component*> sys_sage::Component::GetAllChildrenByType(int _componentType) const
{
    vector<Component*> ret;
    GetAllChildrenByType(&ret, _componentType);
    return ret;
}

void sys_sage::Component::GetAllChildrenByType(std::vector <Component *> *_outArray, int _componentType) const
{
    for(Component * child : children)
    {
        if(child->GetComponentType() == _componentType)
            _outArray->push_back(child);
    }
    return;
}

// int sys_sage::Component::GetNumThreads()
// {
//     if(componentType == sys_sage::ComponentType::Thread)
//         return 1;
//     int numPu = 0;
//     for(Component * child: children)
//     {
//         numPu += child->CountAllSubcomponentsByType(sys_sage::ComponentType::Thread);
//     }
//     return numPu;
// }

int sys_sage::Component::GetSubtreeDepth() const
{
    if(children.empty()) //is leaf
        return 0;
    int maxDepth = 0;
    for(Component* child: children)
    {
        int subtreeDepth = child->GetSubtreeDepth();
        if(subtreeDepth > maxDepth)
            maxDepth = subtreeDepth;
    }
    return maxDepth + 1;
}

sys_sage::Component* sys_sage::Component::GetNthAncestor(int n)
{
    // For cases with incorrect inputs (0 and negative values)
    if (n < 0)
        return nullptr; // n can't be negative
    else if(n == 0)
        return this; // n = 0 means "this"
    
    Component* parent = GetParent();
    
    // No parent means no further ancestors, the search can be stopped here.
    if(parent == nullptr)
        return nullptr; 
    
    // base case
    if(n == 1) 
    {
        return parent;
    }
    return parent->GetNthAncestor(n - 1);
        
}

void sys_sage::Component::GetNthDescendents(std::vector<Component*>* outArray, int depth)
{
    
    if(depth <= 0)
    {   
        outArray->push_back(this);
        // depth++;
        return;
    }
    for(Component* child: children)
    {   
        cout << GetComponentTypeStr() << " (name " << name << ") id " << id << " - children: " << children.size();
        cout << " depth: " << depth<<"\n";
        child->GetNthDescendents(outArray, depth - 1);
    }
    return;
}

std::vector<sys_sage::Component*> sys_sage::Component::GetNthDescendents(int depth)
{
    vector<Component*> outArray;
    GetNthDescendents(&outArray, depth);
    return outArray;
}

void sys_sage::Component::GetSubcomponentsByType(std::vector<Component*>* outArray, int _componentType)
{
    if(_componentType == componentType){
        outArray->push_back(this);
    }
    for(Component* child: children)
    {
        child->GetSubcomponentsByType(outArray, _componentType);
    }
}

std::vector<sys_sage::Component*> sys_sage::Component::GetSubcomponentsByType(int _componentType)
{
    vector<Component*> outArray;
    GetSubcomponentsByType(&outArray, _componentType);
    return outArray;
}

void sys_sage::Component::GetComponentsInSubtree(std::vector<Component*>* outArray)
{
    outArray->push_back(this);
    for(Component * child : children)
    {
        child->GetComponentsInSubtree(outArray);
    }
    return;
}

std::vector<sys_sage::Component*> sys_sage::Component::GetComponentsInSubtree()
{
    vector<Component*> outArray;
    GetComponentsInSubtree(&outArray);
    return outArray;
}

sys_sage::Component* sys_sage::Component::GetSubcomponentById(int _id, int _componentType)
{
    if(componentType == _componentType && id == _id){
        return this;
    }
    for(Component * child : children)
    {
        Component* ret = child->GetSubcomponentById(_id, _componentType);
        if(ret != NULL)
        {
            return ret;
        }
    }
    return NULL;
}

std::vector<sys_sage::Component*> sys_sage::Component::GetAllSubcomponentsByType(int _componentType)
{
    vector<Component*> ret;
    GetAllSubcomponentsByType(&ret, _componentType);
    return ret;
}
void sys_sage::Component::GetAllSubcomponentsByType(std::vector<Component*>* outArray, int _componentType)
{
    if(componentType == _componentType){
        outArray->push_back(this);
    }
    for(Component * child : children)
    {
        child->GetAllSubcomponentsByType(outArray, _componentType);
    }
    return;
}

int sys_sage::Component::CountAllSubcomponents() const
{
    int cnt = children.size();
    for(Component * child : children)
    {
        cnt += child->CountAllSubcomponents();
    }
    return cnt;
}

int sys_sage::Component::CountAllSubcomponentsByType(int _componentType) const
{
    int cnt = 0;
    for(Component * child : children)
    {
        if(child->GetComponentType() == _componentType)
            cnt++;
    }
    for(Component * child : children)
    {
        cnt += child->CountAllSubcomponentsByType(_componentType);
    }
    return cnt;
}

int sys_sage::Component::CountAllChildrenByType(int _componentType) const
{
    int cnt = 0;
    for(Component * child : children)
    {
        if(child->GetComponentType() == _componentType)
            cnt++;
    }

    return cnt;
}

sys_sage::Component* sys_sage::Component::GetAncestorByType(int _componentType)
{
    if(componentType == _componentType){
        return this;
    }
    if(parent != NULL){
        return parent->GetAncestorByType(_componentType);
    }
    return NULL;
}

// void sys_sage::Component::AddDataPath(DataPath* p, int orientation)
// {
//     if(orientation == SYS_SAGE_DATAPATH_OUTGOING)
//         dp_outgoing.push_back(p);
//     if(orientation == SYS_SAGE_DATAPATH_INCOMING)
//         dp_incoming.push_back(p);
// }

void sys_sage::Component::_AddRelation(int32_t relationType, Relation* r)
{
    if(!relations)
        relations = new std::array<std::vector<Relation*>*, RelationType::_num_relation_types>();
    if(!(*relations)[relationType])
        (*relations)[relationType] = new std::vector<Relation*>();
    
    (*relations)[relationType]->push_back(r);
}

sys_sage::DataPath* sys_sage::Component::GetDataPathByType(sys_sage::DataPathType::type  dp_type, sys_sage::DataPathDirection::type direction) const
{
    for(Relation* r: *(*relations)[RelationType::DataPath])
    {
        //either unordered -> check; or orientation is any -> check; or orientation is incoming & DP is incoming or the same outgoing
        if(!r->IsOrdered() || 
            direction == DataPathDirection::Any || 
            (direction == DataPathDirection::Outgoing && r->GetComponent(0) == this) ||  
            (direction == DataPathDirection::Incoming && r->GetComponent(1) == this))
        {
            DataPath* dp = reinterpret_cast<DataPath*>(r);
            if(dp->GetDataPathType() == dp_type)
                return dp;
        }
    }
    return NULL;
}

std::vector<sys_sage::Relation*>& sys_sage::Component::_GetRelations(RelationType::type relationType) const
{
    if (relations &&
        relationType >= 0 && 
        relationType < RelationType::_num_relation_types &&
        (*relations)[relationType]) 
    {
        return *(*relations)[relationType];
    }

    std::vector<Relation*> empty;
    return empty;
}
const std::vector<sys_sage::Relation*>& sys_sage::Component::GetRelations(RelationType::type relationType) const
{
    if (relations &&
        relationType >= 0 && 
        relationType < RelationType::_num_relation_types &&
        (*relations)[relationType]) 
    {
        return *(*relations)[relationType];
    }

    static const std::vector<Relation*> empty;
    return empty;
}
// vector<Relation*>* Component::GetAllRelationsByType(sys_sage::RelationType::type relationType)
// {
//     if(relationType >=0 && relationType < sys_sage::RelationType::_num_relation_types)
//     {
//         if(relations == NULL){
//             std::cout << "relations == NULL" << std::endl;
//             return NULL;
//         }
//         else
//             std::cout << "else    (relations == NULL)" << std::endl;
//         return (*relations)[relationType];
//     }
//     std::cout << "ERROR" << std::endl;
//     //TODO report the error
//     return NULL;  
// }

std::vector<sys_sage::Relation*> sys_sage::Component::GetAllRelationsBy(RelationType::type relationType, int thisComponentPosition) const
{
    vector<Relation*> out_vector;
    for(int curr_rt : RelationType::RelationTypeList)
    {
        if(relationType == RelationType::Any || relationType == curr_rt)
        {
            for(Relation* r : *(*relations)[curr_rt])
            {
                if(!r->IsOrdered() || (r->IsOrdered() && r->GetComponent(thisComponentPosition) == this))
                {
                    out_vector.push_back(r);
                }
            }
        }
    }
    return out_vector;
}

void sys_sage::Component::GetAllDataPathsByType(std::vector<DataPath*>* outDpArr, DataPathType::type dp_type, DataPathDirection::type direction) const
{
    for(Relation* r: *(*relations)[RelationType::DataPath])
    {
        //either unordered -> check; or orientation is any -> check; or orientation is incoming & DP is incoming or the same outgoing
        if(!r->IsOrdered() || 
            direction == DataPathDirection::Any || 
            (direction == DataPathDirection::Outgoing && r->GetComponent(0) == this) ||
            (direction == DataPathDirection::Incoming && r->GetComponent(1) == this))
        {
            DataPath* dp = reinterpret_cast<DataPath*>(r);
            if(dp_type == DataPathType::Any || dp->GetDataPathType() == dp_type)
                outDpArr->push_back(dp);
        }
    }
    // if(orientation & SYS_SAGE_DATAPATH_OUTGOING){
    //     for(DataPath* dp : dp_outgoing){
    //         if(dp->GetDataPathType() == dp_type)
    //             outDpArr->push_back(dp);
    //     }
    // }
    // if(orientation & SYS_SAGE_DATAPATH_INCOMING){
    //     for(DataPath* dp : dp_incoming){
    //         if(dp->GetDataPathType() == dp_type)
    //             outDpArr->push_back(dp);
    //     }
    // }
    return;
}

std::vector<sys_sage::DataPath*> sys_sage::Component::GetAllDataPathsByType(DataPathType::type dp_type, DataPathDirection::type direction) const
{
    vector<DataPath*> outDpArr;
    GetAllDataPathsByType(&outDpArr, dp_type, direction);
    return outDpArr;
}

// vector<DataPath*> sys_sage::Component::GetDataPaths(int orientation)
// {

//     if(orientation == sys_sage::DataPathOrientation::Any)
//         return GetRelations(sys_sage::RelationType::DataPath, -1);
//     else if(orientation == sys_sage::DataPathOrientation::Outgoing)
//         return GetRelations(sys_sage::RelationType::DataPath, 0);
//     else if(orientation == sys_sage::DataPathOrientation::Incoming)
//         return GetRelations(sys_sage::RelationType::DataPath, 1);
//     else
//         return NULL; //TODO should not happen; add some error message?
    
//     // if(orientation == SYS_SAGE_DATAPATH_INCOMING)
//     //     return &dp_incoming;
//     // else if(orientation == SYS_SAGE_DATAPATH_OUTGOING)
//     //     return &dp_outgoing;
//     // else //TODO
//     //     return NULL;
// }


const std::string& sys_sage::Component::GetComponentTypeStr() const
{
    std::string ret(ComponentType::ToString(componentType));
    return ret;
}

// string sys_sage::Component::GetComponentTypeStr()
// {
//     switch(componentType)
//     {
//         case SYS_SAGE_COMPONENT_NONE:
//             return "None";
//         case SYS_SAGE_COMPONENT_THREAD:
//             return "HW_thread";
//         case SYS_SAGE_COMPONENT_CORE:
//             return "Core";
//         case SYS_SAGE_COMPONENT_CACHE:
//             return "Cache";
//         case SYS_SAGE_COMPONENT_SUBDIVISION:
//             return "Subdivision";
//         case SYS_SAGE_COMPONENT_NUMA:
//             return "NUMA";
//         case SYS_SAGE_COMPONENT_CHIP:
//             return "Chip";
//         case SYS_SAGE_COMPONENT_MEMORY:
//             return "Memory";
//         case SYS_SAGE_COMPONENT_STORAGE:
//             return "Storage";
//         case SYS_SAGE_COMPONENT_NODE:
//             return "Node";
//         case SYS_SAGE_COMPONENT_TOPOLOGY:
//             return "Topology";
//         case SYS_SAGE_COMPONENT_QUANTUM_BACKEND:
//             return "Quantum Backend";
//         case SYS_SAGE_COMPONENT_QUBIT:
//             return "Qubit";

//     }
//     return "";
// }

int sys_sage::Component::CheckComponentTreeConsistency() const
{
    int errors = 0;
    for(Component * child : children){
        if(child->GetParent() != this){
            std::cerr << "Component " << child->GetComponentType() << " id " << child->GetName() << " has wrong parent" << std::endl;
            errors++;
        }
    }
    for(Component * child : children){
        errors += child->CheckComponentTreeConsistency();
    }
    return errors;
}

int sys_sage::Component::GetTopologySize(unsigned * out_component_size, unsigned * out_RelationSize) const
{
    return _GetTopologySize(out_component_size, out_RelationSize, NULL);
}

int sys_sage::Component::_GetTopologySize(unsigned * out_component_size, unsigned * out_RelationSize, std::set<Relation*>* countedRelations) const
{
    if(countedRelations == NULL)
        countedRelations = new std::set<Relation*>();

    int component_size = 0;
    switch(componentType)
    {
        case ComponentType::None:
            component_size += sizeof(Component);
        break;
        case ComponentType::Thread:
            component_size += sizeof(Thread);
        break;
        case ComponentType::Core:
            component_size += sizeof(Core);
        break;
        case ComponentType::Cache:
            component_size += sizeof(Cache);
        break;
        case ComponentType::Subdivision:
            component_size += sizeof(Subdivision);
        break;
        case ComponentType::Numa:
            component_size += sizeof(Numa);
        break;
        case ComponentType::Chip:
            component_size += sizeof(Chip);
        break;
        case ComponentType::Memory:
            component_size += sizeof(Memory);
        break;
        case ComponentType::Storage:
            component_size += sizeof(Storage);
        break;
        case ComponentType::Node:
            component_size += sizeof(Node);
        break;
        case ComponentType::Topology:
            component_size += sizeof(Topology);
        break;
    }
    component_size += attrib.size()*(sizeof(std::string)+sizeof(void*)); //TODO improve
    component_size += children.size()*sizeof(Component*);
    //relations -- only counting the vector/array sizes
    if(relations)
    {
        component_size += sizeof(std::array<std::vector<Relation*>*, RelationType::_num_relation_types>);
        for(int i = 0; i<RelationType::_num_relation_types; i++)
        {
            if((*relations)[i] != NULL)
            {
                component_size += sizeof(*(*relations)[i]);
            }
        }
    }
    (*out_component_size) += component_size;

    int relationsSize = 0;


    for(RelationType::type rt : RelationType::RelationTypeList)
    {
        std::vector<Relation*> rv = GetRelations(rt);
        for(Relation* r: rv)
        {
            if(countedRelations->find(r) == countedRelations->end())
            {
                switch(rt)
                {
                    case RelationType::Relation:
                        relationsSize += sizeof(Relation);
                        relationsSize += r->attrib.size() * (sizeof(string)+sizeof(void*)); //TODO improve
                        break;
                    case RelationType::DataPath:
                        relationsSize += sizeof(DataPath);
                        relationsSize += r->attrib.size() * (sizeof(string)+sizeof(void*)); //TODO improve
                        break;
                    case RelationType::QuantumGate:
                        relationsSize += sizeof(QuantumGate);
                        relationsSize += r->attrib.size() * (sizeof(string)+sizeof(void*)); //TODO improve
                        break;
                    case RelationType::CouplingMap:
                        relationsSize += sizeof(CouplingMap);
                        relationsSize += r->attrib.size() * (sizeof(string)+sizeof(void*)); //TODO improve
                        break;
                }
                countedRelations->insert(r);
            }
        }
    }
    (*out_RelationSize) += relationsSize;

    int subtreeSize = 0;
    for(Component * c : children)
    {
        subtreeSize += c->_GetTopologySize(out_component_size, out_RelationSize, countedRelations);
    }

    if(countedRelations != NULL)
        delete countedRelations;
    return component_size + relationsSize + subtreeSize;
}

int sys_sage::Component::GetDepth(bool refresh)
{
    if(refresh)
    {
        depth = 0;
        Component *parent = GetParent();
        while(parent != NULL)
        {
            depth++;
            parent = parent->GetParent();
        }
    }
    
    return depth;
}

void sys_sage::Component::DeleteRelation(Relation * r)
{
    int32_t rt = r->GetType();
    if(rt == RelationType::Relation)
        r->Delete();
    else if(rt == RelationType::DataPath){
        DataPath* dp = reinterpret_cast<DataPath*>(r);
        dp->Delete();
    } else if(rt == RelationType::QuantumGate){
        QuantumGate* qg = reinterpret_cast<QuantumGate*>(r);
        qg->Delete();
    } else if(rt == RelationType::CouplingMap){
        CouplingMap* cm = reinterpret_cast<CouplingMap*>(r);
        cm->Delete();
    } else{ //this should never happen
        std::cout << "ERROR void sys_sage::Component::DeleteRelation(Relation * r)" << std::endl;
        exit(1);
    }    
}

void sys_sage::Component::DeleteAllRelations(RelationType::type relationType)
{
    for(RelationType::type rt : RelationType::RelationTypeList)
    {
        if(relationType == RelationType::Any || relationType == rt)
        {
            while(true)
            {
                vector<Relation*> vec_r = GetRelations(rt);
                if(vec_r.size() > 0)
                {
                    DeleteRelation(vec_r[0]);
                }
                else
                    break;
            }
        }
    }
}

void sys_sage::Component::DeleteDataPath(DataPath * dp)
{
    DeleteRelation(dp);
    // dp->DeleteDataPath();
}

void sys_sage::Component::DeleteAllDataPaths()
{
    DeleteAllRelations(RelationType::DataPath);
    // while(!dp_outgoing.empty())
    // {
    //     DataPath * dp = dp_outgoing.back();
    //     dp->DeleteDataPath();
    // }
    // while(!dp_incoming.empty())
    // {
    //     DataPath * dp = dp_incoming.back();
    //     dp->DeleteDataPath();
    // }
}
void sys_sage::Component::DeleteSubtree() const
{
    while(children.size() > 0)
    {       
        children[0]->Delete(true); // Recursively free children
    }    
    return;
}
void sys_sage::Component::Delete(bool withSubtree)
{
    // Delete subtree and all data paths
    if (withSubtree)
    {
        DeleteSubtree();
    }

    DeleteAllRelations();
    
    //Free all the children
    if(GetParent()!= NULL) 
    {
        Component *myParent = GetParent();
        myParent->RemoveChild(this);
        if (!withSubtree)
        {
            for(Component* child: children)
            {   
                child->SetParent(myParent);
                myParent->InsertChild(child);
            }
        }    
    }
    else //if(GetParent() == NULL && !withSubtree)
    {
        while(children.size() > 0)
        {       
            RemoveChild(children[0]); // Recursively free children
            children[0]->SetParent(NULL);
        }
    }
    // Delete the component itself
    delete this;
}

const std::string& sys_sage::Component::GetName() const {return name;}
void sys_sage::Component::SetName(std::string _name){ name = _name; }
sys_sage::Component* sys_sage::Component::GetParent() const {return parent;}
void sys_sage::Component::SetParent(Component* _parent){parent = _parent;}
const std::vector<sys_sage::Component*>& sys_sage::Component::GetChildren() const {return children;}
std::vector<sys_sage::Component*>& sys_sage::Component::_GetChildren() {return children;}
sys_sage::ComponentType::type sys_sage::Component::GetComponentType() const {return componentType;}

int sys_sage::Component::GetId() const {return id;}

void sys_sage::Storage::SetSize(long long _size){size = _size;} 
long long sys_sage::Storage::GetSize() const{return size;}

const std::string& sys_sage::Chip::GetVendor() const{return vendor;}
void sys_sage::Chip::SetVendor(std::string _vendor){vendor = _vendor;}
const std::string& sys_sage::Chip::GetModel() const{return model;}
void sys_sage::Chip::SetModel(std::string _model){model = _model;}
void sys_sage::Chip::SetChipType(int chipType){type = chipType;}
int sys_sage::Chip::GetChipType() const{return type;}

void sys_sage::Subdivision::SetSubdivisionType(int subdivisionType){type = subdivisionType;}
int sys_sage::Subdivision::GetSubdivisionType() const {return type;}

long long sys_sage::Numa::GetSize() const{return size;}
void sys_sage::Numa::SetSize(long long _size) { size = _size;}

long long sys_sage::Memory::GetSize() const {return size;}
void sys_sage::Memory::SetSize(long long _size) {size = _size;}
bool sys_sage::Memory::GetIsVolatile() const {return is_volatile;}
void sys_sage::Memory::SetIsVolatile(bool _is_volatile) {is_volatile = _is_volatile;}

const std::string& sys_sage::Cache::GetCacheName() const{return cache_type;}
void sys_sage::Cache::SetCacheName(std::string _name) { cache_type = _name;}

int sys_sage::Cache::GetCacheLevel() const{

    std::string extractedDigits = "";
    for (char c : cache_type) {
        // Break, as soon as a digit is found
        if (std::isdigit(c)) {
            extractedDigits += c;
            break;
        }
    }

    if (!extractedDigits.empty()) 
        return stoi(extractedDigits);
    else 
        return 0;
    
}

void sys_sage::Cache::SetCacheLevel(int _cache_level) { cache_type = std::to_string(_cache_level); }
long long sys_sage::Cache::GetCacheSize() const {return cache_size;}
void sys_sage::Cache::SetCacheSize(long long _cache_size){cache_size = _cache_size;}
int sys_sage::Cache::GetCacheLineSize() const{return cache_line_size;}
void sys_sage::Cache::SetCacheLineSize(int _cache_line_size){cache_line_size = _cache_line_size;}
int sys_sage::Cache::GetCacheAssociativityWays() const {return cache_associativity_ways;}
void sys_sage::Cache::SetCacheAssociativityWays(int _associativity) { cache_associativity_ways = _associativity;}



sys_sage::Component::Component(int _id, std::string _name, ComponentType::type _componentType) : id(_id), name(_name), componentType(_componentType)
{
    count = -1;
    SetParent(NULL);
}
sys_sage::Component::Component(Component * parent, int _id, std::string _name, ComponentType::type _componentType) : id(_id), name(_name), componentType(_componentType)
{
    count = -1;
    SetParent(parent);
    if (parent) {
        parent->InsertChild(this);
    }
}
sys_sage::Component::Component(int _id, std::string _name): Component(_id, _name, sys_sage::ComponentType::None) {}
sys_sage::Component::Component(Component * parent, int _id, std::string _name): Component(parent, _id, _name, sys_sage::ComponentType::None) {}

sys_sage::Topology::Topology():Component(0, "sys-sage Topology", sys_sage::ComponentType::Topology){}

sys_sage::Node::Node(int _id, std::string _name):Component(_id, _name, sys_sage::ComponentType::Node){}
sys_sage::Node::Node(Component * parent, int _id, std::string _name):Component(parent, _id, _name, sys_sage::ComponentType::Node){}

sys_sage::Memory::Memory(long long _size, bool _is_volatile):Component(0, "Memory", sys_sage::ComponentType::Memory), size(_size), is_volatile(_is_volatile){}
//Memory::Memory(Component * parent, int _id, string _name, long long _size):Component(parent, _id, _name, SYS_SAGE_COMPONENT_MEMORY), size(_size){}
sys_sage::Memory::Memory(Component * parent, int _id, std::string _name, long long _size, bool _is_volatile):Component(parent, _id, _name, sys_sage::ComponentType::Memory), size(_size), is_volatile(_is_volatile){}


sys_sage::Storage::Storage(long long _size):Component(0, "Storage", sys_sage::ComponentType::Storage), size(_size){}
sys_sage::Storage::Storage(Component * parent, long long _size):Component(parent, 0, "Storage", sys_sage::ComponentType::Storage), size(_size){}

sys_sage::Chip::Chip(int _id, std::string _name, int _type, std::string _vendor, std::string _model):Component(_id, _name, sys_sage::ComponentType::Chip), type(_type), vendor(_vendor), model(_model) {}
sys_sage::Chip::Chip(Component * parent, int _id, std::string _name, int _type, std::string _vendor, std::string _model):Component(parent, _id, _name, sys_sage::ComponentType::Chip), type(_type), vendor(_vendor), model(_model){}

sys_sage::Cache::Cache(int _id, int  _cache_level, long long _cache_size, int _associativity, int _cache_line_size): Component(_id, "Cache", sys_sage::ComponentType::Cache), cache_type(std::to_string(_cache_level)), cache_size(_cache_size), cache_associativity_ways(_associativity), cache_line_size(_cache_line_size){}
sys_sage::Cache::Cache(Component * parent, int _id, std::string _cache_type, long long _cache_size, int _associativity, int _cache_line_size): Component(parent, _id, "Cache", sys_sage::ComponentType::Cache), cache_type(_cache_type), cache_size(_cache_size), cache_associativity_ways(_associativity), cache_line_size(_cache_line_size){}
sys_sage::Cache::Cache(Component * parent, int _id, int _cache_level, long long _cache_size, int _associativity, int _cache_line_size): Cache(parent, _id, std::to_string(_cache_level), _cache_size, _associativity, -1){}

sys_sage::Subdivision::Subdivision(int _id, std::string _name, sys_sage::ComponentType::type _componentType): Component(_id, _name, _componentType) { }
sys_sage::Subdivision::Subdivision(Component * parent, int _id, std::string _name, sys_sage::ComponentType::type _componentType): Component(parent, _id, _name, _componentType) { }
sys_sage::Subdivision::Subdivision(int _id, std::string _name): Component(_id, _name, sys_sage::ComponentType::Subdivision) { }
sys_sage::Subdivision::Subdivision(Component * parent, int _id, std::string _name): Component(parent, _id, _name, sys_sage::ComponentType::Subdivision) { }

sys_sage::Numa::Numa(int _id, long long _size):Subdivision(_id, "Numa", sys_sage::ComponentType::Numa), size(_size){}
sys_sage::Numa::Numa(Component * parent, int _id, long long _size):Subdivision(parent, _id, "Numa", sys_sage::ComponentType::Numa), size(_size){}

sys_sage::Core::Core(int _id, std::string _name):Component(_id, _name, sys_sage::ComponentType::Core){}
sys_sage::Core::Core(Component * parent, int _id, std::string _name):Component(parent, _id, _name, sys_sage::ComponentType::Core){}

sys_sage::Thread::Thread(int _id, std::string _name):Component(_id, _name, sys_sage::ComponentType::Thread){}
sys_sage::Thread::Thread(Component * parent, int _id, std::string _name):Component(parent, _id, _name, sys_sage::ComponentType::Thread){}

sys_sage::QuantumBackend::QuantumBackend(int _id, std::string _name):Component(_id, _name, sys_sage::ComponentType::QuantumBackend){}
sys_sage::QuantumBackend::QuantumBackend(Component * _parent, int _id, std::string _name):Component(_parent, _id, _name, sys_sage::ComponentType::QuantumBackend){}

sys_sage::Qubit::Qubit(int _id, std::string _name):Component(_id, _name, sys_sage::ComponentType::Qubit){}
sys_sage::Qubit::Qubit(Component * parent, int _id, std::string _name):Component(parent, _id, _name, sys_sage::ComponentType::Qubit){}

void sys_sage::QuantumBackend::SetNumQubits(int _num_qubits) { num_qubits = _num_qubits; }

int sys_sage::QuantumBackend::GetNumQubits() const { return num_qubits; }

void sys_sage::QuantumBackend::addGate(QuantumGate *_gate)
{
    gate_types.push_back(_gate);
}

std::vector<sys_sage::QuantumGate*> sys_sage::QuantumBackend::GetGatesBySize(size_t _gate_size) const 
{
    std::vector<QuantumGate*> gates;
    gates.reserve(gate_types.size());
    
    for (QuantumGate * gate : gate_types)
    {
        if(_gate_size == gate->GetGateSize())
            gates.emplace_back(gate);        
    }
    
    return gates;
}

std::vector<sys_sage::QuantumGate*> sys_sage::QuantumBackend::GetGatesByType(size_t _gate_type) const 
{
    std::vector<QuantumGate*> gates;
    gates.reserve(gate_types.size());
    
    for (QuantumGate * gate : gate_types)
    {
        if(_gate_type == gate->GetType())
            gates.emplace_back(gate);        
    }
    
    return gates;
}

std::vector<sys_sage::QuantumGate*> sys_sage::QuantumBackend::GetAllGateTypes() const 
{
    return gate_types;
}

int sys_sage::QuantumBackend::GetNumberofGates() const { return gate_types.size(); }

std::vector<sys_sage::Qubit *> sys_sage::QuantumBackend::GetAllQubits()
{
    auto all_children = GetAllChildrenByType(sys_sage::ComponentType::Qubit);
    std::vector<Qubit *> qubits;
    qubits.reserve(all_children.size());
    
    for (size_t i = 0; i < all_children.size(); ++i)
    {
        Qubit* q = dynamic_cast<Qubit*>(all_children[i]);
        qubits.push_back(q);
    }

    return qubits;
}

// std::set<std::pair<std::uint16_t, std::uint16_t> > QuantumBackend::GetAllCouplingMaps()
// {
//     std::set<std::pair<std::uint16_t, std::uint16_t>> result; 
//     for(auto i = 0; i < num_qubits; ++i)
//     {
//         Qubit* q = dynamic_cast<Qubit*>(GetChild(i));
//         auto coupling_map = q->GetCouplingMapping();
//         for (size_t j = 0; j < coupling_map.size(); ++j)
//         {
//             result.emplace(i, coupling_map[j]);
//         }
//     }

//     return result;
// }

#ifdef QDMI
void sys_sage::QuantumBackend::SetQDMIDevice(QDMI_Device dev)
{
    device = dev;
}

QDMI_Device sys_sage::QuantumBackend::GetQDMIDevice(){ return device; }

#endif



// void Qubit::SetCouplingMapping( const std::vector <NeighbouringQubit> &coupling_mapping, const int &size_coupling_mapping)
// {
//     _coupling_mapping = coupling_mapping;
//     _size_coupling_mapping = coupling_mapping.size();
// }



// const std::vector <Qubit::NeighbouringQubit> & Qubit::GetCouplingMapping() const
// {
//     return _coupling_mapping;
// }

void sys_sage::Qubit::SetProperties(double _t1, double _t2, double _readout_fidelity, double _q1_fidelity, double _readout_length)
{
    t1 = _t1;
    t2 = _t2;
    readout_fidelity = _readout_fidelity;
    q1_fidelity = _q1_fidelity;
    readout_length = _readout_length;

}

double sys_sage::Qubit::GetT1() const { return t1; }    
double sys_sage::Qubit::GetT2() const { return t2; }
double sys_sage::Qubit::GetReadoutFidelity() const { return readout_fidelity; }
double sys_sage::Qubit::Get1QFidelity() const { return q1_fidelity;}
double sys_sage::Qubit::GetReadoutLength() const { return readout_length; }

// const double Qubit::GetWeight() const
// {
//     return _qubit_weight;
// }

//TODO this is a part of the sys-sage FoMaC, not sys-sage core

