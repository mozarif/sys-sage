#ifndef SYS_SAGE_SRC_ATTRIBUTE_HPP
#define SYS_SAGE_SRC_ATTRIBUTE_HPP

#include <nlohmann/json.hpp>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>

#define _SYS_SAGE_CONCAT(x, y) x##y
#define SYS_SAGE_CONCAT(x, y) _SYS_SAGE_CONCAT(x, y)

// this macro handles automatic type registration
#define SYS_SAGE_REGISTER_TYPE(type)                                                                \
namespace sys_sage {                                                                                \
    template <>                                                                                     \
    struct TypeTrait<type> {                                                                        \
        static constexpr bool registered { true };                                                  \
        static constexpr TypeDescriptor descriptor { #type };                                       \
    };                                                                                              \
}                                                                                                   \
                                                                                                    \
inline sys_sage::TypeRegistrar<type> SYS_SAGE_CONCAT(_sysSageTypeRegistrar, __COUNTER__) ( #type );

namespace sys_sage {
    /**
     * @class TypeDescriptor
     *
     * @brief A unique descriptor for a type.
     */
    struct TypeDescriptor {
        std::string_view id;
    };

    /**
     * @class TypeTrait
     *
     * @brief Associates a registered type to its `TypeDescriptor` at compile time.
     */
    template <typename T>
    struct TypeTrait {
        static constexpr bool registered { false };
    };

    /**
     * @class IAttribute
     *
     * @brief A polymorphic interface for type-erased attributes.
     */
    class IAttribute {
    public:
        /**
         * @brief Virtual destructor for correct cleanup of attributes.
         */
        virtual ~IAttribute() = default;

        /**
         * @brief Virtual JSON serialization for atttributes.
         *
         * @param obj The JSON object to be initialized to represent this attribute.
         */
        virtual void Serialize(nlohmann::json &obj) const = 0;
    };
    
    /**
     * @class Attribute
     *
     * @brief A wrapper that stores an attribute of any type.
     */
    template <typename T>
    class Attribute : public IAttribute {
    public:
        /**
         * @brief Generic constructor using perfect forwarding.
         *
         * @param value Initializes this attribute using the given value.
         */
        template <typename U>
        Attribute(U &&_value);
    
        /**
         * @brief Overloaded operator for accessing the underlying value of the
         *        attribute.
         *
         * @return A reference to the value.
         */
        T &operator*();

        /**
         * @brief Overloaded operator for accessing the underlying value of the
         *        constant attribute.
         *
         * @return A reference to the const value.
         */
        const T &operator*() const;
    
        /**
         * @brief Overloaded operator for accessing the underlying value of the
         *        attribute.
         *
         * @return A pointer to the value.
         */
        T *operator->();

        /**
         * @brief Overloaded operator for accessing the underlying value of the
         *        constant attribute.
         *
         * @return A pointer to the constant value.
         */
        const T *operator->() const;

        /**
         * @brief JSON serialization for atttributes.
         *
         * @param obj The JSON object to be initialized to represent this attribute.
         */
        void Serialize(nlohmann::json &obj) const override;
    
    private:
        /**
         * @brief The underlying value.
         */
        T value;
    };

    /**
     * @class TypeCallBack
     *
     * @brief Associates a deserialization callback to a registered type at compile time.
     */
    template <typename T>
    struct TypeCallBack {
        /**
         * @brief Checks whether JSON load logic is provided for this type at compile time.
         */
        static constexpr bool valid = requires(const nlohmann::json &obj) { obj.get<T>(); };
    
        /**
         * @brief Defines a deserialization callback for this type.
         *
         * @param The JSON object containing the serialized attribute.
         *
         * @return The deserialized attribute.
         */
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj);
    };

    /**
     * @class TypeRegistry
     *
     * @brief Manages type registration and stores deserialization callbacks functions.
     */
    class TypeRegistry {
    public:
        /**
         * @brief Returns the global static type registry.
         */
        static TypeRegistry &Instance();
    
        /**
         * @brief Registers a type.
         *
         * @param id A unique identifier for the `TypeDescriptor`.
         */
        template <typename T>
        void Register(std::string_view id);
    
        /**
         * @brief Returns the deserialization callback associated to this type.
         *
         * @param id A unique identifier for the `TypeDescriptor`.
         */
        std::optional<std::unique_ptr<IAttribute>(*)(const nlohmann::json&)> GetCallBack(std::string_view id) const;
    
    private:
        /**
         * @brief Maps the unique identifier of a `TypeDescriptor` to its deserialization callback routine.
         */
        std::unordered_map<std::string_view, std::unique_ptr<IAttribute>(*)(const nlohmann::json&)> callbacks;
    };
    
    /**
     * @class TypeRegistrar
     *
     * @brief Automatically registers a type.
     */
    template <typename T>
    struct TypeRegistrar {
        TypeRegistrar(std::string_view id);
    };
}

#include <sys-sage/attribute.inl>

#endif
