#ifndef COMPONENT
#define COMPONENT

#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <utility>
#include <vector>

#include <sys-sage/defines.hpp>
#include <sys-sage/enums.hpp>
#include <sys-sage/DataPath.hpp>
#include <sys-sage/attribute.hpp>
#include <libxml/parser.h>
#include <nlohmann/json.hpp>

namespace sys_sage { //forward declaration
    class Topology;

    class Relation;
    class DataPath;
    class QuantumGate;
}

namespace sys_sage {
    /**
     * @class Component
     * @brief Generic class for all hardware and logical components in sys-sage.
     *
     * All components inherit from this class, which defines attributes and methods common to all components.
     * This enables a unified interface for tree traversal, querying, and manipulation.
     * Usually, a Component instance is one of the derived subclasses, but a generic Component is also possible and can for instance be used to model custom hardware components.
     */
    class Component {
    public:
        /**
         * @brief Generic Component constructor (no automatic insertion in the Component Tree).
         *        Sets the componentType member to `sys_sage::ComponentType::Generic`.
         *
         * @param _id Numeric ID of the component (default 0)
         * @param _name Name of the component (default "unknown")
         */
        Component(int _id = 0, const std::string &_name = "unknown");

        /**
         * @brief Generic Component constructor with insertion into the Component Tree as the parent's child.
         *        Sets componentType to sys_sage::ComponentType::Generic.
         *
         * @param _parent Pointer to the parent component
         * @param _id Numeric ID of the component (default 0)
         * @param _name Name of the component (default "unknown")
         */
        Component(Component *_parent, int _id = 0, const std::string &_name = "unknown");

        /**
         * @brief Prohibit shallow copies by deleting the implicit copy constructor.
         */
        Component(const Component &) = delete;

        /**
         * @brief Prohibit shallow copies by deleting the implicit copy
         *        assignment operator.
         */
        Component &operator=(const Component &) = delete;

        /**
         * @brief Destructor for components.
         *        Unlinks this component from its parent and children and additionally frees resources.
         *        The destructor does not delete the entire subtree.
         *        Refer to \ref sys_sage::Component::DeleteSubtree() for the latter.
         */
        virtual ~Component();

        /**
         * @brief Deletes the given component and all relations it is associated with.
         *        This assumes that the component and the relations are all HEAP-ALLOCATED.
         *
         * @param comp The component to be deleted.
         */
        static void Delete(Component *comp);

        /**
         * @brief Deletes the whole subtree spanned by the given component and all associated relations.
         *        This assumes that the components in the subtree and the relations are all HEAP-ALLOCATED.
         *
         * @param root The root of the subtree.
         * @param keepRoot If set to `false`, the given component will be deleted as well.
         */
        static void DeleteSubtree(Component *root, bool keepRoot = false);

        /**
         * @brief Inserts a new child at the end of the \ref sys_sage::Component::children vector and sets this component as the child's parent.
         *
         * @param child Pointer to a component.
         *
         * @see GetChildren()
         * @see GetChild(int _id)
         */
        void InsertChild(Component *child);
        
        /**
         * @brief Inserts this component between a parent and one of its children.
         *        The parent component remains the parent, this component becomes a new child of the parent, and the specified child becomes this component's child.
         *
         * @param parent The parent component to which this component will be inserted as a new child.
         * @param child The child component that will become the child of this component and will remain a descendant of the original parent.
         * @param alreadyParentsChild A boolean flag indicating whether this component is already a child of the parent.
         *        \n If true, the function assumes that this component is already present as a child of the parent and only needs to reassign the specified child.
         *        \n If false, the function will add this component as a new child of the parent after reassigning the specified child.
         *
         * @return 0 on success;
         *         \n 1 if the child and parent are not actually child and parent in the Component Tree;
         *         \n 2 if the Component Tree is corrupt (parent is a parent of child but child is not in the parent's children vector);
         *         \n 3 if the Component Tree is corrupt (parent is not a parent of child but child is in the parent's children vector).
        */
        int InsertBetweenParentAndChild(Component* parent, Component* child, bool alreadyParentsChild);
        
        /**
         * @brief Inserts this component between a parent and a (subset of) its children.
         *        The parent component remains parent, this component becomes a new child, and the specified children become the parent's grandchildren.
         *
         * @param parent The parent component to which this component will be inserted as a child.
         * @param children A vector of child components that will become the children of this component and the grandchildren of the original parent.
         * @param alreadyParentsChild A boolean flag indicating whether this component is already a child of the parent.
         *        \n If true, the function assumes that this component is already present as a child of the parent and only needs to reassign the specified children.
         *        \n If false, the function will add this component as a new child of the parent after reassigning the specified children.
         *
         * @return 0 on success;
         *        \n 1 on incompatible parent-children components (one or more children are not the parent's children);
         *        \n 2 on corrupt Component Tree (parent is a parent of the specified children but at least on of them is not in the parernt's children vector);
         *        \n 3 on corrupt Component Tree (parent is not a parent of at least one of the specified children but the children are in the parent's children vector).
        */
        int InsertBetweenParentAndChildren(Component* parent, std::vector<Component*> children, bool alreadyParentsChild);

        /**
         * @brief Removes the passed component from the \ref sys_sage::Component::children, without completely deleting (and deallocating) the child itself.
         *        The child's parent pointer will be set to `nullptr`.
         *
         * @param child Child to remove.
         *
         * @return Number of elements removed (normally 0 or 1).
         */
        int RemoveChild(Component *child);

        /**
         * @brief Set the parent of this component.
         *        This does not add this component to the children vector of the given parent.
         *        Call \ref sys_sage::Component::InsertChild on the parent instead if this is desired.
         *
         * @param parent The new parent.
         *
         * @see InsertChild()
         */
        void SetParent(Component* _parent);

        /**
         * @brief Prints the whole subtree of this component.
         *        The tree is printed in DFS order, so that the hierarchy can be easily seen.
         *        For each component in the subtree, the following is printed: "<string component type> (name <name>) id <id> - children: <num children>
         */
        void PrintSubtree() const;

    private:
        /**
         * @private
         *
         * @brief Helper function for PrintSubtree(), ensures proper indentation.
         *        Using PrintSubtree() is recommended, but this can be used for custom indentation.
         *
         * @param level Number of "  " to print before the component.
         *
         * @see PrintSubtree()
         */
        void _PrintSubtree(int level) const;
    public:

        /**
         * @deprecated Use PrintRelationsInSubtree instead. This function will be removed in the future.
         * @brief Prints all DataPaths that go from or to components in the subtree.
         * @see PrintAllRelationsInSubtree( RelationType::type RelationType = RelationType::Any)
         * @note This function is deprecated and will be removed in the future. Use PrintRelationsInSubtree instead.
         */
        [[ deprecated("Use PrintRelationsInSubtree instead. This function will be removed in the future (used up until version 0.5.2).") ]]
        void PrintAllDataPathsInSubtree();

        /**
         * @brief Prints all Relations in the subtree.
         * @param RelationType Filter by relation type (default: Any)
         */
        [[ deprecated("Use PrintRelationsInSubtree instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void PrintAllRelationsInSubtree(RelationType::type RelationType = RelationType::Any);

        /**
         * @brief Prints all Relations in the subtree.
         *
         * @param relationType Filter by the given RelationType (default: Any).
         */
        void PrintRelationsInSubtree(RelationType::type relationType = RelationType::Any);

        /**
         * @brief Returns the name of the component.
         *
         * @return The name.
         *
         * @see name
         */
        const std::string& GetName() const;

        /**
         * @brief Sets the name of the component.
         *
         * @param _name The new name of the component.
         *
         * @see name
         */
        void SetName(const std::string &_name);

        /**
         * @brief Returns the ID of the component.
         *
         * @return The ID.
         *
         * @see id
         */
        int GetId() const;

        /**
         * @brief Sets the ID of the component.
         *
         * @param _id The new ID of the component.
         *
         * @see id
         */
        void SetId(int _id);

        /**
         * @brief Returns the ComponentType of the component.
         *
         * @return The type of the component.
         *
         * @see componentType
         */
        sys_sage::ComponentType::type GetComponentType() const;

        /**
         * @brief Returns the ComponentType as a human-readable string as defined in \ref sys_sage::ComponentType::names.
         *
         * @return A string representation of the component's type.
         *
         * @see componentType
         */
        const std::string &GetComponentTypeStr() const;

        /**
         * @brief Returns a reference to a constant vector containing all children of the component (empty vector if no children).
         *
         * @return Reference to an immutable vector of children.
         *
         * @see children
         */
        const std::vector<Component*>& GetChildren() const;

        /**
         * @private
         *
         * @brief Returns a reference to the non-const children vector (internal use).
         *
         * @return Reference to a mutable vector of children.
         *
         * @see children
         */
        std::vector<Component*>& _GetChildren();

        /**
         * @brief Returns a pointer to the component's parent (`nullptr` if this component is the root).
         *
         * @return The component' parent.
         *
         * @see parent
         */
        Component* GetParent() const;

        /**
         * @brief Retrieve a child with a specific ID.
         *        Identical to GetChildById.
         *        \n Should there be more children with the same ID, the first match will be retrieved (i.e. the one with lower index in the children vector).
         *
         * @param The target ID.
         *
         * @return The child with the specified ID or `nullptr` if no such child exists.
         *
         * @see GetChildById
         */
        Component* GetChild(int _id) const;

        /**
         * @brief Retrieve a child with a specific ID.
         *        \n Should there be more children with the same ID, the first match will be retrieved (i.e. the one with lower index in the children vector).
         *
         * @param The target ID.
         *
         * @return The child with the specified ID or `nullptr` if no such child exists.
         */
        Component* GetChildById(int _id) const;

        /**
         * @brief Retrieve a child of a specific type.
         *        \n Should there be more children with the same type, the first match will be retrieved (i.e. the one with lower index in the children vector).
         *
         * @param _componentType The ComponentType to match.
         *
         * @return Pointer to the first matching child, or nullptr if not found.
         */
        Component* GetChildByType(ComponentType::type _componentType) const;
        /**
         * @brief Searches for all children matching the given component type.
         * @param _componentType Required type of components
         * @return Vector of all matching children
         */
        [[ deprecated("Use FindChildrenByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Component*> GetAllChildrenByType(ComponentType::type _componentType) const;

        /**
         * @brief Searches for all children matching the given ComponentType.
         *
         * @param _componentType Required ComponentType.
         *
         * @return A new vector of all matching children.
         */
        std::vector<Component*> FindChildrenByType(ComponentType::type _componentType) const;

        /**
         * @brief Searches for all the children matching the given component type.
         * 
         * @param _componentType - Required type of components
         * @param outArray - output parameter (vector with results)
            \n An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
            \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
        */
        [[ deprecated("Use FindChildrenByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void GetAllChildrenByType(std::vector<Component *> *_outArray, ComponentType::type _componentType) const;

        /**
         * @brief Searches for all the children matching the given ComponentType.
         * 
         * @param outArray An output vector to append the matching children to.
         * @param _componentType Required ComponentType.
         */
        void FindChildrenByType(std::vector<Component *> &_outArray, ComponentType::type _componentType) const;

        /**
        * @brief Searches the subtree to find a component with a matching id and componentType, i.e. looks for a certain component with a matching ID. The search is a DFS. The search starts with the calling component.
        * @return Returns first occurence that matches these criteria.
        * @param _id - the id to look for
        * @param _componentType - the component type where to look for the id
        * @return Component * matching the criteria. Returns the first match. NULL if no match found
        */
        [[ deprecated("Use GetDescendantById instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        Component* GetSubcomponentById(int _id, ComponentType::type _componentType);

        /**
         * @brief Looks for a component with a matching ID and ComponentType within the subtree spanned by this component.
         *        The subtree is traversed in DFS order.
         *
         * @param _id The target ID.
         * @param _componentType The target type of the component.
         *
         * @return Returns the first occurence that matches these criteria.
         */
        Component *GetDescendantById(int _id, ComponentType::type _componentType);

        /**
         * @brief Searches for all the subcomponents (children, their children and so on) matching the given component type.
         * 
         * @param _componentType - Required type of components
         * @param outArray - output parameter (vector with results)
            \n An input is pointer to a std::std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
            \n The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
        */
        [[ deprecated("Use FindDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void GetAllSubcomponentsByType(std::vector<Component*>* outArray, ComponentType::type _componentType);

        /**
         * @brief Searches for all the descendants in the subtree matching the given ComponentType.
         * 
         * @param outArray An output vector to append the matching descendants to.
         * @param _componentType - Required ComponentType.
         */
        void FindDescendantsByType(std::vector<Component*> &outArray, ComponentType::type _componentType);

        /**
         * @brief Searches for all the subcomponents (children, their children and so on) matching the given component type.
         * 
         * @param _componentType - Required type of components.
         * @returns A vector of all the subcomponents matching the _componentType.
        */
        [[ deprecated("Use FindDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Component*> GetAllSubcomponentsByType(ComponentType::type _componentType);

        /**
         * @brief Searches for all the descendants in the subtree matching the given ComponentType.
         * 
         * @param _componentType - Required ComponentType.
         *
         * @return A new vector of all matching descendants.
         */
        std::vector<Component*> FindDescendantsByType(ComponentType::type _componentType);

        /**
         * @brief Counts number of subcomponents (children, grandchildren, etc.).
         * @return Number of subcomponents
         */
        [[ deprecated("Use CountDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int CountAllSubcomponents() const;
        
        /**
         * @brief Counts number of subcomponents (children, their children and so on) matching the requested component type.
         * @param _componentType - ComponentType to look for.
         * @return Returns number of subcomponents matching the requested component type.
         */
        [[ deprecated("Use CountDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int CountAllSubcomponentsByType(ComponentType::type _componentType) const;

        /**
         * @brief Counts the number of descendants in the subtree matching the requested ComponentType.
         *
         * @param _componentType The ComponentType to look for.
         *
         * @return Aggregate number of descendants matching the criterion.
         */
        int CountDescendantsByType(ComponentType::type _componentType) const;

        /**
        * @brief Counts number of children matching the requested component type.
        * @param _componentType - ComponentType to look for.
        * @return Returns number of children matching the requested component type.
        */
        [[ deprecated("Use CountChildrenByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int CountAllChildrenByType(ComponentType::type _componentType) const;

        /**
         * @brief Counts the number of children in the subtree matching the requested ComponentType.
         *
         * @param _componentType The ComponentType to look for.
         *
         * @return Aggregate number of children matching the criterion.
         */
        int CountChildrenByType(ComponentType::type _componentType) const;

        /**
         * @brief Moves up the tree until a parent of the given type is found.
         *
         * @param _componentType Desired ComponentType.
         *
         * @return Pointer to the ancestor, or nullptr if not found.
         */
        Component* GetAncestorByType(ComponentType::type _componentType);
        /**
         * @brief Retrieves maximal distance to a leaf (i.e. the depth of the subtree).
         * 0=leaf, 1=children are leaves, 2=at most children's children are leaves .....
         * @return maximal distance to a leaf
         */
        [[ deprecated("Use CalcSubtreeDepth instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int GetSubtreeDepth() const;//0=empty, 1=1element,...

        /**
         * @brief Retrieves maximal distance to a leaf (i.e. the depth of the subtree).
         *
         * @return 0 if the component is a leaf;
         *         \n 1 if children are leaves;
         *         \n 2 if at most children's children are leaves;
         *         \n and so on...
         */
        int CalcSubtreeDepth() const;//0=empty, 1=1element,...

        /**
         * @brief Retrieves the Nth ancestor, which resides N levels above in the tree.
         *
         * @param n How many levels above this component.
         *
         * @returns nullptrs if the number of levels exceeds the distance to the root of the tree;
         *          \n This component if n = 0;
         *          \n The parent if n = 1;
         *          \n The grandparent if n = 2;
         *          \n and so on...
         */
        Component* GetNthAncestor(int n);

        /**
         * @brief Retrieves a std::vector of Component pointers, which reside 'depth' levels deeper. The tree is traversed in order as the children are stored in std::vector children.
         * E.g. if depth=1, only children of the current are retrieved; if depth=2, only children of the children are retrieved..
         * @param depth - how many levels down the tree should be looked
         * @param outArray - output parameter (vector with results)
         *   An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
         *   The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
         */
        [[ deprecated("Use FindNthDescendants instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void GetNthDescendents(std::vector<Component*>* outArray, int depth);

        /**
         * @brief Retrieves a std::vector of Component pointers, which reside 'depth' levels deeper. The tree is traversed in order as the children are stored in std::vector children.
         * E.g. if depth=1, only children of the current are retrieved; if depth=2, only children of the children are retrieved..
         * @param depth - how many levels down the tree should be looked
         * @param outArray - output parameter (vector with results)
         *   An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
         *   The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, nothing will be pushed into the vector.)
         */

        /**
         * @brief Retrieves all descendants N levels below this component relative to the subtree spanned by this component.
         *        The subtree is traversed in DFS order.
         *
         *        \n If N is larger than the subtree depth, then no descandants will be retrieved;
         *        \n If N = 1, then the children will be retrievd;
         *        \n If N = 2, then the grandchildren will be retrieved;
         *        \n and so on...
         * 
         * @param outArray An output vector to append the matching descendants to.
         * @param depth How many levels below this component.
         */
        void FindNthDescendants(std::vector<Component*> &outArray, int depth);

        /**
         * @brief Retrieves a std::vector of Component pointers, which reside 'depth' levels deeper. 
         * The tree is traversed in order as the children are stored in the std::vector.
         * E.g. if depth=1, only children of the current are retrieved; if depth=2, only children of the children are retrieved..
         * @param depth - how many levels down the tree should be looked
         * @return A std::vector<Component*> with the results.
         */
        [[ deprecated("Use FindNthDescendants instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Component*> GetNthDescendents(int depth);

        /**
         * @brief Retrieves all descendants N levels below this component relative to the subtree spanned by this component.
         *        The subtree is traversed in DFS order.
         *
         * @param depth How many levels below this component.
         *
         * @return A new vector of all matching descendants.
         *
         *        \n If N is larger than the subtree depth, then no descandants will be retrieved;
         *        \n If N = 1, then the children will be retrievd;
         *        \n If N = 2, then the grandchildren will be retrieved;
         *        \n and so on...
         */
        std::vector<Component*> FindNthDescendants(int depth);

        /**
         * @brief Retrieves a std::vector of Component pointers, which reside in the subtree and have a matching type. 
         * The tree is traversed DFS in order as the children are stored in each std::vector children.
         * @param componentType - componentType
         * @param outArray - output parameter (vector with results)
         *   An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
         *   The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
         */
        [[ deprecated("Use FindDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void GetSubcomponentsByType(std::vector<Component*>* outArray, ComponentType::type componentType);

        /**
         * @brief Retrieves a std::vector of Component pointers, which reside in the subtree and have a matching type. 
         * The tree is traversed DFS in order as the children are stored in the std::vector.
         * @param componentType - componentType
         * @return A std::vector<Component*> with the results.
         */
        [[ deprecated("Use FindDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Component*> GetSubcomponentsByType(ComponentType::type _componentType);

        /**
         * @brief Retrieves a std::vector of Component pointers, which form the subtree (current node and all the subcomponents) of this.
         * @param outArray - output parameter (vector with results)
         *   An input is pointer to a std::vector<Component *>, in which the elements will be pushed. It must be allocated before the call (but does not have to be empty).
         *   The method pushes back the found elements -- i.e. the elements(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
         */
        [[ deprecated("Use FindDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void GetComponentsInSubtree(std::vector<Component*>* outArray);

        /**
         * @brief Retrieves a std::vector of Component pointers, which form the subtree (current node and all the subcomponents) of this.
         * @return A std::vector<Component*> with the results.
         */
        [[ deprecated("Use FindDescendantsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Component*> GetComponentsInSubtree();

        /**
         * @brief Returns a (const) reference to the internal vector of relations for a given type.
         * @param relationType Type of relation (see RelationType for available types). Only use specific Relation Types, not RelationType::Any (you will get an empty vector).
         * @return const std::vector<Relation*>& (reference to internal structure)
         * @note The vector is const so that the Relations of a Component cannot be manipulated this way. Use new Relation()/Relation::Delete() to modify the list of Relations.
         * @see FindAllRelationsBy(RelationType::type relationType = RelationType::Any, int thisComponentPosition = -1) as an alternative offering more flexibility at the price of increased overhead through generating a new output vector.
         */
        [[ deprecated("Use GetRelationsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        const std::vector<Relation*>& GetRelations(RelationType::type relationType) const;

        /**
         * @brief Returns a (const) reference to the internal vector of relations for a given type.
         * @param relationType Type of relation (see RelationType for available types). Only use specific Relation Types, not RelationType::Any (you will get an empty vector).
         * @return const std::vector<Relation*>& (reference to internal structure)
         * @note The vector is const so that the Relations of a Component cannot be manipulated this way. Use new Relation()/Relation::Delete() to modify the list of Relations.
         * @see FindAllRelationsBy(RelationType::type relationType = RelationType::Any, int thisComponentPosition = -1) as an alternative offering more flexibility at the price of increased overhead through generating a new output vector.
         */
        const std::vector<Relation*>& GetRelationsByType(RelationType::type relationType) const;

        /**
         * @private
         * @brief Returns a non-const reference to the internal vector of relations (should not be called externally -- you can break things).
         * @param relationType Type of relation
         * @return std::vector<Relation*>& (reference to internal structure)
         * @see public alternative GetRelations(RelationType::type relationType) const
         */
        [[ deprecated("Use _GetRelationsByType instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Relation*>& _GetRelations(RelationType::type relationType) const;

        /**
         * @brief Returns a (const) reference to the internal vector of relations for a given type.
         * @param relationType Type of relation (see RelationType for available types). Only use specific Relation Types, not RelationType::Any (you will get an empty vector).
         * @return const std::vector<Relation*>& (reference to internal structure)
         * @note The vector is const so that the Relations of a Component cannot be manipulated this way. Use new Relation()/Relation::Delete() to modify the list of Relations.
         * @see FindAllRelationsBy(RelationType::type relationType = RelationType::Any, int thisComponentPosition = -1) as an alternative offering more flexibility at the price of increased overhead through generating a new output vector.
         */
        std::vector<Relation*>& _GetRelationsByType(RelationType::type relationType) const;

        /**
         * @brief Returns a newly-constructed vector of all relations of a given type and position.
         * @param relationType Type of relation (default: Any)
         * @param thisComponentPosition Position of this component in the relation (default: -1 = do NOT care about position)
         * @return Vector of matching relations (copy, not reference)
         * @see getRelations(RelationType::type relationType) const as an alternative that returns a reference to the internal structure, i.e. has less overhead.
         */
        [[ deprecated("Use FindRelations instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<Relation*> GetAllRelationsBy(RelationType::type relationType = RelationType::Any, int thisComponentPosition = -1) const;

        /**
         * @brief Returns a newly-constructed vector of all relations of a given type and position.
         * @param relationType Type of relation (default: Any)
         * @param thisComponentPosition Position of this component in the relation (default: -1 = do NOT care about position)
         * @return Vector of matching relations (copy, not reference)
         * @see getRelations(RelationType::type relationType) const as an alternative that returns a reference to the internal structure, i.e. has less overhead.
         */
        std::vector<Relation*> FindRelations(RelationType::type relationType = RelationType::Any, int thisComponentPosition = -1) const;

        /**
         * @private
         * @brief Only called by Relation's AddComponent/UpdateComponent.
         * @param relationType Type of relation
         * @param r Pointer to the relation
         */
        void _AddRelation(RelationType::type relationType, Relation* r);

        /**
         * @brief Retrieves a DataPath* from the list of this component's data paths with matching DataPathCategory and DataPathDirection.
         * The first match is returned.
         * @param dp_category DataPath category to search for
         * @param direction Orientation (default: Any)
         * @return Pointer to the found DataPath, or nullptr if not found
         */
        DataPath* GetDataPathByCategory(DataPathCategory::type dp_category, DataPathDirection::type direction = DataPathDirection::Any) const;
        
        /**
         * @brief Retrieves all DataPath* from the list of this component's data paths with matching type and orientation.
         * Results are returned in std::vector<DataPath*>* outDpArr, where first the matching data paths in dp_outgoing are pushed back, then the ones in dp_incoming.
         * @param outDpArr - output parameter (vector with results)
         * An input is pointer to a std::vector<DataPath *>, in which the data paths will be pushed. It must be allocated before the call (but does not have to be empty).
         * The method pushes back the found data paths -- i.e. the data paths(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
         * @param dp_category DataPath category to search for (default: Any)
         * @param direction Orientation/direction of a DataPath (default: Any)
         */
        [[ deprecated("Use FindDataPaths instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void GetAllDataPaths(std::vector<DataPath*>* outDpArr, DataPathCategory::type dp_category = DataPathCategory::Any, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
         * @brief Retrieves all DataPath* from the list of this component's data paths with matching type and orientation.
         * Results are returned in std::vector<DataPath*>* outDpArr, where first the matching data paths in dp_outgoing are pushed back, then the ones in dp_incoming.
         * @param outDpArr - output parameter (vector with results)
         * An input is pointer to a std::vector<DataPath *>, in which the data paths will be pushed. It must be allocated before the call (but does not have to be empty).
         * The method pushes back the found data paths -- i.e. the data paths(pointers) can be found in this array after the method returns. (If no found, the vector is not changed.)
         * @param dp_category DataPath type to search for (default: Any)
         * @param direction Orientation/direction of a DataPath (default: Any)
         */
        void FindDataPaths(std::vector<DataPath*> &outDpArr, DataPathCategory::type dp_category = DataPathCategory::Any, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
         * @brief Retrieves all DataPath* from the list of this component's data paths with matching type and orientation/direction.
         * Results are returned in a std::vector<DataPath*>*.
         * @param dp_category DataPath type to search for (default: Any)
         * @param direction Orientation (default: Any)
         * @return Vector of matching DataPaths
         */
        [[ deprecated("Use FindDataPaths instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        std::vector<DataPath*> GetAllDataPaths(DataPathCategory::type dp_category = DataPathCategory::Any, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
         * @brief Retrieves all DataPath* from the list of this component's data paths with matching type and orientation/direction.
         * Results are returned in a std::vector<DataPath*>*.
         * @param dp_category DataPath type to search for (default: Any)
         * @param direction Orientation (default: Any)
         * @return Vector of matching DataPaths
         */
        std::vector<DataPath*> FindDataPaths(DataPathCategory::type dp_category = DataPathCategory::Any, DataPathDirection::type direction = DataPathDirection::Any) const;

        /**
        @brief Checks the consistency of the component tree starting from this component.

        This function verifies that each child component has this component set as its parent.
        It logs an error message for each child that has an incorrect parent and increments the error count.
        The function then recursively checks the consistency of the entire subtree rooted at each child component.

        @return The total number of inconsistencies found in the component tree.
        
        The function returns the total number of errors found in the component tree, including errors in the direct children and any nested descendants.
        */
        [[ deprecated("Use CheckSubtreeConsistency instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int CheckComponentTreeConsistency() const;

        /**
        @brief Checks the consistency of the component tree starting from this component.

        This function verifies that each child component has this component set as its parent.
        It logs an error message for each child that has an incorrect parent and increments the error count.
        The function then recursively checks the consistency of the entire subtree rooted at each child component.

        @return The total number of inconsistencies found in the component tree.
        
        The function returns the total number of errors found in the component tree, including errors in the direct children and any nested descendants.
        */
        int CheckSubtreeConsistency() const;

        /**
         * @brief Calculates approximate memory footprint of the subtree of this element (including the relevant Relations).
         * @param out_component_size output parameter (contains the footprint of the component tree elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
         * @param out_dataPathSize output parameter (contains the footprint of the data-path graph elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
         * @return The total size in bytes
         */
        [[ deprecated("Use CalcSubtreeSize instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize) const;

        /**
         * @brief Calculates approximate memory footprint of the subtree of this element (including the relevant Relations).
         * @param out_component_size output parameter (contains the footprint of the component tree elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
         * @param out_dataPathSize output parameter (contains the footprint of the data-path graph elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
         * @return The total size in bytes
         */
        int CalcSubtreeSize(unsigned * out_component_size, unsigned * out_dataPathSize) const;

        /**
         * @private
         * Helper function of int GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize); -- normally you would call this one.
         * \n Calculates approximate memory footprint of the subtree of this element (including the relevant data paths). Does not count DataPaths stored in counted_dataPaths.
         * @param out_component_size - output parameter (contains the footprint of the component tree elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
         * @param out_dataPathSize - output parameter (contains the footprint of the data-path graph elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
         * @param counted_dataPaths - std::set<DataPath*>* of data paths that should not be counted
         * @return The total size in bytes
         * @see GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize);
         */
        [[ deprecated("Use _CalcSubtreeSize instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int _GetTopologySize(unsigned * out_component_size, unsigned * out_RelationSize, std::set<Relation*>* countedRelations) const;

        /**
         * @private
         * Helper function of int GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize); -- normally you would call this one.
         * \n Calculates approximate memory footprint of the subtree of this element (including the relevant data paths). Does not count DataPaths stored in counted_dataPaths.
         * @param out_component_size - output parameter (contains the footprint of the component tree elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
         * @param out_dataPathSize - output parameter (contains the footprint of the data-path graph elements); an already allocated unsigned * is the input, the value is expected to be 0 (the result is accumulated here)
         * @param counted_dataPaths - std::set<DataPath*>* of data paths that should not be counted
         * @return The total size in bytes
         * @see GetTopologySize(unsigned * out_component_size, unsigned * out_dataPathSize);
         */
        int _CalcSubtreeSize(unsigned * out_component_size, unsigned * out_RelationSize, std::set<Relation*> &countedRelations) const;

        /**
         * @brief Retrieves the depth (level) of a component in the topology.
         * @param refresh If true, recalculate the position (depth) of the component in the tree; if false, return the already stored value
         * @return Depth (level) of the component in the topology
         * @see depth
         */
        [[ deprecated("Use CalcDepth instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        int GetDepth(bool refresh);

        /**
         * @brief Retrieves the depth (level) of a component in the topology.
         * @param refresh If true, recalculate the position (depth) of the component in the tree; if false, return the already stored value
         * @return Depth (level) of the component in the topology
         * @see depth
         */
        int CalcDepth(bool refresh);

        /**
         * @private
         * @brief Helper for XML dump generation.
         * Should normally not be used directly. Used internally for exporting the topology to XML.
         * @see exportToXml(Component* root, string path = "", std::function<int(string,void*,string*)> custom_search_attrib_key_fcn = NULL)
         * @return Pointer to the created XML subtree node.
         */
        virtual xmlNodePtr _CreateXmlSubtree();

        /**
         * @private
         *
         * @brief Initializes a JSON object that represents this component.
         *        Intended for internal use.
         *
         * @param obj The JSON object to be initialized.
         */
        virtual void _ToJson(nlohmann::json &obj) const;

        /**
         * @private
         *
         * @brief Initializes this component through JSON. Intended for
         *        internal use.
         *
         * @param obj The JSON object containing the data.
         *
         * @return 0 on success, 1 otherwise.
         */
        virtual int _FromJson(const nlohmann::json &obj);

        /**
         * @brief Deletes all relations of this component (optionally filtered by type).
         * @param relationType Relation type to delete (default: Any)
         */
        [[ deprecated("Use DeleteRelations instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void DeleteAllRelations(RelationType::type relationType = RelationType::Any);

        /**
         * @brief Deletes all relations of this component (optionally filtered by type).
         *        This assumes that all the relations are HEAP-ALLOCATED.
         *
         * @param relationType Relation type to delete (default: Any)
         */
        void DeleteRelations(RelationType::type relationType = RelationType::Any);

        /**
         * @deprecated Use void DeleteRelations(RelationType::type relationType = sys_sage::RelationType::Any) instead.
         * @brief Deletes all DataPaths of this component.
         */
        [[ deprecated("Use DeleteRelations instead. This function will be removed in the future (used up until version 1.0.0).") ]]
        void DeleteAllDataPaths();

#ifdef SS_PAPI
        /**
         * @brief Prints all PAPI metrics of the given event set within the
         *        subtree spanned by this component.
         *
         * @param eventSet Specifies the event set of interest. If the value is
         *        `PAPI_NULL`, then all event sets will be printed.
         */
        void PrintPAPImetricsInSubtree(int eventSet = -1) const;

        /**
         * @brief Retrieves all relations used for collecting PAPI metrics
         *        within the subtree spanned by this component.
         *
         * @return A vector containing said relations.
         */
        std::vector<Relation *> FindPAPIrelationsInSubtree() const;

        /**
         * @brief Retrieves all relations used for collecting PAPI metrics
         *        within the subtree spanned by this component.
         *
         * @param papiRelations A vector used for storing said relations.
         */
        void FindPAPIrelationsInSubtree(std::vector<Relation *> &papiRelations) const;
#endif

        /**
         * @brief Iterator type for attributes iteration.
         */
        using attribIterator = std::map<std::string, std::unique_ptr<IAttribute>>::iterator;

        /**
         * @brief Constant iterator type for attributes iteration.
         */
        using constAttribIterator = std::map<std::string, std::unique_ptr<IAttribute>>::const_iterator;

        /**
         * @brief Size type for estimating the number of stored attributes.
         */
        using attribSizeType = std::map<std::string, std::unique_ptr<IAttribute>>::size_type;

        /**
         * @brief Inserts an attribute using a key-value pair.
         *
         * @param key The key that is associated with the attribute.
         * @param value The value of the attribute.
         *
         * @return A pointer to the respective object storing the value of the
         *         new attribute.
         */
        template <typename T>
        std::decay_t<T> *SetAttribute(const std::string &key, T &&value);

        /**
         * @brief Retrieves the stored value of the attribute that is
         *        associated with the given key.
         *
         * @param key The key that is associated with the attribute.
         *
         * @return A pointer to the respective object storing the value of the
         *         attribute. May be `nullptr` if no attribute is associated
         *         with the given key or the requested type doesn't match the
         *         stored type.
         */
        template <typename T>
        T *GetAttribute(const std::string &key);

        /**
         * @brief Retrieves the stored value of the constant attribute that is
         *        associated with the given key.
         *
         * @param key The key that is associated with the constant attribute.
         *
         * @return A pointer to the respective object storing the value of the
         *         constant attribute. May be `nullptr` if no attribute is
         *         associated with the given key or the requested type doesn't
         *         match the stored type.
         */
        template <typename T>
        const T *GetAttribute(const std::string &key) const;

        /**
         * @brief Retrieves the stored value of the attribute iterator.
         *
         * @param it The iterator of the attribute.
         *
         * @return A pointer to the respective object storing the value of the
         *         attribute. May be `nullptr` if no attribute is associated
         *         with the given iterator or the requested type doesn't match
         *         the stored type.
         */
        template <typename T>
        T *GetAttribute(attribIterator it);

        /**
         * @brief Retrieves the stored value of the constant attribute iterator.
         *
         * @param it The iterator of the constant attribute.
         *
         * @return A pointer to the respective object storing the value of the
         *         constant attribute. May be `nullptr` if no attribute is
         *         associated with the given iterator or the requested type
         *         doesn't match the stored type.
         */
        template <typename T>
        const T *GetAttribute(constAttribIterator it) const;

        /**
         * @brief Updates an existing attribute that is associated with the
         *        given key with a new value. May reuse existing memory and can
         *        be more resource-efficient. If no such attribute exists, the
         *        key-value pair is used to insert a new one.
         *
         * @param key The key that is associated with the attribute.
         * @param value The new value of the attribute.
         *
         * @return A pointer to the respective object storing the updated value
         *         of the attribute. May be `nullptr` if the type of the new
         *         value does not match the stored type of the existing
         *         attribute. Use the `Component::SetAttribute` method to
         *         update in this case.
         */
        template <typename T>
        std::decay_t<T> *UpdateAttribute(const std::string &key, T &&value);

        /**
         * @brief Updates an existing attribute that is associated with the
         *        given iterator with a new value. May reuse existing memory
         *        and can be more resource-efficient.
         *
         * @param it The iterator that is associated with the attribute.
         * @param value The new value of the attribute.
         *
         * @return A pointer to the respective object storing the updated value
         *         of the attribute. May be `nullptr` if no attribute is
         *         associated with the given iterator or the type of the new
         *         value does not match the stored type. Use the
         *         `Component::SetAttribute` method to update in the latter
         *         case.
         */
        template <typename T>
        std::decay_t<T> *UpdateAttribute(attribIterator it, T &&value);

        /**
         * @brief Returns the number of stored attributes.
         *
         * @return The respective size.
         */
        attribSizeType GetAttributesSize() const;

        /**
         * @private
         *
         * @brief Inserts a type-erased attribute.
         *
         * @param key The key associated with the attribute.
         * @param attribute The type-erased attribute.
         */
        // TODO: somehow make this private and find a better name
        void _EmplaceAttribute(const std::string &key, std::unique_ptr<IAttribute> &attribute);

        /**
         * @brief Returns an iterator to the beginning of the attributes.
         *
         * @return The iterator of the first attribute.
         */
        attribIterator AttributesBegin();

        /**
         * @brief Returns an iterator to the beginning of the constant
         *        attributes.
         *
         * @return The iterator of the first constant attribute.
         */
        constAttribIterator AttributesBegin() const;

        /**
         * @brief Returns an iterator to the end of the attributes.
         *
         * @return The iterator of the last attribute.
         */
        attribIterator AttributesEnd();

        /**
         * @brief Returns an iterator to the end of the constant attributes.
         *
         * @return The iterator of the last constant attribute.
         */
        constAttribIterator AttributesEnd() const;

        /**
         * @brief Removes the attribute that is associated to the given key.
         *
         * @param key The key that is associated to the attribute.
         */
        void EraseAttribute(const std::string &key);

        /**
         * @brief Removes the attribute of the given iterator.
         *
         * @param key The iterator of the attribute.
         *
         * @return The iterator to the next attribute.
         */
        attribIterator EraseAttribute(attribIterator it);

        /**
         * @brief Removes all attributes.
         */
        void ClearAttributes();

    protected:
        /**
         * @brief Protected constructor for derived classes (no automatic insertion in the Component Tree).
         * @param _id Numeric ID of the component
         * @param _name Name of the component
         * @param _componentType Component type (of type sys_sage::ComponentType::type)
         */
        Component(int _id, const std::string &_name, ComponentType::type _componentType);

        /**
         * @brief Protected constructor for derived classes with insertion into the Component Tree.
         * @param parent Pointer to the parent component
         * @param _id Numeric ID of the component
         * @param _name Name of the component
         * @param _componentType Component type (of type sys_sage::ComponentType::type)
         */
        Component(Component * parent, int _id, const std::string &_name, ComponentType::type _componentType);

        int id; /**< Numeric ID of the component. There is no requirement for uniqueness of the ID, however it is advised to have unique IDs at least in the realm of parent's children (siblings). Some tree search functions, which take the id as a search parameter search for first match, so the user is responsible to manage uniqueness in the realm of the search subtree (or should be aware of the consequences of not doing so). Component's ID is set by the constructor, and is retrieved via int GetId(); */
        int depth; /**< Depth (level) of the Component in the Component Tree */
        std::string name; /**< Name of the component (as a std::string). */
        int count{-1}; /**< Can be used to represent multiple Components with the same properties. By default, it represents only 1 component, and is set to -1. */
        /**
        Component type of the component. The component type denotes of which class the instance is (often the components are stored as Component*, even though they are a member of one of the child classes)
        Component type is constant, set by constructor, readonly. 
        It can be of types as listed in ComponentType::type (which is user-extensible).*/
        const ComponentType::type componentType;
        std::vector<Component*> children; /**< Contains the list (std::vector) of pointers to children of the component in the component tree. */
        Component* parent { nullptr }; /**< Contains pointer to the parent component in the component tree. If this component is the root, parent will be nullptr.*/
        
        /**
         * Contains a list (std::array) of different Relation types. 
         * Initially nullptr, it is allocated on the first call to AddRelation() or new Relation().
         * The array size is RelationType::_num_relation_types, which is defined in RelationType.
         * Each element of the array is a pointer to a std::vector<Relation*> that contains all Relations of that type. (also lazy-allocated)
         */
        std::array<std::vector<Relation*>*, RelationType::_num_relation_types>* relations = nullptr;

        /**
         * The attributes map.
         */
        std::map<std::string, std::unique_ptr<IAttribute>> attributes;
    };

} //namespace sys_sage 

#include <sys-sage/Component.inl>

#endif //COMPONENT_HPP
