#ifndef SYS_SAGE_SRC_ATTRIBUTE_HPP
#define SYS_SAGE_SRC_ATTRIBUTE_HPP

#include <sys-sage/comptime_str_manipulation.hpp>
#include <nlohmann/json.hpp>
#include <array>
#include <concepts>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

#define SYS_SAGE_PARENTHESES ()

// recursively expands the macro 128 times
#define SYS_SAGE_EXPAND_128(...) SYS_SAGE_EXPAND_64(SYS_SAGE_EXPAND_64(__VA_ARGS__))
#define SYS_SAGE_EXPAND_64(...) SYS_SAGE_EXPAND_32(SYS_SAGE_EXPAND_32(__VA_ARGS__))
#define SYS_SAGE_EXPAND_32(...) SYS_SAGE_EXPAND_16(SYS_SAGE_EXPAND_16(__VA_ARGS__))
#define SYS_SAGE_EXPAND_16(...) SYS_SAGE_EXPAND_8(SYS_SAGE_EXPAND_8(__VA_ARGS__))
#define SYS_SAGE_EXPAND_8(...) SYS_SAGE_EXPAND_4(SYS_SAGE_EXPAND_4(__VA_ARGS__))
#define SYS_SAGE_EXPAND_4(...) SYS_SAGE_EXPAND_2(SYS_SAGE_EXPAND_2(__VA_ARGS__))
#define SYS_SAGE_EXPAND_2(...) SYS_SAGE_EXPAND_1(SYS_SAGE_EXPAND_1(__VA_ARGS__))
#define SYS_SAGE_EXPAND_1(...) __VA_ARGS__

// applies the map `x -> typename x` to every argument of the macro
// the maximum number of supported macro arguments is 128
#define SYS_SAGE_MAP_TYPENAME(...) __VA_OPT__(SYS_SAGE_EXPAND_128(SYS_SAGE_MAP_TYPENAME_INTERNAL(__VA_ARGS__)))
#define SYS_SAGE_MAP_TYPENAME_INTERNAL(arg, ...) SYS_SAGE_TYPENAME(arg)__VA_OPT__(, SYS_SAGE_MAP_TYPENAME_INTERNAL_AGAIN SYS_SAGE_PARENTHESES (__VA_ARGS__))
#define SYS_SAGE_MAP_TYPENAME_INTERNAL_AGAIN() SYS_SAGE_MAP_TYPENAME_INTERNAL
#define SYS_SAGE_TYPENAME(x) typename x

// applies the map `x -> TypeTrait<x, false>::id` to every argument of the macro
// the maximum number of supported macro arguments is 128
#define SYS_SAGE_MAP_TYPETRAIT(...) __VA_OPT__(SYS_SAGE_EXPAND_128(SYS_SAGE_MAP_TYPETRAIT_INTERNAL(__VA_ARGS__)))
#define SYS_SAGE_MAP_TYPETRAIT_INTERNAL(arg, ...) SYS_SAGE_TYPETRAIT(arg)__VA_OPT__(, ", ", SYS_SAGE_MAP_TYPETRAIT_INTERNAL_AGAIN SYS_SAGE_PARENTHESES (__VA_ARGS__))
#define SYS_SAGE_MAP_TYPETRAIT_INTERNAL_AGAIN() SYS_SAGE_MAP_TYPETRAIT_INTERNAL
#define SYS_SAGE_TYPETRAIT(x) TypeTrait<x, false>::id

#define SYS_SAGE_STRINGIFY(...) #__VA_ARGS__

// Generates meta information for the specified type
// DOES NOT REGISTER THE TYPE
#define SYS_SAGE_SPECIALIZE_TYPE_TRAIT(...)                                       \
namespace sys_sage {                                                              \
    template <bool b>                                                             \
    struct TypeTrait<__VA_ARGS__, b> {                                            \
        static constexpr bool serializable = HasToJson<__VA_ARGS__>;              \
        static constexpr bool deserializable = HasFromJson<__VA_ARGS__>;          \
                                                                                  \
        static constexpr decltype(auto) id = SYS_SAGE_STRINGIFY(__VA_ARGS__);     \
                                                                                  \
        template <typename U = __VA_ARGS__> requires (deserializable)             \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj) \
        {                                                                         \
            return std::make_unique<Attribute<U>>(obj.get<U>());                  \
        }                                                                         \
    };                                                                            \
}

// Generates meta information for the specified templated type
// DOES NOT REGISTER THE TEMPLATED TYPE
#define SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT(type, ...)                                                                         \
namespace sys_sage {                                                                                                                \
    template <SYS_SAGE_MAP_TYPENAME(__VA_ARGS__), bool b>                                                                           \
    struct TypeTrait<type<__VA_ARGS__>, b> {                                                                                        \
        static constexpr bool serializable = HasToJson<type<__VA_ARGS__>>;                                                          \
        static constexpr bool deserializable = HasFromJson<type<__VA_ARGS__>>;                                                      \
                                                                                                                                    \
        static constexpr decltype(auto) id = CompStrCat<SYS_SAGE_STRINGIFY(type), "<", SYS_SAGE_MAP_TYPETRAIT(__VA_ARGS__), ">">(); \
                                                                                                                                    \
        template <typename U = type<__VA_ARGS__>> requires (deserializable)                                                         \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)                                                   \
        {                                                                                                                           \
            return std::make_unique<Attribute<U>>(obj.get<U>());                                                                    \
        }                                                                                                                           \
    };                                                                                                                              \
}

// Generates meta information for the specified type
// REGISTERS THE TYPE
#define SYS_SAGE_REGISTER_TYPE_TRAIT(...)                                                  \
namespace sys_sage {                                                                       \
    template <bool b>                                                                      \
    struct TypeTrait<__VA_ARGS__, b> {                                                     \
        static constexpr bool serializable = HasToJson<__VA_ARGS__>;                       \
        static constexpr bool deserializable = HasFromJson<__VA_ARGS__>;                   \
                                                                                           \
        static constexpr decltype(auto) id = SYS_SAGE_STRINGIFY(__VA_ARGS__);              \
                                                                                           \
        template <typename U = __VA_ARGS__> requires (deserializable)                      \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)          \
        {                                                                                  \
            return std::make_unique<Attribute<U>>(obj.get<U>());                           \
        }                                                                                  \
                                                                                           \
        template <typename U = __VA_ARGS__>                                                \
        inline static const auto registrar = []                                            \
        {                                                                                  \
            if constexpr (b && serializable && deserializable)                             \
                TypeRegistry::Instance().Register(id, Deserialize);                        \
            return std::tuple<>{};                                                         \
        }();                                                                               \
                                                                                           \
        __attribute__((used, retain))                                                      \
        static void UseRegistrar()                                                         \
        {                                                                                  \
            if constexpr (b && serializable && deserializable)                             \
                (void) registrar<__VA_ARGS__>;                                             \
        }                                                                                  \
    };                                                                                     \
}

// Generates meta information for the specified templated type
// ONLY REGISTERS A FULLY INSTANTIATED TEMPLATED TYPE
#define SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(type, ...)                                                                           \
namespace sys_sage {                                                                                                                \
    template <SYS_SAGE_MAP_TYPENAME(__VA_ARGS__), bool b>                                                                           \
    struct TypeTrait<type<__VA_ARGS__>, b> {                                                                                        \
        static constexpr bool serializable = HasToJson<type<__VA_ARGS__>>;                                                          \
        static constexpr bool deserializable = HasFromJson<type<__VA_ARGS__>>;                                                      \
                                                                                                                                    \
        static constexpr decltype(auto) id = CompStrCat<SYS_SAGE_STRINGIFY(type), "<", SYS_SAGE_MAP_TYPETRAIT(__VA_ARGS__), ">">(); \
                                                                                                                                    \
        template <typename U = type<__VA_ARGS__>> requires (deserializable)                                                         \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)                                                   \
        {                                                                                                                           \
            return std::make_unique<Attribute<U>>(obj.get<U>());                                                                    \
        }                                                                                                                           \
                                                                                                                                    \
        template <typename U = type<__VA_ARGS__>>                                                                                   \
        inline static const auto registrar = []                                                                                     \
        {                                                                                                                           \
            if constexpr (b && serializable && deserializable)                                                                      \
                TypeRegistry::Instance().Register(id, Deserialize);                                                                 \
            return std::tuple<>{};                                                                                                  \
        }();                                                                                                                        \
                                                                                                                                    \
        __attribute__((used, retain))                                                                                               \
        static void UseRegistrar()                                                                                                  \
        {                                                                                                                           \
            if constexpr (b && serializable && deserializable)                                                                      \
                (void) registrar<type<__VA_ARGS__>>;                                                                                \
        }                                                                                                                           \
    };                                                                                                                              \
}

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

    // forward declaration
    template <typename T, bool>
    struct TypeTrait;
    
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
         * @brief Initializes the registry with some pre-defined types.
         */
        TypeRegistry();
        /**
         * @brief Maps the unique identifier of a `TypeDescriptor` to its deserialization callback routine.
         */
        std::unordered_map<std::string_view, std::unique_ptr<IAttribute>(*)(const nlohmann::json&)> callbacks;
    };

    /**
     * @class TypeTraits
     *
     * @brief Provides compile time meta data for a type.
     */
    template <typename T, bool b = true>
    struct TypeTrait {
        static constexpr bool serializable = HasToJson<T>;
        static constexpr bool deserializable = HasFromJson<T>;

        static constexpr decltype(auto) id = CompStrExtract<T>();

        template <typename U = T> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<U>>(obj.get<U>());
        }

        template <typename U = T>
        inline static const auto registrar = []
        {
            if constexpr (b && serializable && deserializable)
                TypeRegistry::Instance().Register(id, Deserialize);
            return std::tuple<>{};
        }();

        __attribute__((used, retain))
        static void UseRegistrar()
        {
            if constexpr (b && serializable && deserializable)
                (void) registrar<T>;
        }
    };
}

///////////////////////////////////////////////////////////////////////////////
/////////////// SOME OUT-OF-THE-BOX TYPE TRAIT SPECIALIZATIONS ////////////////
///////////////////////////////////////////////////////////////////////////////

SYS_SAGE_SPECIALIZE_TYPE_TRAIT(bool)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(char)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(signed char)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(unsigned char)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(short)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(unsigned short)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(int)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(unsigned int)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(long)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(unsigned long)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(long long)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(unsigned long long)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(float)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(double)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(long double)
SYS_SAGE_SPECIALIZE_TYPE_TRAIT(std::string)

SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(std::vector, T)
SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(std::map, K, V)
SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(std::unordered_map, K, V)

#include <sys-sage/attribute.inl>

#endif
