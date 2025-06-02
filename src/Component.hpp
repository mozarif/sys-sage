#ifndef COMPONENT
#define COMPONENT

#include <iostream>
#include <vector>
#include <map>
#include <set>

#include "defines.hpp"
#include "enums.hpp"
#include "DataPath.hpp"
#include <libxml/parser.h>




// #define SYS_SAGE_COMPONENT_NONE 1 /**< class Component (do not use normally)*/
// #define SYS_SAGE_COMPONENT_THREAD 2 /**< class Thread */
// #define SYS_SAGE_COMPONENT_CORE 4 /**< class Core */
// #define SYS_SAGE_COMPONENT_CACHE 8 /**< class Cache */
// #define SYS_SAGE_COMPONENT_SUBDIVISION 16 /**< class Subdivision */
// #define SYS_SAGE_COMPONENT_NUMA 32 /**< class Numa */
// #define SYS_SAGE_COMPONENT_CHIP 64 /**< class Chip */
// #define SYS_SAGE_COMPONENT_MEMORY 128 /**< class Memory */
// #define SYS_SAGE_COMPONENT_STORAGE 256 /**< class Storage */
// #define SYS_SAGE_COMPONENT_NODE 512 /**< class Node */
// #define SYS_SAGE_COMPONENT_TOPOLOGY 1024 /**< class Topology */

// /* To-do: Add defines for different quantum backends
// * For e.g.: SYS_SAGE_COMPONENT_NEUTRAL_ATOMS, SYS_SAGE_COMPONENT_SUPERCONDUCTING, etc.

// */
// #define SYS_SAGE_COMPONENT_QUANTUM_BACKEND 2048 /**< class QuantumBackend */
// #define SYS_SAGE_COMPONENT_QUBIT 4096 /**< class Qubit */

// #define SYS_SAGE_SUBDIVISION_TYPE_NONE 1 /**< Generic Subdivision type. */
// #define SYS_SAGE_SUBDIVISION_TYPE_GPU_SM 2 /**< Subdivision type for GPU SMs */

// #define SYS_SAGE_CHIP_TYPE_NONE 1 /**< Generic Chip type. */
// #define SYS_SAGE_CHIP_TYPE_CPU 2 /**< Chip type used for a CPU. */
// #define SYS_SAGE_CHIP_TYPE_CPU_SOCKET 4 /**< Chip type used for one CPU socket. */
// #define SYS_SAGE_CHIP_TYPE_GPU 8 /**< Chip type used for a GPU.*/


///////////////////////////////////////////////////

namespace sys_sage { //forward declaration
    class Topology;

    class Relation;
    class DataPath;
    class QuantumGate;
}


namespace sys_sage {
    //SVTODO make sure parameters such as ComponentType are of the correct type
    //SVTODO split component.hpp and datapath.hpp into different classes

    /**
    Generic class Component - all components inherit from this class, i.e. this class defines attributes and methods common to all components.
    \n Therefore, these can be used universally among all components. Usually, a Component instance would be an instance of one of the child classes, but a generic component (instance of class Component) is also possible.
    */
    class Component {
    public:
        /**
        Generic Component constructor (no automatic insertion in the Component Tree). Usually one of the derived subclasses for different Component Types will be created. Sets:
        @param _id = id, default 0
        @param _name = name, default "unknown"
        @param _componentType = componentType, default sys_sage::ComponentType::None
        */
        //SVDOCTODO
        Component(int _id = 0, std::string _name = "unknown");
        /**
        Generic Component constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Usually one of the derived subclasses for different Component Types will be created. Sets:
        @param parent = the parent 
        @param _id = id, default 0
        @param _name = name, default "unknown"
        @param _componentType = componentType, default sys_sage::ComponentType::None
        */
        //SVDOCTODO
        Component(Component * parent, int _id = 0, std::string _name = "unknown");
        //SVTODO reevaluate the delete vs destructor
        //SVDOCTODO update the doc accordingly to "reevaluate the delete vs destructor"
        /**
        * @private
        * Use Delete() or DeleteSubtree() for deleting and deallocating the components. 
        */
        virtual ~Component() = default;
        /**
        Inserts a Child component to this component (in the Component Tree).
        The child pointer will be inserted at the end of std::vector of children (retrievable through GetChildren(), GetChild(int _id) etc.)
        @param child - a pointer to a Component (or any class instance that inherits from Component).
        @see GetChildren()
        @see GetChild(int _id)
        */
        void InsertChild(Component * child);
        
        /**
         * Inserts this component between a parent and one of its children. The parent component remains the parent, this Component becomes a new child of the parent, and the specified child becomes this component's child.
         * @param parent The parent component to which this component will be inserted as a child.
         * @param child The child component that will become the child of this component and will remain a descendant of the original parent.
         * @param alreadyParentsChild A boolean flag indicating whether this component is already a child of the parent. 
         *        \n If true, the function assumes that this component is already present as a child of the parent and only needs to reassign the specified child.
         *        \n If false, the function will add this component as a new child of the parent after reassigning the specified child.
         * @return 0 on success; 
         *         1 if the child and parent are not child and parent in the component tree; 
         *         2 if the component tree is corrupt (parent is a parent of child but child is not in the parent's children list); 
         *         3 if the component tree is corrupt (parent is not a parent of child but child is in the parent's children list).
        */
        int InsertBetweenParentAndChild(Component* parent, Component* child, bool alreadyParentsChild);
        
        /**
         * Inserts this component between a parent and a (subset of) his children. The parent component remains parent, this Component becomes a new child, and the children become parent's grandchildren.
         * @param parent The parent component to which this component will be inserted as a child.
         * @param children A vector of child components that will become the children of this component and the grandchildren of the original parent.
         * @param alreadyParentsChild A boolean flag indicating whether this component is already a child of the parent. 
         *        \n If true, the function assumes that this component is already present as a child of the parent and only needs to reassign the specified children.
         *        \n If false, the function will add this component as a new child of the parent after reassigning the specified children.
         * @return 0 on success
         *        \n 1 on incompatible parent-children components (one or more children are not parent's children); 
         *        \n 2 on corrupt component tree (parent is a parent of child but child is NOT in children list of parent); 
         *        \n 3 on corrupt component tree (parent is NOT a parent of child but child is in children list of parent)
        */
        int InsertBetweenParentAndChildren(Component* parent, std::vector<Component*> children, bool alreadyParentsChild);
        
        /**
        Removes the passed component from the list of children, without completely deleting (and deallocating) the child itself
        @param child - child to remove
        @return how many elements were deleted (normally, 0 or 1 should be possible)
        */
        int RemoveChild(Component * child);
        /**
        Define a parent to the component. This is usually used when inserting a component in the tree (by calling InsertChild on the parent, and calling SetParent on the child).
        @param parent - a pointer to a Component (or any class instance that inherits from Component).
        @see InsertChild()
        */
        void SetParent(Component* parent);
        /**
        Prints the whole subtree of this component (including the component itself) to stdout. The tree is printed in DFS order, so that the hierarchy can be easily seen. Each child is indented by "  ".
        For each component in the subtree, the following is printed: "<string component type> (name <name>) id <id> - children: <num children>
        */
        void PrintSubtree() const;

        /**
        @private
        Helper function of PrintSubtree();, which ensures the proper indentation. Using PrintSubtree(); is recommended, however this method can be used as well.
        @param level - number of "  " to print out in front of the first component.
        @see PrintSubtree();
        */
        void _PrintSubtree(int level) const;
        /**
         * OBSOLETE. Use PrintAllRelationsInSubtree instead. This function will be removed in the future.
         * 
        Prints to stdout basic information about all DataPaths that go either from or to the components in the subtree.
        \n For each component, all outgoing and incoming DataPaths are printed, i.e. a DataPath may be printed twice.
        */
        [[deprecated("Use PrintAllRelationsInSubtree instead. This function will be removed in the future.")]]
        void PrintAllDataPathsInSubtree();
        //SVDOCTODO
        //SVDOCTODO similar doc to PrintAllDataPathsInSubtree but not obsolete
        void PrintAllRelationsInSubtree(RelationType::type RelationType = RelationType::Any);
        /**
        Returns name of the component.
        @return name
        @see name
        */
        const std::string& GetName() const;
        /**
        Sets name of the component.
        @param _name - name of the component
        @see name
        */
        void SetName(std::string _name);
        /**
        Returns id of the component.
        @return id
        @see id
        */
        int GetId() const;
        /**
        Returns component type of the component. The component type denotes of which class the instance is (Often the components are stored as Component*, even though they are a member of one of the child classes)
        \n SYS_SAGE_COMPONENT_NONE -> class Component
        \n SYS_SAGE_COMPONENT_THREAD -> class Thread
        \n SYS_SAGE_COMPONENT_CORE -> class Core
        \n SYS_SAGE_COMPONENT_CACHE -> class Cache
        \n SYS_SAGE_COMPONENT_SUBDIVISION -> class Subdivision
        \n SYS_SAGE_COMPONENT_NUMA -> class Numa
        \n SYS_SAGE_COMPONENT_CHIP -> class Chip
        \n SYS_SAGE_COMPONENT_MEMORY -> class Memory
        \n SYS_SAGE_COMPONENT_STORAGE -> class Storage
        \n SYS_SAGE_COMPONENT_NODE -> class Node
        \n SYS_SAGE_COMPONENT_TOPOLOGY -> class Topology
        @returns componentType
        @see componentType
        */
        //SVDOCTODO
        sys_sage::ComponentType::type GetComponentType() const;
        /**
        Returns component type in human-readable string.
        \n SYS_SAGE_COMPONENT_NONE -> "None"
        \n SYS_SAGE_COMPONENT_THREAD -> "HW_thread"
        \n SYS_SAGE_COMPONENT_CORE -> "Core"
        \n SYS_SAGE_COMPONENT_CACHE -> "Cache"
        \n SYS_SAGE_COMPONENT_SUBDIVISION -> "Subdivision"
        \n SYS_SAGE_COMPONENT_NUMA -> "NUMA"
        \n SYS_SAGE_COMPONENT_CHIP -> "Chip"
        \n SYS_SAGE_COMPONENT_MEMORY -> "Memory"
        \n SYS_SAGE_COMPONENT_STORAGE -> "Storage"
        \n SYS_SAGE_COMPONENT_NODE -> "Node"
        \n SYS_SAGE_COMPONENT_TOPOLOGY -> "Topology"
        @returns string component type
        @see componentType
        */
        //SVDOCTODO
        std::string GetComponentTypeStr() const;
        /**
        Returns a pointer to std::vector containing all children of the component (empty vector if no children) .
        @returns std::vector<Component *> * with children
        */
        const std::vector<Component*>& GetChildren() const;
        //SVDOCTODO private
        std::vector<Component*>& _GetChildren();
        /**
        Returns Component pointer to parent (or NULL if this component is the root)
        */
        Component* GetParent() const;
        /**
        Identical to GetChildById
        Retrieve a Component* to a child with child.id=_id.
        \n Should there be more children with the same id, the first match will be retrieved (i.e. the one with lower index in the children array.)
        @see GetChildById
        */
        Component* GetChild(int _id) const;

        /**
        Retrieve a Component* to a child with child.id=_id.
        \n Should there be more children with the same id, the first match will be retrieved (i.e. the one with lower index in the children array.)
        */
        Component* GetChildById(int _id) const;

        /**
        Retrieve a Component* to a child matching the given component type.
        \n Should there be more children with the same type, the first match will be retrieved (i.e. the one with lower index in the children array.)
        */
        Component* GetChildByType(int _componentType) const;

        /**
         * Searches for all the children matching the given component type.
         * 
         * @param _componentType - Required type of components
         * @returns A vector of all the children matching the _componentType
        */
        std::vector<Component*> GetAllChildrenByType(int _componentType) const;

        /**
         * Searches for all the children matching the given component type.
         * 
         * @param _componentType - Required type of components
         * @param outArray - output parameter (vector with results)
            \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
            \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
        */
        void GetAllChildrenByType(std::vector<Component *> *_outArray, int _componentType) const;
        /**
        Searches the subtree to find a component with a matching id and componentType, i.e. looks for a certain component with a matching ID. The search is a DFS. The search starts with the calling component.
        \n Returns first occurence that matches these criteria.
        @param _id - the id to look for
        @param _componentType - the component type where to look for the id
        @return Component * matching the criteria. Returns the first match. NULL if no match found
        */
        Component* GetSubcomponentById(int _id, int _componentType);
        
        /**
         * Searches for all the subcomponents (children, their children and so on) matching the given component type.
         * 
         * @param _componentType - Required type of components
         * @param outArray - output parameter (vector with results)
            \n An input is pointer to a std::std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
            \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
        */
        void GetAllSubcomponentsByType(std::vector<Component*>* outArray, int _componentType);
        
        /**
         * Searches for all the subcomponents (children, their children and so on) matching the given component type.
         * 
         * @param _componentType - Required type of components.
         * @returns A vector of all the subcomponents matching the _componentType.
        */
        std::vector<Component*> GetAllSubcomponentsByType(int _componentType);
        
        /**
        Counts number of subcomponents (children, their children and so on).
        @return Returns number of subcomponents.
        */
        int CountAllSubcomponents() const;
        
        /**
        Counts number of subcomponents (children, their children and so on) matching the requested component type.
        @param _componentType - Component type to look for.
        @return Returns number of subcomponents matching the requested component type.
        */
        int CountAllSubcomponentsByType(int _componentType) const;

        /**
        Counts number of children matching the requested component type.

        @param _componentType - Component type to look for.
        @return Returns number of children matching the requested component type.
        */
        int CountAllChildrenByType(int _componentType) const;
        
        /**
        Moves up the tree until a parent of given type.
        @param _componentType - the desired component type
        @return Component * matching the criteria. NULL if no match found
        */
        Component* GetAncestorByType(int _componentType);       
        /**
        Retrieves maximal distance to a leaf (i.e. the depth of the subtree).
        \n 0=leaf, 1=children are leaves, 2=at most children's children are leaves .....
        @return maximal distance to a leaf
        */
        int GetSubtreeDepth() const;//0=empty, 1=1element,...
        
        /**
        Retrieves Nth ancestor, which resides N levels above. 
        \n E.g. if n=1, the parent is retrieved; if n=2, the grandparent is retrieved and so on.
        @param n - how many levels above the tree should be looked.
        @returns The ancestor residing N levels above.
        */
        Component* GetNthAncestor(int n);

        /**
        Retrieves a std::vector of Component pointers, which reside 'depth' levels deeper. The tree is traversed in order as the children are stored in std::vector children.
        \n E.g. if depth=1, only children of the current are retrieved; if depth=2, only children of the children are retrieved..
        @param depth - how many levels down the tree should be looked
        @param outArray - output parameter (vector with results)
            \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
            \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
        */
        void GetNthDescendents(std::vector<Component*>* outArray, int depth);

        /**
        Retrieves a std::vector of Component pointers, which reside 'depth' levels deeper. The tree is traversed in order as the children are stored in the std::vector.
        \n E.g. if depth=1, only children of the current are retrieved; if depth=2, only children of the children are retrieved..
        @param depth - how many levels down the tree should be looked
        @return A std::std::vector<Component*> with the results.
        */
        std::vector<Component*> GetNthDescendents(int depth);

        /**
        Retrieves a std::vector of Component pointers, which reside in the subtree and have a matching type. The tree is traversed DFS in order as the children are stored in each std::vector children.
        @param componentType - componentType
        @param outArray - output parameter (vector with results)
            \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
            \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
        */
        void GetSubcomponentsByType(std::vector<Component*>* outArray, int componentType);

        /**
        Retrieves a std::vector of Component pointers, which reside in the subtree and have a matching type. The tree is traversed DFS in order as the children are stored in the std::vector.
        @param componentType - componentType
        @return A std::vector<Component*> with the results.
        */
        std::vector<Component*> GetSubcomponentsByType(int _componentType);
        
        /** 
        Retrieves a std::vector of Component pointers, which form the subtree (current node and all the subcomponents) of this.
        @param outArray - output parameter (vector with results)
            \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
            \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
        */
        void GetComponentsInSubtree(std::vector<Component*>* outArray);

        /**  
        Retrieves a std::vector of Component pointers, which form the subtree (current node and all the subcomponents) of this.
        @return A std::vector<Component*> with the results.
        */
        std::vector<Component*> GetComponentsInSubtree();

        //SVDOCTODO
        //SVDOCTODO mention FindAllRelationsBy as an alternative
        //SVDOCTODO this one just returns a pointer to the internal structure -- the object already exists and is managed (deleted) by sys-sage
        //SVDOCTODO mention that std::vector<Relation*>& x = _GetRelations(type); returns a reference (to manipulate with the object) and std::vector<Relation*> x = _GetRelations(type); returns a copy
        const std::vector<Relation*>& GetRelations(RelationType::type relationType) const;
        //SVDOCTODO 
        //SVDOCTODO is private, should not be called
        //SVDOCTODO mention that std::vector<Relation*>& x = _GetRelations(type); returns a reference (to manipulate with the object) and std::vector<Relation*> x = _GetRelations(type); returns a copy
        std::vector<Relation*>& _GetRelations(RelationType::type relationType) const;
        //SVDOCTODO 
        //SVDOCTODO is this a good name?
        //SVDOCTODO mention GetRelations as an alternative
        //SVDOCTODO returns a newly-constructed vector, so the user can do anything with it
        //SVDOCTODO this method creates a new vector and fills it with data; returns a new vector
        std::vector<Relation*> GetAllRelationsBy(RelationType::type relationType = RelationType::Any, int thisComponentPosition = -1) const;

        // SVDOCTODO was removed -> use GetAllDataPathsByType insteas with dataType::Any
        // //SVDOCTODO update 
        // //SVDOCTODO changed to return vector<DataPath*> instead of vector<DataPath*>* 
        // /**     
        // Returns the DataPaths of this component according to their orientation.
        // @param orientation - either SYS_SAGE_DATAPATH_OUTGOING or SYS_SAGE_DATAPATH_INCOMING
        // @return Pointer to std::vector<DataPath *> with the result (dp_outgoing on SYS_SAGE_DATAPATH_OUTGOING, or dp_incoming on SYS_SAGE_DATAPATH_INCOMING, otherwise NULL)
        // @see dp_incoming
        // @see dp_outgoing
        // */
        // vector<DataPath*> GetDataPaths(int orientation = sys_sage::DataPathOrientation::Any);
        /**
        @private
        Only called by Relation's int Relation::AddComponent(Component* c), Relation::UpdateComponent
        */
        void _AddRelation(int32_t relationType, Relation* r);

        
        /**
        Retrieves a DataPath * from the list of this component's data paths with matching type and orientation.
        \n The first match is returned -- first SYS_SAGE_DATAPATH_OUTGOING are searched, then SYS_SAGE_DATAPATH_INCOMING.
        @param type - DataPath type (type) to search for
        @param orientation - orientation of the DataPath (SYS_SAGE_DATAPATH_OUTGOING or SYS_SAGE_DATAPATH_INCOMING or a logical or of these)
        @return DataPath pointer to the found data path; NULL if nothing found.
        */
        DataPath* GetDataPathByType(DataPathType::type dp_type, DataPathDirection::type direction = DataPathDirection::Any) const;
        
        //SVDOCTODO new parameters, new name
        /**
        Retrieves all DataPath * from the list of this component's data paths with matching type and orientation.
        Results are returned in std::vector<DataPath*>* outDpArr, where first the matching data paths in dp_outgoing are pushed back, then the ones in dp_incoming.
        @param type - DataPath type (type) to search for.
        @param orientation - orientation of the DataPath (SYS_SAGE_DATAPATH_OUTGOING or SYS_SAGE_DATAPATH_INCOMING or a logical or of these)
        @param outDpArr - output parameter (vector with results)
            \n An input is pointer to a std::vector<DataPath *>, in which the data paths will be pushed. It must be allocated before the call (but does not have to be empty).
            \n The method pushes back the found data paths -- i.e. the data paths(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
        */
        void GetAllDataPaths(std::vector<DataPath*>* outDpArr, DataPathType::type dp_type = DataPathType::Any, DataPathDirection::type direction = DataPathDirection::Any) const;

        //SVDOCTODO new parameters, new name
        /**
        Retrieves all DataPath * from the list of this component's data paths with matching type and orientation.
        Results are returned in a std::vector<DataPath*>*, where first the matching data paths in dp_outgoing are pushed back, then the ones in dp_incoming.
        @param dp_type - DataPath type (dp_type) to search for.
        @param orientation - orientation of the DataPath (SYS_SAGE_DATAPATH_OUTGOING or SYS_SAGE_DATAPATH_INCOMING or a logical or of these)
        @return A std::vector<DataPath*> with the results.
        */
        std::vector<DataPath*> GetAllDataPaths(DataPathType::type dp_type = DataPathType::Any, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
        @brief Checks the consistency of the component tree starting from this component.

        This function verifies that each child component has this component set as its parent.
        It logs an error message for each child that has an incorrect parent and increments the error count.
        The function then recursively checks the consistency of the entire subtree rooted at each child component.

        @return The total number of inconsistencies found in the component tree.
        
        The function returns the total number of errors found in the component tree, including errors in the direct children and any nested descendants.
        */
        int CheckComponentTreeConsistency() const;
        //SVDOCTODO
        /**
        Calculates approximate memory footprint of the subtree of this element (including the relevant data paths).
        @param out_component_size - output parameter (contains the footprint of the component tree elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
        @param out_dataPathSize - output parameter (contains the footprint of the data-path graph elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
        @return The total size in bytes
        */
        int GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize) const;
        /**
        //SVDOCTODO
        @private
        Helper function of int GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize); -- normally you would call this one.
        \n Calculates approximate memory footprint of the subtree of this element (including the relevant data paths). Does not count DataPaths stored in counted_dataPaths.
        @param out_component_size - output parameter (contains the footprint of the component tree elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
        @param out_dataPathSize - output parameter (contains the footprint of the data-path graph elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
        @param counted_dataPaths - std::set<DataPath*>* of data paths that should not be counted
        @return The total size in bytes
        @see GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize);
        */
        int _GetTopologySize(unsigned * out_component_size, unsigned * out_RelationSize, std::set<Relation*>* countedRelations) const;

        /**
         * Retrieves the depth (level) of a component in the topology.
         * @param refresh - Boolean value, if true: recalculate the position (depth) of the component in the tree,
         *                  if false, return the already stored value
         * @return The depth (level) of a component in the topology
         * @see depth
        */
        int GetDepth(bool refresh);

        /**
        @private
        !!Should normally not be used!! Helper function of XML dump generation.
        @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL);
        */
        virtual xmlNodePtr _CreateXmlSubtree();
        
        //SVDOCTODO
        void DeleteRelation(Relation * r);
        /**
         * [[deprecated("Use void DeleteRelation(Relation * r) instead.")]]
        Deletes and de-allocates the DataPath pointer from the list(std::vector) of outgoing and incoming DataPaths of the Components.
        @param dp - DataPath to Delete
        */
        [[deprecated("DeleteDataPath is deprecated. Use void DeleteRelation(Relation * r) instead.")]]
        void DeleteDataPath(DataPath * dp);

        void DeleteAllRelations(RelationType::type relationType = RelationType::Any);
        /**
         * [[deprecated("Use void DeleteAllRelations(int32_t relationType = sys_sage::RelationType::Any) instead.")]]
         * Deletes all DataPaths of this component.
        */
        [[deprecated("DeleteAllDataPaths is deprecated. Use void DeleteAllRelations(int32_t relationType = sys_sage::RelationType::Any) instead.")]]
        void DeleteAllDataPaths();
        /**
        Deletes the whole subtree (all the children) of the component.
        */
        void DeleteSubtree() const;
        /**
        Deletes a component, its children (if withSubtree = true) and all the associated data paths.
        If only the component itself is deleted, its children are inserted into its parent's children list.
        @param withSubtree - if true, the whole subtree is deleted, otherwise only the component itself.
        */
        void Delete(bool withSubtree = true);

        /**
        * A map for storing arbitrary pieces of information or data.
        * - The `key` denotes the name of the attribute.
        * - The `value` points to the data, stored as a `void*`.
        *
        * This data structure is designed to store a wide variety of data types by
        * utilizing pointers to void. Due to its flexibility, it is essential to manage
        * the types and memory allocation/deallocation carefully to avoid issues such
        * as memory leaks or undefined behavior.
        *
        * Usage:
        * 
        * 1. Adding a new key-value pair:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * int* value = new int(42); // Dynamically allocate memory for the value
        * attrib[key] = static_cast<void*>(value); // Store the value in the map
        * ```
        * 
        * 2. Retrieving data from an existing key:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * if (attrib.find(key) != attrib.end()) {
        *     int* retrievedValue = static_cast<int*>(attrib[key]);
        *     std::cout << "Value: " << *retrievedValue << std::endl;
        * } else {
        *     std::cout << "Key not found." << std::endl;
        * }
        * ```
        * 
        * 3. Checking for the existence of a key:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * if (attrib.find(key) != attrib.end()) {
        *     std::cout << "Key exists." << std::endl;
        * } else {
        *     std::cout << "Key does not exist." << std::endl;
        * }
        * ```
        * 
        * 4. Removing a key-value pair and freeing memory:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * if (attrib.find(key) != attrib.end()) {
        *     int* value = static_cast<int*>(attrib[key]);
        *     delete value; // Free the dynamically allocated memory
        *     attrib.erase(key); // Remove the key-value pair from the map
        * }
        * ```
        * 
        * 5. Updating the value for an existing key:
        * 
        * ```cpp
        * std::string key = "exampleKey";
        * if (attrib.find(key) != attrib.end()) {
        *     int* oldValue = static_cast<int*>(attrib[key]);
        *     delete oldValue; // Free the old value
        *     int* newValue = new int(100); // Allocate new value
        *     attrib[key] = static_cast<void*>(newValue); // Update the map
        * }
        * ```
        * 
        * Note:
        * - Proper memory management is crucial when using `void*` pointers. Always ensure
        *   that dynamically allocated memory is freed when no longer needed.
        * - Type safety is not enforced, so it is important to cast pointers to the correct
        *   type when retrieving values from the map.
        */
        std::map<std::string, void*> attrib;
        
    protected:
        /**
        Generic Component constructor (no automatic insertion in the Component Tree). Usually one of the derived subclasses for different Component Types will be created. Sets:
        @param _id = id, default 0
        @param _name = name, default "unknown"
        @param _componentType = componentType, default sys_sage::ComponentType::None
        */
        //SVDOCTODO
        Component(int _id, std::string _name, ComponentType::type _componentType);
        /**
        Generic Component constructor with insertion into the Component Tree as the parent 's child (as long as parent is an existing Component). Usually one of the derived subclasses for different Component Types will be created. Sets:
        @param parent = the parent 
        @param _id = id, default 0
        @param _name = name, default "unknown"
        @param _componentType = componentType, default sys_sage::ComponentType::None
        */
        //SVDOCTODO
        Component(Component * parent, int _id, std::string _name, ComponentType::type _componentType);

        int id; /**< Numeric ID of the component. There is no requirement for uniqueness of the ID, however it is advised to have unique IDs at least in the realm of parent's children. Some tree search functions, which take the id as a search parameter search for first match, so the user is responsible to manage uniqueness in the realm of the search subtree (or should be aware of the consequences of not doing so). Component's ID is set by the constructor, and is retrieved via int GetId(); */
        int depth; /**< Depth (level) of the Component in the Component Tree */
        std::string name; /**< Name of the component (as a std::string). */
        int count{-1}; /**< Can be used to represent multiple Components with the same properties. By default, it represents only 1 component, and is set to -1. */
        //SVDOCTODO old values
        /**
        Component type of the component. The component type denotes of which class the instance is (Often the components are stored as Component*, even though they are a member of one of the child classes)
        \n This attribute is constant, set by the constructor, and READONLY.
        \n SYS_SAGE_COMPONENT_NONE -> class Component
        \n SYS_SAGE_COMPONENT_THREAD -> class Thread
        \n SYS_SAGE_COMPONENT_CORE -> class Core
        \n SYS_SAGE_COMPONENT_CACHE -> class Cache
        \n SYS_SAGE_COMPONENT_SUBDIVISION -> class Subdivision
        \n SYS_SAGE_COMPONENT_NUMA -> class Numa
        \n SYS_SAGE_COMPONENT_CHIP -> class Chip
        \n SYS_SAGE_COMPONENT_MEMORY -> class Memory
        \n SYS_SAGE_COMPONENT_STORAGE -> class Storage
        \n SYS_SAGE_COMPONENT_NODE -> class Node
        \n SYS_SAGE_COMPONENT_TOPOLOGY -> class Topology
        */
        const ComponentType::type componentType;
        std::vector<Component*> children; /**< Contains the list (std::vector) of pointers to children of the component in the component tree. */
        Component* parent { nullptr }; /**< Contains pointer to the parent component in the component tree. If this component is the root, parent will be NULL.*/
        
        
        // vector<DataPath*> dp_incoming; /**< Contains references to data paths that point to this component. @see DataPath */
        // vector<DataPath*> dp_outgoing; /**< Contains references to data paths that point from this component. @see DataPath */
        
        //SVDOCTODO
        std::array<std::vector<Relation*>*, RelationType::_num_relation_types>* relations = nullptr;
    };

} //namespace sys_sage 
#endif //COMPONENT_HPP
