#ifndef SYS_SAGE_SRC_ATTRIBUTE_HPP
#define SYS_SAGE_SRC_ATTRIBUTE_HPP

#include <nlohmann/json.hpp>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <ranges>
#include <string_view>
#include <unordered_map>

// this macro handles automatic type registration for atomic types
#define SYS_SAGE_REGISTER_TYPE(type)                                                                                       \
namespace sys_sage {                                                                                                       \
    template <>                                                                                                            \
    struct TypeTraits<type>                                                                                                \
    {                                                                                                                      \
        static constexpr bool registered = true;                                                                           \
        static constexpr bool serializable = requires(const type &attr) { nlohmann::json(attr); };                         \
        static constexpr bool deserializable = requires(const nlohmann::json &obj) { obj.get<type>(); };                   \
        static constexpr decltype(auto) id = #type;                                                                        \
        inline static const auto registrar = [] { TypeRegistry::Instance().Register<type>(id); return std::tuple<>{}; }(); \
                                                                                                                           \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)                                          \
        {                                                                                                                  \
            if constexpr (deserializable)                                                                                  \
                return std::make_unique<Attribute<type>>(obj.get<type>());                                                 \
            else                                                                                                           \
                return nullptr;                                                                                            \
        }                                                                                                                  \
    };                                                                                                                     \
}

namespace sys_sage {
    /**
     * @class TypeTraits
     *
     * @brief Provides compile time meta data for a type.
     */
    template <typename T>
    struct TypeTrait {
        static constexpr bool registered = false;
        static constexpr bool serializable = requires(const T &attr) { nlohmann::json(attr); };
        static constexpr bool deserializable = requires(const nlohmann::json &obj) { obj.get<T>(); };
        static constexpr decltype(auto) id = "";
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
}

#include <sys-sage/attribute.inl>
#include <sys-sage/comptime_str_concat.inl>

#endif
