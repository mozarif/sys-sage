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

#define SYS_SAGE_STRINGIFY(...) #__VA_ARGS__
#define SYS_SAGE_EXTRACT_ARG(...) __VA_ARGS__

// this macro handles type registration for atomic types
#define SYS_SAGE_REGISTER_TYPE_INTERNAL(type)                                     \
namespace sys_sage {                                                              \
    template <>                                                                   \
    struct TypeTrait<type, true> {                                                \
        static constexpr bool serializable = HasToJson<type>;                     \
        static constexpr bool deserializable = HasFromJson<type>;                 \
        static constexpr bool registered = serializable && deserializable;        \
                                                                                  \
        static constexpr decltype(auto) id = SYS_SAGE_STRINGIFY(type);            \
                                                                                  \
        template <typename U = type> requires (deserializable)                    \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj) \
        {                                                                         \
            return std::make_unique<Attribute<U>>(obj.get<U>());                  \
        }                                                                         \
                                                                                  \
        template <typename U = type> requires (!deserializable)                   \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json&)     \
        {                                                                         \
            return nullptr;                                                       \
        }                                                                         \
                                                                                  \
        inline static const auto registrar = []                                   \
        {                                                                         \
            if constexpr (serializable && deserializable) {                       \
                TypeRegistry::Instance().Register(id, Deserialize);               \
            }                                                                     \
            return std::tuple<>{};                                                \
        }();                                                                      \
    };                                                                            \
                                                                                  \
    template <>                                                                   \
    struct TypeTrait<type, false> {                                               \
        static constexpr bool serializable = HasToJson<type>;                     \
        static constexpr bool deserializable = HasFromJson<type>;                 \
        static constexpr bool registered = serializable && deserializable;        \
                                                                                  \
        static constexpr decltype(auto) id = SYS_SAGE_STRINGIFY(type);            \
    };                                                                            \
}

#define SYS_SAGE_REGISTER_TYPE(...) SYS_SAGE_REGISTER_TYPE_INTERNAL(SYS_SAGE_EXTRACT_ARG(__VA_ARGS__))

namespace sys_sage {
    /**
     * @brief A concept for checking if a type supports JSON serialization.
     */
    template <typename T>
    concept HasToJson = requires (nlohmann::json &obj, const T &attr) { nlohmann::adl_serializer<T>::to_json(obj, attr); };

    /**
     * @brief A concept for checking if a type supports JSON deserialization.
     */
    template <typename T>
    concept HasFromJson = requires (const nlohmann::json &obj, T &attr) { nlohmann::adl_serializer<T>::from_json(obj, attr); }
                       || requires (const nlohmann::json &obj) { { nlohmann::adl_serializer<T>::from_json(obj) } -> std::same_as<T>; };

    /**
     * @class TypeTraits
     *
     * @brief Provides compile time meta data for a type.
     */
    template <typename T, bool b = true>
    struct TypeTrait {
        static constexpr bool serializable = HasToJson<T>;
        static constexpr bool deserializable = HasFromJson<T>;
        static constexpr bool registered = false;
    
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
        void Register(std::string_view id, std::unique_ptr<IAttribute> (*callback)(const nlohmann::json&));
    
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
