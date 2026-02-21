#ifndef SYS_SAGE_SRC_JSON_SERIALIZATION_HPP
#define SYS_SAGE_SRC_JSON_SERIALIZATION_HPP

#include "Component.hpp"
#include "Relation.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>

namespace sys_sage {
    /**
     * @brief Dumps the entire subtree and the corresponding relation graph
     *        spanned by the provided component into a JSON file.
     *
     * @param component The component of interest.
     *
     * @param path The location of the output JSON file. If the path is empty,
     *             the JSON data will be dumped into stdout.
     *
     * @return 0 on success, 1 otherwise.
     */
    int DumpJson(const Component *component, const std::filesystem::path &path = "");

    /**
     * @brief Loads the component tree from JSON.
     *
     * @param path The path to the JSON file.
     *
     * @return A pointer to the root of the component tree. May return
     *         `nullptr` in case of an error.
     */
    Component *LoadJson(const std::filesystem::path &path);

    /**
     * @brief Provides idiomatic JSON serialization for all components.
     *
     * @param obj The JSON object to be initialized.
     * @param component The component of interest.
     */
    void to_json(nlohmann::json &obj, const Component &component);

    /**
     * @brief Provides idiomatic JSON serialization for all components.
     *
     * @param obj The JSON object to be initialized.
     * @param component The component of interest.
     */
    void to_json(nlohmann::json &obj, const Component *component);

    /**
     * @brief Provides idiomatic JSON deserialization for all components.
     *
     * @param obj The JSON object containing the data.
     * @param component The component to be initialized.
     */
    void from_json(const nlohmann::json &obj, Component &component);

    /**
     * @brief Provides idiomatic JSON deserialization for all components.
     *        This function allocates the object on the heap and returns a pointer.
     *
     * @param obj The JSON object containing the data.
     * @param component The component to be initialized.
     */
    void from_json(const nlohmann::json &obj, Component *&component);

    /**
     * @brief Provides idiomatic JSON serialization for all relations.
     *
     * @param obj The JSON object to be initialized.
     * @param relation The relation of interest.
     */
    void to_json(nlohmann::json &obj, const Relation &relation);

    /**
     * @brief Provides idiomatic JSON serialization for all relations.
     *
     * @param obj The JSON object to be initialized.
     * @param relation The relation of interest.
     */
    void to_json(nlohmann::json &obj, const Relation *relation);

    /**
     * @brief Provides custom JSON deserialization for all relations.
     *
     * @param obj The JSON object containing the data.
     * @param root The root of the component tree.
     *
     * @return The initialized relation.
     */
    template <typename T>
    T GetRelation(const nlohmann::json &obj, Component *root)
    {
        T relation;
        relation.FromJson(obj, root);
        return relation; // rely on RVO
    }

    /**
     * @brief Provides custom JSON deserialization for all relations. This
     *        function allocates the object on the heap and returns a pointer.
     *
     * @param obj The JSON object containing the data.
     * @param root The root of the component tree.
     *
     * @return A pointer to the initialized relation.
     */
    template <>
    Relation *GetRelation<Relation *>(const nlohmann::json &obj, Component *root);
}

#endif
