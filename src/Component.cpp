#include "Component.hpp"

#include <algorithm>

void Component::PrintSubtree() { PrintSubtree(0); }
void Component::PrintSubtree(int level)
{
    
    //cout << "---PrintSubtree---" << endl;
    for (int i = 0; i < level; ++i)
        cout << "  ";

    cout << GetComponentTypeStr() << " (name " << name << ") id " << id << " - children: " << children.size();
    cout << " level: " << level<<"\n";
    // sleep(2);
    for(Component* child: children)
    {
        //cout << "size of children: " << child->children.size() << "\n";
        child->PrintSubtree(level + 1);
    }
}
void Component::PrintAllDataPathsInSubtree()
{
    PrintAllRelationsInSubtree(sys_sage::RelationType::DataPath);
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
void Component::PrintAllRelationsInSubtree(sys_sage::RelationType::type relationType)
{
    using namespace sys_sage;
    vector<Component*> subtreeList;
    GetComponentsInSubtree(&subtreeList);
    for(Component * c : subtreeList)
    {   
        for(RelationType::type rt : RelationType::RelationTypeList)
        {
            if (relationType == rt || relationType == RelationType::Any)
            {
                vector<Relation*>* c_relations = c->GetAllRelationsByType(rt);
                if(c_relations != NULL && c_relations->size() > 0)
                {
                    std::cout << RelationType::to_string(rt) << "s regarding Component (" << c->GetComponentTypeStr() << ") id " << c->GetId() << std::endl;
                    for(Relation * r : *c_relations )
                    {
                        cout << "    ";
                        r->Print();
                    }
                }
            }
        }


    }
}

void Component::InsertChild(Component * child)
{
    child->SetParent(this);
    children.push_back(child);
}
int Component::InsertBetweenParentAndChild(Component* parent, Component* child, bool alreadyParentsChild)
{
    //consistency check
    vector<Component*> * p_children = parent->GetChildren();
    if(child->GetParent() != parent){
        if(std::find(p_children->begin(), p_children->end(), child) != p_children->end())
            return 1; //child and parent are not child and parent in the component tree
        else
            return 2; //corrupt component tree -> bad thing
    }
    else{
        if(std::find(p_children->begin(), p_children->end(), child) == p_children->end())
            return 3; //corrupt component tree -> bad thing
    }

    //remove from grandparent's list; set new parent; insert child into the new component's list
    p_children->erase(std::remove(p_children->begin(), p_children->end(), child), p_children->end());
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
int Component::InsertBetweenParentAndChildren(Component* parent, vector<Component*> children, bool alreadyParentsChild)
{
    vector<Component*> * p_children = parent->GetChildren();
    for(Component* child: children) //first just check for consistency
    {
        bool isParent = (child->GetParent() == parent);      
        if(std::find(p_children->begin(), p_children->end(), child) == p_children->end()){  //child not listed as parent's child
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
        p_children->erase(std::remove(p_children->begin(), p_children->end(), child), p_children->end());
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
int Component::RemoveChild(Component * child)
{
    int orig_size = children.size();
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
    return orig_size - children.size();
    //return std::erase(children, child); -- not supported in some compilers
}
Component* Component::GetChild(int _id)
{
    return GetChildById(_id);
}

Component* Component::GetChildById(int _id)
{
    for(Component* child: children)
    {
        if(child->id == _id)
            return child;
    }
    return NULL;
}
Component* Component::GetChildByType(int _componentType)
{
    for(Component* child: children)
    {
        if(child->GetComponentType() == _componentType)
            return child;
    }
    return NULL;
}

vector<Component*> Component::GetAllChildrenByType(int _componentType)
{
    vector<Component*> ret;
    GetAllChildrenByType(&ret, _componentType);
    return ret;
}

void Component::GetAllChildrenByType(vector <Component *> *_outArray, int _componentType)
{
    for(Component * child : children)
    {
        if(child->GetComponentType() == _componentType)
            _outArray->push_back(child);
    }
    return;
}

int Component::GetNumThreads()
{
    if(componentType == SYS_SAGE_COMPONENT_THREAD)
        return 1;
    int numPu = 0;
    for(Component * child: children)
    {
        numPu += child->CountAllSubcomponentsByType(SYS_SAGE_COMPONENT_THREAD);
    }
    return numPu;
}

int Component::GetSubtreeDepth()
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

Component* Component::GetNthAncestor(int n)
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

void Component::GetNthDescendents(vector<Component*>* outArray, int depth)
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

vector<Component*> Component::GetNthDescendents(int depth)
{
    vector<Component*> outArray;
    GetNthDescendents(&outArray, depth);
    return outArray;
}

void Component::GetSubcomponentsByType(vector<Component*>* outArray, int _componentType)
{
    if(_componentType == componentType){
        outArray->push_back(this);
    }
    for(Component* child: children)
    {
        child->GetSubcomponentsByType(outArray, _componentType);
    }
}

vector<Component*> Component::GetSubcomponentsByType(int _componentType)
{
    vector<Component*> outArray;
    GetSubcomponentsByType(&outArray, _componentType);
    return outArray;
}

void Component::GetComponentsInSubtree(vector<Component*>* outArray)
{
    outArray->push_back(this);
    for(Component * child : children)
    {
        child->GetComponentsInSubtree(outArray);
    }
    return;
}

vector<Component*> Component::GetComponentsInSubtree()
{
    vector<Component*> outArray;
    GetComponentsInSubtree(&outArray);
    return outArray;
}

Component* Component::FindSubcomponentById(int _id, int _componentType)
{
    return GetSubcomponentById(_id, _componentType);
}
Component* Component::GetSubcomponentById(int _id, int _componentType)
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

void Component::FindAllSubcomponentsByType(vector<Component*>* outArray, int _componentType)
{
    GetAllSubcomponentsByType(outArray, _componentType);
}
vector<Component*> Component::GetAllSubcomponentsByType(int _componentType)
{
    vector<Component*> ret;
    GetAllSubcomponentsByType(&ret, _componentType);
    return ret;
}
void Component::GetAllSubcomponentsByType(vector<Component*>* outArray, int _componentType)
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

int Component::CountAllSubcomponents()
{
    int cnt = children.size();
    for(Component * child : children)
    {
        cnt += child->CountAllSubcomponents();
    }
    return cnt;
}

int Component::CountAllSubcomponentsByType(int _componentType)
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

int Component::CountAllChildrenByType(int _componentType)
{
    int cnt = 0;
    for(Component * child : children)
    {
        if(child->GetComponentType() == _componentType)
            cnt++;
    }

    return cnt;
}

Component* Component::FindParentByType(int _componentType)
{
    return GetAncestorByType(_componentType);
}

Component* Component::GetAncestorByType(int _componentType)
{
    if(componentType == _componentType){
        return this;
    }
    if(parent != NULL){
        return parent->GetAncestorByType(_componentType);
    }
    return NULL;
}

// void Component::AddDataPath(DataPath* p, int orientation)
// {
//     if(orientation == SYS_SAGE_DATAPATH_OUTGOING)
//         dp_outgoing.push_back(p);
//     if(orientation == SYS_SAGE_DATAPATH_INCOMING)
//         dp_incoming.push_back(p);
// }

void Component::_AddRelation(int32_t relationType, Relation* r)
{
    std::cout << "_AddRelation r=" << r << " Component id " << id << ", relations=" << relations << ", relationType=" << relationType << std::endl;
    if(!relations)
        relations = new std::array<std::vector<Relation*>*, sys_sage::RelationType::_num_relation_types>();
    if(!(*relations)[relationType])
        (*relations)[relationType] = new std::vector<Relation*>();
    
    std::cout << "    r=" << r << " (*relations)[relationType]=" << (*relations)[relationType] << ", relations=" << relations << std::endl;
    (*relations)[relationType]->push_back(r);
}

DataPath* Component::GetDataPathByType(sys_sage::DataPathType::type  dp_type, sys_sage::DataPathOrientation::type orientation)
{
    using namespace sys_sage;
    for(Relation* r: *(*relations)[RelationType::DataPath])
    {
        //either unordered -> check; or orientation is any -> check; or orientation is incoming & DP is incoming or the same outgoing
        if(!r->IsOrdered() || 
            orientation == DataPathOrientation::Any || 
            (orientation == DataPathOrientation::Outgoing && r->GetComponent(0) == this) ||  
            (orientation == DataPathOrientation::Incoming && r->GetComponent(1) == this))
        {
            DataPath* dp = reinterpret_cast<DataPath*>(r);
            if(dp->GetDataPathType() == dp_type)
                return dp;
        }
    }
    return NULL;
}

vector<Relation*>* Component::GetAllRelationsByType(sys_sage::RelationType::type relationType)
{
    if(relationType >=0 && relationType < sys_sage::RelationType::_num_relation_types)
    {
        if(relations == NULL){
            std::cout << "relations == NULL" << std::endl;
            return NULL;
        }
        else
            std::cout << "else    (relations == NULL)" << std::endl;
        return (*relations)[relationType];
    }
    std::cout << "ERROR" << std::endl;
    //TODO report the error
    return NULL;  
}

vector<Relation*> Component::GetRelations(sys_sage::RelationType::type relationType, int thisComponentPosition)
{
    using namespace sys_sage;
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

void Component::GetAllDataPathsByType(vector<DataPath*>* outDpArr, sys_sage::DataPathType::type dp_type, sys_sage::DataPathOrientation::type orientation)
{
    using namespace sys_sage;

    for(Relation* r: *(*relations)[RelationType::DataPath])
    {
        //either unordered -> check; or orientation is any -> check; or orientation is incoming & DP is incoming or the same outgoing
        if(!r->IsOrdered() || 
            orientation == DataPathOrientation::Any || 
            (orientation == DataPathOrientation::Outgoing && r->GetComponent(0) == this) ||
            (orientation == DataPathOrientation::Incoming && r->GetComponent(1) == this))
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

vector<DataPath*> Component::GetAllDataPathsByType(sys_sage::DataPathType::type dp_type, sys_sage::DataPathOrientation::type orientation)
{
    vector<DataPath*> outDpArr;
    GetAllDataPathsByType(&outDpArr, dp_type, orientation);
    return outDpArr;
}

// vector<DataPath*> Component::GetDataPaths(int orientation)
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

string Component::GetComponentTypeStr()
{
    switch(componentType)
    {
        case SYS_SAGE_COMPONENT_NONE:
            return "None";
        case SYS_SAGE_COMPONENT_THREAD:
            return "HW_thread";
        case SYS_SAGE_COMPONENT_CORE:
            return "Core";
        case SYS_SAGE_COMPONENT_CACHE:
            return "Cache";
        case SYS_SAGE_COMPONENT_SUBDIVISION:
            return "Subdivision";
        case SYS_SAGE_COMPONENT_NUMA:
            return "NUMA";
        case SYS_SAGE_COMPONENT_CHIP:
            return "Chip";
        case SYS_SAGE_COMPONENT_MEMORY:
            return "Memory";
        case SYS_SAGE_COMPONENT_STORAGE:
            return "Storage";
        case SYS_SAGE_COMPONENT_NODE:
            return "Node";
        case SYS_SAGE_COMPONENT_TOPOLOGY:
            return "Topology";
        case SYS_SAGE_COMPONENT_QUANTUM_BACKEND:
            return "Quantum Backend";
        case SYS_SAGE_COMPONENT_QUBIT:
            return "Qubit";

    }
    return "";
}

int Component::CheckComponentTreeConsistency()
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

int Component::GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize)
{
    return _GetTopologySize(out_component_size, out_dataPathSize, NULL);
}

int Component::_GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize, std::set<DataPath*>* counted_dataPaths)
{
    return 0;
    //SVTODO
    // if(counted_dataPaths == NULL)
    //     counted_dataPaths = new std::set<DataPath*>();

    // int component_size = 0;
    // switch(componentType)
    // {
    //     case SYS_SAGE_COMPONENT_NONE:
    //     break;
    //     case SYS_SAGE_COMPONENT_THREAD:
    //         component_size += sizeof(Thread);
    //     break;
    //     case SYS_SAGE_COMPONENT_CORE:
    //         component_size += sizeof(Core);
    //     break;
    //     case SYS_SAGE_COMPONENT_CACHE:
    //         component_size += sizeof(Cache);
    //     break;
    //     case SYS_SAGE_COMPONENT_SUBDIVISION:
    //         component_size += sizeof(Subdivision);
    //     break;
    //     case SYS_SAGE_COMPONENT_NUMA:
    //         component_size += sizeof(Numa);
    //     break;
    //     case SYS_SAGE_COMPONENT_CHIP:
    //         component_size += sizeof(Chip);
    //     break;
    //     case SYS_SAGE_COMPONENT_MEMORY:
    //         component_size += sizeof(Memory);
    //     break;
    //     case SYS_SAGE_COMPONENT_STORAGE:
    //         component_size += sizeof(Storage);
    //     break;
    //     case SYS_SAGE_COMPONENT_NODE:
    //         component_size += sizeof(Node);
    //     break;
    //     case SYS_SAGE_COMPONENT_TOPOLOGY:
    //         component_size += sizeof(Topology);
    //     break;
    // }
    // component_size += attrib.size()*(sizeof(string)+sizeof(void*)); //TODO improve
    // component_size += children.size()*sizeof(Component*);
    // (*out_component_size) += component_size;

    // int dataPathSize = 0;
    // dataPathSize += dp_incoming.size() * sizeof(DataPath*);
    // dataPathSize += dp_outgoing.size() * sizeof(DataPath*);
    // for(auto it = std::begin(dp_incoming); it != std::end(dp_incoming); ++it) {
    //     if(!counted_dataPaths->count((DataPath*)(*it))) {
    //         //cout << "new datapath " << (DataPath*)(*it) << endl;
    //         dataPathSize += sizeof(DataPath);
    //         dataPathSize += (*it)->attrib.size() * (sizeof(string)+sizeof(void*)); //TODO improve
    //         counted_dataPaths->insert((DataPath*)(*it));
    //     }
    // }
    // for(auto it = std::begin(dp_outgoing); it != std::end(dp_outgoing); ++it) {
    //     if(!counted_dataPaths->count((DataPath*)(*it))){
    //         //cout << "new datapath " << (DataPath*)(*it) << endl;
    //         dataPathSize += sizeof(DataPath);
    //         dataPathSize += (*it)->attrib.size() * (sizeof(string)+sizeof(void*)); //TODO improve
    //         counted_dataPaths->insert((DataPath*)(*it));
    //     }
    // }
    // (*out_dataPathSize) += dataPathSize;

    // int subtreeSize = 0;
    // for(auto it = std::begin(children); it != std::end(children); ++it) {
    //     subtreeSize += (*it)->GetTopologySize(out_component_size, out_dataPathSize, counted_dataPaths);
    // }

    // if(counted_dataPaths != NULL)
    //     delete counted_dataPaths;
    // return component_size + dataPathSize + subtreeSize;
}

int Component::GetDepth(bool refresh)
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

void Component::DeleteRelation(Relation * r)
{
    using namespace sys_sage;
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
        std::cout << "ERROR void Component::DeleteRelation(Relation * r)" << std::endl;
        exit(1);
    }    
}

void Component::DeleteAllRelations(sys_sage::RelationType::type relationType)
{
    for(sys_sage::RelationType::type rt : sys_sage::RelationType::RelationTypeList)
    {
        for(Relation* r: *(*relations)[rt])
        {
            DeleteRelation(r);
        }
    }
}

void Component::DeleteDataPath(DataPath * dp)
{
    DeleteRelation(dp);
    // dp->DeleteDataPath();
}

void Component::DeleteAllDataPaths()
{
    DeleteAllRelations(sys_sage::RelationType::DataPath);
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
void Component::DeleteSubtree()
{
    while(children.size() > 0)
    {       
        children[0]->Delete(true); // Recursively free children
    }    
    return;
}
void Component::Delete(bool withSubtree)
{
    // Delete subtree and all data paths
    if (withSubtree)
    {
        DeleteSubtree();
    }
    DeleteAllDataPaths();
    
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

void Component::SetName(string _name){ name = _name; }
Component* Component::GetParent(){return parent;}
void Component::SetParent(Component* _parent){parent = _parent;}
vector<Component*>* Component::GetChildren(){return &children;}
int Component::GetComponentType(){return componentType;}
string Component::GetName(){return name;}
int Component::GetId(){return id;}

void Storage::SetSize(long long _size){size = _size;} 
long long Storage::GetSize(){return size;}

string Chip::GetVendor(){return vendor;}
void Chip::SetVendor(string _vendor){vendor = _vendor;}
string Chip::GetModel(){return model;}
void Chip::SetModel(string _model){model = _model;}
void Chip::SetChipType(int chipType){type = chipType;}
int Chip::GetChipType(){return type;}

void Subdivision::SetSubdivisionType(int subdivisionType){type = subdivisionType;}
int Subdivision::GetSubdivisionType(){return type;}

long long Numa::GetSize(){return size;}
void Numa::SetSize(long long _size) { size = _size;}

long long Memory::GetSize() {return size;}
void Memory::SetSize(long long _size) {size = _size;}

string Cache::GetCacheName(){return cache_type;}
void Cache::SetCacheName(string _name) { cache_type = _name;}

int Cache::GetCacheLevel(){

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

void Cache::SetCacheLevel(int _cache_level) { cache_type = to_string(_cache_level); }
long long Cache::GetCacheSize(){return cache_size;}
void Cache::SetCacheSize(long long _cache_size){cache_size = _cache_size;}
int Cache::GetCacheLineSize(){return cache_line_size;}
void Cache::SetCacheLineSize(int _cache_line_size){cache_line_size = _cache_line_size;}
int Cache::GetCacheAssociativityWays(){return cache_associativity_ways;}
void Cache::SetCacheAssociativityWays(int _associativity) { cache_associativity_ways = _associativity;}

Component::Component(int _id, string _name, int _componentType) : id(_id), name(_name), componentType(_componentType)
{
    count = -1;
    SetParent(NULL);
}
Component::Component(Component * parent, int _id, string _name, int _componentType) : id(_id), name(_name), componentType(_componentType)
{
    count = -1;
    SetParent(parent);
    if (parent) {
        parent->InsertChild(this);
    }
}

Topology::Topology():Component(0, "sys-sage Topology", SYS_SAGE_COMPONENT_TOPOLOGY){}

Node::Node(int _id, string _name):Component(_id, _name, SYS_SAGE_COMPONENT_NODE){}
Node::Node(Component * parent, int _id, string _name):Component(parent, _id, _name, SYS_SAGE_COMPONENT_NODE){}

Memory::Memory():Component(0, "Memory", SYS_SAGE_COMPONENT_MEMORY){}
Memory::Memory(Component * parent, int _id, string _name, long long _size):Component(parent, _id, _name, SYS_SAGE_COMPONENT_MEMORY), size(_size){}

Storage::Storage():Component(0, "Storage", SYS_SAGE_COMPONENT_STORAGE){}
Storage::Storage(Component * parent):Component(parent, 0, "Storage", SYS_SAGE_COMPONENT_STORAGE){}

Chip::Chip(int _id, string _name, int _type):Component(_id, _name, SYS_SAGE_COMPONENT_CHIP), type(_type) {}
Chip::Chip(Component * parent, int _id, string _name, int _type):Component(parent, _id, _name, SYS_SAGE_COMPONENT_CHIP), type(_type){}

Cache::Cache(int _id, int  _cache_level, long long _cache_size, int _associativity, int _cache_line_size): Component(_id, "Cache", SYS_SAGE_COMPONENT_CACHE), cache_type(to_string(_cache_level)), cache_size(_cache_size), cache_associativity_ways(_associativity), cache_line_size(_cache_line_size){}
Cache::Cache(Component * parent, int _id, string _cache_type, long long _cache_size, int _associativity, int _cache_line_size): Component(parent, _id, "Cache", SYS_SAGE_COMPONENT_CACHE), cache_type(_cache_type), cache_size(_cache_size), cache_associativity_ways(_associativity), cache_line_size(_cache_line_size){}
Cache::Cache(Component * parent, int _id, int _cache_level, long long _cache_size, int _associativity, int _cache_line_size): Cache(parent, _id, to_string(_cache_level), _cache_size, _associativity, -1){}

Subdivision::Subdivision(Component * parent, int _id, string _name, int _componentType): Component(parent, _id, _name, _componentType)
{
    //if(_componentType != SYS_SAGE_COMPONENT_SUBDIVISION && componentType != SYS_SAGE_COMPONENT_NUMA)
        //TODO solve this -- this should not happen
}
Subdivision::Subdivision(int _id, string _name, int _componentType): Component(_id, _name, _componentType)
{
    //if(_componentType != SYS_SAGE_COMPONENT_SUBDIVISION && componentType != SYS_SAGE_COMPONENT_NUMA)
        //TODO solve this -- this should not happen
}

Numa::Numa(int _id, long long _size):Subdivision(_id, "Numa", SYS_SAGE_COMPONENT_NUMA), size(_size){}
Numa::Numa(Component * parent, int _id, long long _size):Subdivision(parent, _id, "Numa", SYS_SAGE_COMPONENT_NUMA), size(_size){}

Core::Core(int _id, string _name):Component(_id, _name, SYS_SAGE_COMPONENT_CORE){}
Core::Core(Component * parent, int _id, string _name):Component(parent, _id, _name, SYS_SAGE_COMPONENT_CORE){}

Thread::Thread(int _id, string _name):Component(_id, _name, SYS_SAGE_COMPONENT_THREAD){}
Thread::Thread(Component * parent, int _id, string _name):Component(parent, _id, _name, SYS_SAGE_COMPONENT_THREAD){}

QuantumBackend::QuantumBackend(int _id, string _name):Component(_id, _name, SYS_SAGE_COMPONENT_QUANTUM_BACKEND){}

QuantumBackend::QuantumBackend(Component * _parent, int _id, string _name):Component(_parent, _id, _name, SYS_SAGE_COMPONENT_QUANTUM_BACKEND){}

void QuantumBackend::SetNumQubits(int _num_qubits) { num_qubits = _num_qubits; }

int QuantumBackend::GetNumQubits() const { return num_qubits; }

void QuantumBackend::addGate(QuantumGate *_gate)
{
    gate_types.push_back(_gate);
}

std::vector<QuantumGate*> QuantumBackend::GetGatesBySize(size_t _gate_size) const 
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

std::vector<QuantumGate*> QuantumBackend::GetGatesByType(size_t _gate_type) const 
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

std::vector<QuantumGate*> QuantumBackend::GetAllGateTypes() const 
{
    return gate_types;
}

int QuantumBackend::GetNumberofGates() const { return gate_types.size(); }

std::vector<Qubit *> QuantumBackend::GetAllQubits()
{
    auto all_children = GetAllChildrenByType(SYS_SAGE_COMPONENT_QUBIT);
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
void QuantumBackend::SetQDMIDevice(QDMI_Device dev)
{
    device = dev;
}

QDMI_Device QuantumBackend::GetQDMIDevice(){ return device; }

#endif

Qubit::Qubit(int _id, string _name):Component(_id, _name, SYS_SAGE_COMPONENT_QUBIT){}

Qubit::Qubit(Component * parent, int _id, string _name):Component(parent, _id, _name, SYS_SAGE_COMPONENT_QUBIT){}

// void Qubit::SetCouplingMapping( const std::vector <NeighbouringQubit> &coupling_mapping, const int &size_coupling_mapping)
// {
//     _coupling_mapping = coupling_mapping;
//     _size_coupling_mapping = coupling_mapping.size();
// }



// const std::vector <Qubit::NeighbouringQubit> & Qubit::GetCouplingMapping() const
// {
//     return _coupling_mapping;
// }

void Qubit::SetProperties(double _t1, double _t2, double _readout_fidelity, double _q1_fidelity, double _readout_length)
{
    t1 = _t1;
    t2 = _t2;
    readout_fidelity = _readout_fidelity;
    q1_fidelity = _q1_fidelity;
    readout_length = _readout_length;

}

const double Qubit::GetT1() const { return t1; }    
const double Qubit::GetT2() const { return t2; }
const double Qubit::GetReadoutFidelity() const { return readout_fidelity; }
const double Qubit::Get1QFidelity() const { return q1_fidelity;}
const double Qubit::GetReadoutLength() const { return readout_length; }

// const double Qubit::GetWeight() const
// {
//     return _qubit_weight;
// }

//TODO this is a part of the sys-sage FoMaC, not sys-sage core

