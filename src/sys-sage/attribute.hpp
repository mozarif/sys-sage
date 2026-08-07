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

#define SYS_SAGE_STRINGIFY(...) #__VA_ARGS__

// Generates meta information for the specified type
// DOES NOT REGISTER THE TYPE
#define SYS_SAGE_SPECIALIZE_TYPE_TRAIT(...)                                       \
namespace sys_sage {                                                              \
    template <bool b>                                                             \
    struct TypeTrait<__VA_ARGS__, b> {                                            \
        static constexpr bool serializable = IsSerializable<__VA_ARGS__>;         \
        static constexpr bool deserializable = IsDeserializable<__VA_ARGS__>;     \
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

// Generates meta information for the specified type
// REGISTERS THE TYPE
#define SYS_SAGE_REGISTER_TYPE_TRAIT(...)                                         \
namespace sys_sage {                                                              \
    template <bool b>                                                             \
    struct TypeTrait<__VA_ARGS__, b> {                                            \
        static constexpr bool serializable = IsSerializable<__VA_ARGS__>;         \
        static constexpr bool deserializable = IsDeserializable<__VA_ARGS__>;     \
                                                                                  \
        static constexpr decltype(auto) id = SYS_SAGE_STRINGIFY(__VA_ARGS__);     \
                                                                                  \
        template <typename U = __VA_ARGS__> requires (deserializable)             \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj) \
        {                                                                         \
            return std::make_unique<Attribute<U>>(obj.get<U>());                  \
        }                                                                         \
                                                                                  \
        template <typename U = __VA_ARGS__>                                       \
        inline static const auto registrar = []                                   \
        {                                                                         \
            if constexpr (b && serializable && deserializable)                    \
                TypeRegistry::Instance().Register(id, Deserialize);               \
            return std::tuple<>{};                                                \
        }();                                                                      \
                                                                                  \
        __attribute__((used, retain))                                             \
        static void UseRegistrar()                                                \
        {                                                                         \
            if constexpr (b && serializable && deserializable)                    \
                (void) registrar<__VA_ARGS__>;                                    \
        }                                                                         \
    };                                                                            \
}

// Generates meta information for the specified templated type
// DOES NOT REGISTER THE TEMPLATED TYPE
#define SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT(type)                                                                              \
namespace sys_sage {                                                                                                                \
    template <typename... Ts, bool b>                                                                                               \
    struct TypeTrait<type<Ts...>, b> {                                                                                              \
        static constexpr bool serializable = IsSerializable<type<Ts...>>;                                                           \
        static constexpr bool deserializable = IsDeserializable<type<Ts...>>;                                                       \
                                                                                                                                    \
        static constexpr decltype(auto) id = CompStrCat<SYS_SAGE_STRINGIFY(type), "<", CompStrCatVariadicTemplate<Ts...>(), ">">(); \
                                                                                                                                    \
        template <typename U = type<Ts...>> requires (deserializable)                                                               \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)                                                   \
        {                                                                                                                           \
            return std::make_unique<Attribute<U>>(obj.get<U>());                                                                    \
        }                                                                                                                           \
    };                                                                                                                              \
}

// Generates meta information for the specified templated type
// ONLY REGISTERS A FULLY INSTANTIATED TEMPLATED TYPE
#define SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(type)                                                                                \
namespace sys_sage {                                                                                                                \
    template <typename... Ts, bool b>                                                                                               \
    struct TypeTrait<type<Ts...>, b> {                                                                                              \
        static constexpr bool serializable = IsSerializable<type<Ts...>>;                                                           \
        static constexpr bool deserializable = IsDeserializable<type<Ts...>>;                                                       \
                                                                                                                                    \
        static constexpr decltype(auto) id = CompStrCat<SYS_SAGE_STRINGIFY(type), "<", CompStrCatVariadicTemplate<Ts...>(), ">">(); \
                                                                                                                                    \
        template <typename U = type<Ts...>> requires (deserializable)                                                               \
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)                                                   \
        {                                                                                                                           \
            return std::make_unique<Attribute<U>>(obj.get<U>());                                                                    \
        }                                                                                                                           \
                                                                                                                                    \
        template <typename U = type<Ts...>>                                                                                         \
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
                (void) registrar<type<Ts...>>;                                                                                      \
        }                                                                                                                           \
    };                                                                                                                              \
}

// blacklists the specified type from JSON serialization
#define SYS_SAGE_BLACKLIST_TYPE_FROM_SERIALIZATION(...)                     \
namespace sys_sage {                                                        \
    template <>                                                             \
    struct IsBlacklistedFromSerialization<__VA_ARGS__> : std::true_type {}; \
}

// blacklists the specified type from JSON deserialization
#define SYS_SAGE_BLACKLIST_TYPE_FROM_DESERIALIZATION(...)                     \
namespace sys_sage {                                                          \
    template <>                                                               \
    struct IsBlacklistedFromDeserialization<__VA_ARGS__> : std::true_type {}; \
}

// blacklists the specified templated type from JSON serialization
#define SYS_SAGE_BLACKLIST_TEMPLATED_TYPE_FROM_SERIALIZATION(type)          \
namespace sys_sage {                                                        \
    template <typename... Ts>                                               \
    struct IsBlacklistedFromSerialization<type<Ts...>> : std::true_type {}; \
}

// blacklists the specified templated type from JSON deserialization
#define SYS_SAGE_BLACKLIST_TEMPLATED_TYPE_FROM_DESERIALIZATION(type)          \
namespace sys_sage {                                                          \
    template <typename... Ts>                                                 \
    struct IsBlacklistedFromDeserialization<type<Ts...>> : std::true_type {}; \
}

namespace sys_sage {
    /**
     * @brief A concept for checking if a `to_json` function exists for the given type.
     */
    template <typename T>
    concept HasToJson = requires (nlohmann::json &obj, const T &attr) { nlohmann::adl_serializer<T>::to_json(obj, attr); };

    /**
     * @brief A concept for checking if a `from_json` function exists for the given type.
     */
    template <typename T>
    concept HasFromJson = requires (const nlohmann::json &obj, T &attr) { nlohmann::adl_serializer<T>::from_json(obj, attr); }
                       || requires (const nlohmann::json &obj) { { nlohmann::adl_serializer<T>::from_json(obj) } -> std::same_as<T>; };

    /**
     * @brief Blacklists types that do not support JSON serialization, even though a `to_json` function exists.
     *        It is used to circumvent a bug in nlohmann-json.
     */
    template <typename T>
    struct IsBlacklistedFromSerialization : std::false_type {};

    /**
     * @brief Blacklists types that do not support JSON deserialization, even though a `from_json` function exists.
     *        It is used to circumvent a bug in nlohmann-json.
     */
    template <typename T>
    struct IsBlacklistedFromDeserialization : std::false_type {};

    /**
     * @brief A concept for checking if a type supports JSON serialization.
     */
    template <typename T>
    concept IsSerializable = !IsBlacklistedFromSerialization<T>::value && HasToJson<T>;

    /**
     * @brief A concept for checking if a type supports JSON deserialization.
     */
    template <typename T>
    concept IsDeserializable = !IsBlacklistedFromDeserialization<T>::value && HasFromJson<T>;

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
        static constexpr bool serializable = IsSerializable<T>;
        static constexpr bool deserializable = IsDeserializable<T>;

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

    template <CompStr compStr>
    consteval decltype(auto) CompStrCatVariadicTemplate()
    {
        return CompStrToLiteral<compStr>();
    }

    template <CompStr compStr, typename T, typename... Ts>
    consteval decltype(auto) CompStrCatVariadicTemplate()
    {
        return CompStrCatVariadicTemplate<CompStrCat<compStr, ", ", TypeTrait<T, false>::id>(), Ts...>();
    }

    template <typename T, typename... Ts>
    consteval decltype(auto) CompStrCatVariadicTemplate()
    {
        return CompStrCatVariadicTemplate<TypeTrait<T, false>::id, Ts...>();
    }

    template <typename... Ts> requires (sizeof...(Ts) == 0)
    consteval decltype(auto) CompStrCatVariadicTemplate()
    {
        return "";
    }
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

SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(std::vector)
SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(std::map)
SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(std::unordered_map)
SYS_SAGE_REGISTER_TEMPLATED_TYPE_TRAIT(std::tuple)

SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT(std::allocator)
SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT(std::less)
SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT(std::hash)
SYS_SAGE_SPECIALIZE_TEMPLATED_TYPE_TRAIT(std::equal_to)

namespace sys_sage {
    template <typename T1, typename T2, bool b>
    struct TypeTrait<std::pair<const T1, T2>, b> {
        static constexpr bool serializable = IsSerializable<std::pair<const T1, T2>>;
        static constexpr bool deserializable = IsDeserializable<std::pair<const T1, T2>>;

        static constexpr decltype(auto) id = CompStrCat<"std::pair<const ", TypeTrait<T1, false>::id, ", ", TypeTrait<T2, false>::id, ">">();

        template <typename U = std::pair<const T1, T2>> requires (deserializable)
        static std::unique_ptr<IAttribute> Deserialize(const nlohmann::json &obj)
        {
            return std::make_unique<Attribute<U>>(obj.get<U>());
        }
    };
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////// BLACKLIST SOME TYPES //////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace sys_sage {
    template <typename Key, typename T, typename Compare, typename Allocator>
    struct IsBlacklistedFromDeserialization<std::multimap<Key, T, Compare, Allocator>> : std::bool_constant<!std::same_as<Key, std::string>> {};
}

namespace sys_sage {
    template <typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
    struct IsBlacklistedFromDeserialization<std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator>> : std::bool_constant<!std::same_as<Key, std::string>> {};
}

#include <sys-sage/attribute.inl>

#endif
