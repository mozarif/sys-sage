#ifndef SYS_SAGE_SRC_ATTRIBUTE_HPP
#define SYS_SAGE_SRC_ATTRIBUTE_HPP

#include <nlohmann/json.hpp>
#include <sys-sage/comptime_str_manipulation.hpp>
#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

#define SYS_SAGE_EXPAND_128(...) SYS_SAGE_EXPAND_64(SYS_SAGE_EXPAND_64(__VA_ARGS__))
#define SYS_SAGE_EXPAND_64(...) SYS_SAGE_EXPAND_32(SYS_SAGE_EXPAND_32(__VA_ARGS__))
#define SYS_SAGE_EXPAND_32(...) SYS_SAGE_EXPAND_16(SYS_SAGE_EXPAND_16(__VA_ARGS__))
#define SYS_SAGE_EXPAND_16(...) SYS_SAGE_EXPAND_8(SYS_SAGE_EXPAND_8(__VA_ARGS__))
#define SYS_SAGE_EXPAND_8(...) SYS_SAGE_EXPAND_4(SYS_SAGE_EXPAND_4(__VA_ARGS__))
#define SYS_SAGE_EXPAND_4(...) SYS_SAGE_EXPAND_2(SYS_SAGE_EXPAND_2(__VA_ARGS__))
#define SYS_SAGE_EXPAND_2(...) SYS_SAGE_EXPAND_1(SYS_SAGE_EXPAND_1(__VA_ARGS__))
#define SYS_SAGE_EXPAND_1(...) __VA_ARGS__

#define SYS_SAGE_PARENTHESES ()

#define SYS_SAGE_TYPENAME(x) typename x
#define SYS_SAGE_MAP_TYPENAME(...) __VA_OPT__(SYS_SAGE_EXPAND_128(SYS_SAGE_MAP_TYPENAME_INTERNAL(__VA_ARGS__)))
#define SYS_SAGE_MAP_TYPENAME_INTERNAL(arg, ...) SYS_SAGE_TYPENAME(arg)__VA_OPT__(, SYS_SAGE_MAP_TYPENAME_INTERNAL_AGAIN SYS_SAGE_PARENTHESES (__VA_ARGS__))
#define SYS_SAGE_MAP_TYPENAME_INTERNAL_AGAIN() SYS_SAGE_MAP_TYPENAME_INTERNAL

#define SYS_SAGE_TYPETRAIT(x) TypeTrait<x, false>::id
#define SYS_SAGE_MAP_TYPETRAIT(...) __VA_OPT__(SYS_SAGE_EXPAND_128(SYS_SAGE_MAP_TYPETRAIT_INTERNAL(__VA_ARGS__)))
#define SYS_SAGE_MAP_TYPETRAIT_INTERNAL(arg, ...) SYS_SAGE_TYPETRAIT(arg)__VA_OPT__(, ", ", SYS_SAGE_MAP_TYPETRAIT_INTERNAL_AGAIN SYS_SAGE_PARENTHESES (__VA_ARGS__))
#define SYS_SAGE_MAP_TYPETRAIT_INTERNAL_AGAIN() SYS_SAGE_MAP_TYPETRAIT_INTERNAL

#define SYS_SAGE_STRINGIFY(...) #__VA_ARGS__
#define SYS_SAGE_REGISTER_TYPE(...) SYS_SAGE_REGISTER_TYPE_INTERNAL(SYS_SAGE_EXPAND_1(__VA_ARGS__))
#define SYS_SAGE_SPECIALIZE_TRAIT(...) SYS_SAGE_SPECIALIZE_TRAIT_INTERNAL(SYS_SAGE_EXPAND_1(__VA_ARGS__))

#define SYS_SAGE_REGISTER_TYPE_INTERNAL(type)                                       \
namespace sys_sage {                                                                \
    template <bool b>                                                               \
    struct TypeTrait<type, b> {                                                     \
        static constexpr bool serializable = HasToJson<type>;                       \
        static constexpr bool deserializable = HasFromJson<type>;                   \
                                                                                    \
        static constexpr decltype(auto) id = SYS_SAGE_STRINGIFY(type);              \
                                                                                    \
        template <typename U = type> requires (deserializable)                      \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)   \
        {                                                                           \
            return std::make_unique<Attribute<U>>(obj.get<U>());                    \
        }                                                                           \
                                                                                    \
        template <typename U = type> requires (b && serializable && deserializable) \
        inline static const auto registrar = []                                     \
        {                                                                           \
            TypeRegistry::Instance().Register(id, Deserialize);                     \
            return std::tuple<>{};                                                  \
        }();                                                                        \
                                                                                    \
        __attribute__((used, retain))                                               \
        static void UseRegistrar()                                                  \
        {                                                                           \
            if constexpr (b && serializable && deserializable)                      \
                (void) registrar<type>;                                             \
        }                                                                           \
    };                                                                              \
}

#define SYS_SAGE_SPECIALIZE_TRAIT_INTERNAL(type)                                  \
namespace sys_sage {                                                              \
    template <bool b>                                                             \
    struct TypeTrait<type, b> {                                                   \
        static constexpr bool serializable = HasToJson<type>;                     \
        static constexpr bool deserializable = HasFromJson<type>;                 \
                                                                                  \
        static constexpr decltype(auto) id = SYS_SAGE_STRINGIFY(type);            \
                                                                                  \
        template <typename U = type> requires (deserializable)                    \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj) \
        {                                                                         \
            return std::make_unique<Attribute<U>>(obj.get<U>());                  \
        }                                                                         \
    };                                                                            \
}

#define SYS_SAGE_REGISTER_TEMPLATE_TYPE(template_type, ...)                                                                                  \
namespace sys_sage {                                                                                                                         \
    template <SYS_SAGE_MAP_TYPENAME(__VA_ARGS__), bool b>                                                                                    \
    struct TypeTrait<template_type<__VA_ARGS__>, b> {                                                                                        \
        static constexpr bool serializable = HasToJson<template_type<__VA_ARGS__>>;                                                          \
        static constexpr bool deserializable = HasFromJson<template_type<__VA_ARGS__>>;                                                      \
                                                                                                                                             \
        static constexpr decltype(auto) id = CompStrCat<SYS_SAGE_STRINGIFY(template_type), "<", SYS_SAGE_MAP_TYPETRAIT(__VA_ARGS__), ">">(); \
                                                                                                                                             \
        template <typename U = template_type<__VA_ARGS__>> requires (deserializable)                                                         \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)                                                            \
        {                                                                                                                                    \
            return std::make_unique<Attribute<U>>(obj.get<U>());                                                                             \
        }                                                                                                                                    \
                                                                                                                                             \
        template <typename U = template_type<__VA_ARGS__>> requires (b && serializable && deserializable)                                    \
        inline static const auto registrar = []                                                                                              \
        {                                                                                                                                    \
            TypeRegistry::Instance().Register(id, Deserialize);                                                                              \
            return std::tuple<>{};                                                                                                           \
        }();                                                                                                                                 \
                                                                                                                                             \
        __attribute__((used, retain))                                                                                                        \
        static void UseRegistrar()                                                                                                           \
        {                                                                                                                                    \
            if constexpr (b && serializable && deserializable)                                                                               \
                (void) registrar<template_type<__VA_ARGS__>>;                                                                                \
        }                                                                                                                                    \
    };                                                                                                                                       \
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

        template <typename U = T> requires (!deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return nullptr;
        }

        template <typename U = T> requires (b && serializable && deserializable)
        inline static const auto registrar = []
        {
            TypeRegistry::Instance().Register(id, Deserialize);
            return std::tuple<>{};
        }();

        __attribute__((used, retain))
        static void UseRegistrar()
        {
            // TODO: fix this
            if constexpr (b && serializable && deserializable)
                (void) registrar<T>;
        }
    };

    template <typename T, bool b>
    struct TypeTrait<std::vector<T>, b> {
        static constexpr bool serializable = HasToJson<std::vector<T>>;
        static constexpr bool deserializable = HasFromJson<std::vector<T>>;
    
        static constexpr decltype(auto) id = CompStrCat<"std::vector<", TypeTrait<T, false>::id, ">">();

        template <typename U = std::vector<T>> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<U>>(obj.get<U>());
        }

        template <typename U = std::vector<T>> requires (b && serializable && deserializable)
        inline static const auto registrar = []
        {
            TypeRegistry::Instance().Register(id, Deserialize);
            return std::tuple<>{};
        }();

        __attribute__((used, retain))
        static void UseRegistrar()
        {
            if constexpr (b && serializable && deserializable)
                (void) registrar<std::vector<T>>;
        }
    };

    template <typename K, typename V, bool b>
    struct TypeTrait<std::map<K, V>, b> {
        static constexpr bool serializable = HasToJson<std::map<K, V>>;
        static constexpr bool deserializable = HasFromJson<std::map<K, V>>;
    
        static constexpr decltype(auto) id = CompStrCat<"std::map<", TypeTrait<K, false>::id, TypeTrait<V, false>::id, ">">();

        template <typename U = std::map<K, V>> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<U>>(obj.get<U>());
        }

        template <typename U = std::map<K, V>> requires (b && serializable && deserializable)
        inline static const auto registrar = []
        {
            TypeRegistry::Instance().Register(id, Deserialize);
            return std::tuple<>{};
        }();

        __attribute__((used, retain))
        static void UseRegistrar()
        {
            if constexpr (b && serializable && deserializable)
                (void) registrar<std::map<K, V>>;
        }
    };

    //template <typename K, typename V, bool b>
    //struct TypeTrait<std::unordered_map<K, V>, b> {
    //    static constexpr bool serializable = HasToJson<std::unordered_map<K, V>>;
    //    static constexpr bool deserializable = HasFromJson<std::unordered_map<K, V>>;
    //
    //    static constexpr decltype(auto) id = CompStrCat<"std::unordered_map<", TypeTrait<K, false>::id, ", ", TypeTrait<V, false>::id, ">">();

    //    template <typename U = std::unordered_map<K, V>> requires (deserializable)
    //    static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
    //    {
    //        return std::make_unique<Attribute<U>>(obj.get<U>());
    //    }

    //    template <typename U = std::unordered_map<K, V>> requires (b && serializable && deserializable)
    //    inline static const auto registrar = []
    //    {
    //        TypeRegistry::Instance().Register(id, Deserialize);
    //        return std::tuple<>{};
    //    }();

    //    __attribute__((used, retain))
    //    static void UseRegistrar()
    //    {
    //        if constexpr (b && serializable && deserializable)
    //            (void) registrar<std::unordered_map<K, V>>;
    //    }
    //};
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// PRE-DEFINE SOME TYPES /////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SYS_SAGE_SPECIALIZE_TRAIT(bool)
SYS_SAGE_SPECIALIZE_TRAIT(char)
SYS_SAGE_SPECIALIZE_TRAIT(signed char)
SYS_SAGE_SPECIALIZE_TRAIT(unsigned char)
SYS_SAGE_SPECIALIZE_TRAIT(short int)
SYS_SAGE_SPECIALIZE_TRAIT(unsigned short int)
SYS_SAGE_SPECIALIZE_TRAIT(int)
SYS_SAGE_SPECIALIZE_TRAIT(unsigned int)
SYS_SAGE_SPECIALIZE_TRAIT(long int)
SYS_SAGE_SPECIALIZE_TRAIT(unsigned long int)
SYS_SAGE_SPECIALIZE_TRAIT(long long int)
SYS_SAGE_SPECIALIZE_TRAIT(unsigned long long int)
SYS_SAGE_SPECIALIZE_TRAIT(float)
SYS_SAGE_SPECIALIZE_TRAIT(double)
SYS_SAGE_SPECIALIZE_TRAIT(long double)
SYS_SAGE_SPECIALIZE_TRAIT(std::string)

#include <sys-sage/attribute.inl>

#endif
