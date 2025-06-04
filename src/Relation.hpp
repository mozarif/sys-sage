#ifndef RELATION_HPP
#define RELATION_HPP

#include <map>
#include <vector>
#include <string>
#include <libxml/parser.h>

#include "defines.hpp"
#include "enums.hpp"

// #include "Component.hpp"

namespace sys_sage { //forward declaration
    class Component;
    class Qubit;
}

namespace sys_sage {

    /**
     * @class Relation
     * @brief Represents a generic relationship or connectivity between an arbitrary number of components.
     *
     * This class defines a generic relationship with methods to set and get
     * the type, id, and name of the relationship. It also provides pure virtual
     * functions for printing and deleting the relationship, making it an abstract class.
     */
    class Relation {
    public:
        //SVDOCTODO
        Relation(const std::vector<Component*>& components, int _id = 0, bool _ordered = true);
        /**
         * @brief Sets the id of the relationship.
         * @param _id The id of the relationship to set.
         */
        void SetId(int _id);
        /**
         * @brief Gets the id of the relationship.
         * @return The current id of the relationship.
         */
        int GetId() const;
        /**
         * @brief Gets the type of the relationship.
         * @return The current type of the relationship.
         */
        int GetType() const;
        //SVDOCTODO
        std::string GetTypeStr() const;
        //SVDOCTODO
        bool IsOrdered() const;
        //SVDOCTODO
        bool ContainsComponent(Component* c) const;
        //SVDOCTODO
        Component* GetComponent(int index) const;
        //SVDOCTODO
        const std::vector<Component*>& GetComponents() const;

        //SVDOCTODO
        /**
         * @brief Pure virtual function to print the details of the relationship.
         * 
         * Derived classes must implement this function to provide specific
         * printing behavior.
         */
        virtual void Print() const;
        //SVDOCTODO private
        void _PrintRelationAttrib() const;
        //SVDOCTODO private
        void _PrintRelationComponentInfo() const;
        //SVDOCTODO
        void AddComponent(Component* c);
        //SVDOCTODO
        int UpdateComponent(int index, Component * _new_component);
        //SVDOCTODO
        //SVDOCTODO mention that it only replaces first entry of _old_component found (using std::find)
        int UpdateComponent(Component* _old_component, Component * _new_component);

        //SVDOCTODO
        virtual xmlNodePtr _CreateXmlEntry();
        //SVDOCTODO
        /**
         * @brief Pure virtual function to delete the relationship.
         * 
         * Derived classes must implement this function to provide specific
         * deletion behavior.
         */
        virtual void Delete();//TODO
        /**
         * @brief Destructor for the Relation class.
         * 
         * This is a virtual destructor to ensure proper cleanup of derived classes.
         */
        ~Relation() = default;
    protected:
        Relation(int _relation_type);
        Relation(const std::vector<Component*>& components, int _id, bool _ordered, int _relation_type);


        bool ordered;
        /**
         * @brief The id of the relationship.
         * 
         * This member variable stores the unique identifier for the relationship.
         */
        int id;
        /**
         * @brief The type of the relationship.
         * 
         * This member variable stores the type or category of the relationship.
         */
        int type;
        /**
         * @brief A vector of components associated with the relationship.
         * 
         * This member variable holds pointers to components that are part of
         * the relationship.
         */
        std::vector<Component*> components;

    public:
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
    };

}
#endif //RELATION_HPP